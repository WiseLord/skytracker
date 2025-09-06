#ifndef STEPPER_H
#define STEPPER_H

#include <stdint.h>
#include <stdbool.h>

// STEP/DIR pins
#define STEP_Port               GPIOB
#define STEP_Pin                LL_GPIO_PIN_10
#define DIR_Port                GPIOB
#define DIR_Pin                 LL_GPIO_PIN_11

#define EN_Port                 GPIOA
#define EN_Pin                  LL_GPIO_PIN_4

#define MS1_Port                GPIOA
#define MS1_Pin                 LL_GPIO_PIN_7
#define MS2_Port                GPIOA
#define MS2_Pin                 LL_GPIO_PIN_6

typedef struct {
    int32_t target;
    int32_t position;
    int32_t queue;
    int32_t speed;
    bool hold;
    bool track;
    bool slow;
} Stepper;

void stepperInit();

Stepper *stepperGet(void);

void stepperAdd(int32_t value);
void stepperReset(void);
void stepperTrack(bool value);
void stepperHold(bool value);
void stepperSlowDown(bool value);

#endif // STEPPER_H
