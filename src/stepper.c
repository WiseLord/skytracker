#include "stepper.h"

#include "hwlibs.h"
#include "timers.h"
#include "utils.h"

static Stepper stepper = {
    .hold = false,
    .track = false,
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
    timerInit(TIM_STEP, 99, 71); // 72MHz / 100 / 72 = 10kHz

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

void TIM_STEP_HANDLER(void)
{
    if (LL_TIM_IsActiveFlag_UPDATE(TIM_STEP)) {
        // Clear the update interrupt flag
        LL_TIM_ClearFlag_UPDATE(TIM_STEP);

        // Skip if no action required
        if (!stepper.hold || stepper.queue == 0) {
            return;
        }

        // Select direction
        if (stepper.queue > 0) {
            stepper.queue--;
            stepper.step++;
            SET(DIR);
        } else {
            stepper.queue++;
            stepper.step--;
            CLR(DIR);
        }

        // Do one step
        SET(STEP);
        utiluDelay(1);
        CLR(STEP);
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
