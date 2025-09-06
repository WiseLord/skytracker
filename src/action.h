#ifndef ACTION_H
#define ACTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint8_t ActionType;
enum {
    ACTION_NONE = 0,

    ACTION_BTN_SHORT,
    ACTION_BTN_LONG,
    ACTION_ENCODER,

    ACTION_ROTATE,
    ACTION_STEP_RESET,
    ACTION_TRACK,
    ACTION_SLOWDOWN,
    ACTION_STEP_SIZE,

    ACTION_STANDBY,
    ACTION_BACKLIGHT,

    ACTION_TYPE_END
};

typedef struct {
    ActionType type;
    int32_t value;
} Action;

#ifdef __cplusplus
}
#endif

#endif // ACTION_H
