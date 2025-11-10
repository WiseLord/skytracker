#ifndef STEPPER_H
#define STEPPER_H

#include <stdint.h>
#include <stdbool.h>

// STEP/DIR pins
#define STEP_Port               GPIOA
#define STEP_Pin                LL_GPIO_PIN_1
#define DIR_Port                GPIOA
#define DIR_Pin                 LL_GPIO_PIN_0

#define EN_Port                 GPIOA
#define EN_Pin                  LL_GPIO_PIN_4

#define MS1_Port                GPIOA
#define MS1_Pin                 LL_GPIO_PIN_7
#define MS2_Port                GPIOA
#define MS2_Pin                 LL_GPIO_PIN_6

#define STEPS_PER_EVOLUTION     200
#define MICROSTEPS              64

typedef struct {
    uint16_t prescaler;
    uint16_t reload;
    const char *name;
} Divider;

typedef uint8_t DividerType;
enum {
    DIVIDER_BEGIN = 0,

    DIVIDER_STAR = DIVIDER_BEGIN,
    DIVIDER_MOON,

    DIVIDER_END
};

typedef struct {
    int32_t target;
    int32_t position;
    int32_t queue;
    int32_t speed;
    int8_t div;
    bool hold;
    bool track;
    bool slow;
} Stepper;

void stepperInit();

Stepper *stepperGet(void);

void stepperAdd(int32_t value);
void stepperReset(void);
void stepperDivider(DividerType div);
const char *stepperDividerName(void);
void stepperTrack(bool value);
void stepperHold(bool value);
void stepperSlowDown(bool value);

#endif // STEPPER_H
