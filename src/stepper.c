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

static Stepper stepper = {
    .hold = false,
    .track = false,
    .speed = 1,
};

void stepperAdd(int32_t value)
{   if (stepper.hold) {
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
    timerInit(TIM_TRACK, 46638, 432); // 72MHz / 46639 / 433 = 3.565294 Hz
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
            stepper.queue++;
        }
    }
}

__attribute__((always_inline))
static inline void doStep(int32_t speed) {
    if (speed) {
        if (speed > 0) {
            SET(DIR);
            stepper.queue--;
            stepper.step++;
        }
        if (speed < 0) {
            CLR(DIR);
            stepper.queue++;
            stepper.step--;
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
            if (stepper.speed > stepper.queue) {
                stepper.speed--;
            } else {
                stepper.speed++;
            }
        }
        if (stepper.queue < 0) {
            if (stepper.speed < stepper.queue) {
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
}

void stepperReset(void)
{
    stepper.step = 0;
    stepper.queue = 0;
}
