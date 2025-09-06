#ifndef SETTINGS_H
#define SETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint8_t Param;
enum {
    PARAM_NULL = 0,

    //----------------------------------------------------------------

    PARAM_SYSTEM_ENC_RES = 70,

    PARAM_END
};

static inline void settingsInit(void) { return; }

static inline int16_t settingsRead(Param param, int16_t defValue) { return defValue; }
static inline void settingsStore(Param param, int16_t value) { return; };

#ifdef __cplusplus
}
#endif

#endif // SETTINGS_H
