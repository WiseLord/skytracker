#include "stepper.h"

#include "hwlibs.h"
#include "timers.h"
#include "utils.h"

#define ABS(x)          ((x) >= 0 ? (x) : (-x))
#define MAX(x, y)       ((x) > (y) ? (x) : (y))
#define MIN(x, y)       ((x) < (y) ? (x) : (y))

#define RELOAD_MAX          65535
#define SPEED_MAX           2000
#define USTEPS_PER_SEC_MAX  100

static const Divider dividers[DIVIDER_END] = {
    [DIVIDER_STAR] = {46638, 432, " звёзды"},  // 72MHz / 46639 / 433 = 3.565294 Hz
    [DIVIDER_MOON] = {8196, 2556, "   Луна"},
};

static Stepper stepper = {
    .div = DIVIDER_STAR,
    .hold = false,
    .track = false,
    .speed = 0,
};

void stepperAdd(int32_t value)
{   if (stepper.hold) {
        stepper.target += value;
        stepper.queue += value;
    }
}

void stepperInit()
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
#ifdef STM32F3
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
#endif

    GPIO_InitStruct.Pin = STEP_Pin;
    LL_GPIO_Init(STEP_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = DIR_Pin;
    LL_GPIO_Init(DIR_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = EN_Pin;
    LL_GPIO_Init(EN_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MS1_Pin;
    LL_GPIO_Init(MS1_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = MS2_Pin;
    LL_GPIO_Init(MS2_Port, &GPIO_InitStruct);

    CLR(STEP);
    CLR(DIR);

    //  MS21:   TMC2208 TMC2209
    //    00:   1/8     1/8
    //    01:   1/2     1/32
    //    10:   1/4     1/64
    //    11:   1/16    1/16

    // Set 1/64 microstep for TMC2209
    SET(MS2);
    CLR(MS1);

    // Turn off motors
    SET(EN);

    // Step period => 100us
    timerInit(TIM_STEP, 99, 65535);

    // Track period 280482us
    stepperDivider(DIVIDER_STAR);
}

Stepper *stepperGet()
{
    return &stepper;
}

void TIM_TRACK_HANDLER(void)
{
    if (LL_TIM_IsActiveFlag_UPDATE(TIM_TRACK)) {
        // Clear the update interrupt flag
        LL_TIM_ClearFlag_UPDATE(TIM_TRACK);

        // Callbacks
        if (stepper.hold && stepper.track) {
            stepperAdd(+1);
        }
    }
}

__attribute__((always_inline))
static inline void doStep(int32_t speed) {
    if (speed) {
        if (speed > 0) {
            SET(DIR);
            stepper.queue--;
            stepper.position++;
        }
        if (speed < 0) {
            CLR(DIR);
            stepper.queue++;
            stepper.position--;
        }
        // Do one step
        SET(STEP);
        utiluDelay(1);
        CLR(STEP);
    }

    int32_t reload = RELOAD_MAX;
    if (speed) {
        reload = 200000 / ABS(speed);
    }
    if (reload > RELOAD_MAX) {
        reload = RELOAD_MAX;
    }
    LL_TIM_SetAutoReload(TIM_STEP, (uint32_t)reload);
}

void TIM_STEP_HANDLER(void)
{
    // int32_t direction = 0;

    if (LL_TIM_IsActiveFlag_UPDATE(TIM_STEP)) {
        // Clear the update interrupt flag
        LL_TIM_ClearFlag_UPDATE(TIM_STEP);

        // Skip if motor not active
        if (!stepper.hold) {
            return;
        }

        if (stepper.queue == 0) {
            if (stepper.speed <= 1 && stepper.speed >= -1) {
                stepper.speed = 0;
            }
        }
        if (stepper.queue > 0) {
            if (stepper.speed >= stepper.queue) {
                stepper.speed--;
            } else {
                stepper.speed++;
            }
        }
        if (stepper.queue < 0) {
            if (stepper.speed <= stepper.queue) {
                stepper.speed++;
            } else {
                stepper.speed--;
            }
        }

        // Max speed limit
        if (stepper.speed > SPEED_MAX) {
            stepper.speed = SPEED_MAX;
        }
        if (stepper.speed < -SPEED_MAX) {
            stepper.speed = -SPEED_MAX;
        }

        doStep(stepper.speed);
    }
}

void stepperTrack(bool value)
{
    stepper.track = value;
}

void stepperHold(bool value)
{
    stepper.hold = value;

    if (stepper.hold) {
        CLR(EN);
    } else {
        SET(EN);
    }

    stepperReset();
}

void stepperReset(void)
{
    stepper.target = 0;
    stepper.position = 0;
    stepper.queue = 0;
    stepper.speed = 0;
}

void stepperSlowDown(bool value)
{
    stepper.slow = value;
    if (value) {
        LL_TIM_SetPrescaler(TIM_STEP, 999);
    } else {
        LL_TIM_SetPrescaler(TIM_STEP, 99);
    }
}

void stepperDivider(DividerType div)
{
    if (div >= DIVIDER_END) {
        div = DIVIDER_BEGIN;
    }

    stepper.div = div;

    LL_TIM_DisableCounter(TIM_TRACK);
    LL_TIM_SetCounter(TIM_TRACK, dividers[div].reload);

    timerInit(TIM_TRACK, dividers[div].prescaler, dividers[div].reload);
}

const char *stepperDividerName(void)
{
    return dividers[stepper.div].name;
}
