#include "app.h"

#include <stdio.h>

#include "action.h"
#include "display/glcd.h"
#include "input.h"
#include "inttypes.h"
#include "settings.h"
#include "stepper.h"
#include "utils.h"

#define FLAG_EXIT           0
#define FLAG_ENTER          1
#define FLAG_SWITCH         2

#define LCD_BR_MIN          1
#define LCD_BR_MAX          32

static Action action = {
    .type = ACTION_NONE,
    .value = FLAG_ENTER,
};

static App app = {
    .step = 64,
    .backlight = true,
    .clear = true,
};

static void actionSet(ActionType type, int32_t value)
{
    action.type = type;
    action.value = value;
}

void appInit()
{
    glcdInit(GLCD_LANDSCAPE);

    settingsInit();
    utilInitSysCounter();

    inputInit();

    stepperInit();

    // Graphics
    int16_t w = glcdGet()->rect.w;
    int16_t h = glcdGet()->rect.h;

    glcdDrawRect(0, 0, w, h, COLOR_BLACK);
    glcdSetBacklight(app.backlight);
}

void appRun()
{
    appActionGet();
    appActionRemap();
    appActionHandle();

    action.type = ACTION_NONE;

    appShow();
}

static Action appGetButtons()
{
    Action ret = {.type = ACTION_NONE, .value = 0};

    CmdBtn cmdBtn = inputGetBtnCmd();

    if (cmdBtn.btn) {
        ret.value = cmdBtn.btn;
        if (cmdBtn.flags & BTN_FLAG_LONG_PRESS) {
            ret.type = ACTION_BTN_LONG;
        } else {
            ret.type = ACTION_BTN_SHORT;
        }
    }

    return ret;
}

static Action appGetEncoder(void)
{
    Action ret = {.type = ACTION_NONE, .value = 0};

    ret.value = inputGetEncoder();

    if (ret.value) {
        ret.type = ACTION_ENCODER;
    }

    return ret;
}

static void actionRemapBtnShort(void)
{
    switch (action.value) {
    case BTN_D0:
        actionSet(ACTION_STANDBY, FLAG_SWITCH);
        break;
    case BTN_D1:
        actionSet(ACTION_TRACK, FLAG_SWITCH);
        break;
    case BTN_D3:
        actionSet(ACTION_STEP_SIZE, FLAG_SWITCH);
        break;
    case BTN_D4:
        actionSet(ACTION_ROTATE, -app.step);
        break;
    case BTN_D5:
        actionSet(ACTION_ROTATE, +app.step);
        break;
    default:
        break;
    }
}

static void actionRemapBtnLong(void)
{
    switch (action.value) {
    case BTN_D0:
        actionSet(ACTION_BACKLIGHT, FLAG_SWITCH);
        break;
    case BTN_D1:
        actionSet(ACTION_DIVIDER, +1);
        break;
    case BTN_D3:
        actionSet(ACTION_STEP_RESET, 0);
        // actionSet(ACTION_SLOWDOWN, FLAG_SWITCH);
        break;
    case BTN_D4:
        actionSet(ACTION_ROTATE, -STEPS_PER_EVOLUTION * MICROSTEPS);
        break;
    case BTN_D5:
        actionSet(ACTION_ROTATE, +STEPS_PER_EVOLUTION * MICROSTEPS);
        break;
    default:
        break;
    }
}

static void actionRemapEncoder(void)
{
    int16_t encCnt = action.value;

    actionSet(ACTION_ROTATE, encCnt);
}

void appActionGet(void)
{
    if (ACTION_NONE == action.type) {
        action = appGetButtons();
    }

    if (ACTION_NONE == action.type) {
        action = appGetEncoder();
    }
}

void appActionRemap(void)
{
    switch (action.type) {
    case ACTION_BTN_SHORT:
        actionRemapBtnShort();
        break;
    case ACTION_BTN_LONG:
        actionRemapBtnLong();
        break;
    default:
        break;
    }

    if (ACTION_ENCODER == action.type) {
        actionRemapEncoder();
    }
}

void appActionHandle(void)
{
    Stepper *s = stepperGet();

    switch (action.type) {
    case ACTION_ROTATE:
        stepperAdd(action.value);
        break;
    case ACTION_TRACK:
        stepperTrack(!s->track);
        break;
    case ACTION_STEP_RESET:
        stepperReset();
        break;
    case ACTION_DIVIDER:
        stepperDivider(s->div + 1);
        break;
    case ACTION_SLOWDOWN:
        stepperSlowDown(!s->slow);
        break;
    case ACTION_STEP_SIZE:
        app.step >>= 1;
        if (app.step <= 0) {
            app.step = (1 << 16);
        }
        break;
    case ACTION_BACKLIGHT:
        app.backlight = !app.backlight;
        glcdSetBacklight(app.backlight);
        break;
    case ACTION_STANDBY:
        stepperHold(!s->hold);
        break;
    }
}

#include "hwlibs.h"

void appShow()
{
    Stepper *s = stepperGet();
    GlcdRect r = glcdGet()->rect;

    glcdSetFontColor(COLOR_WHITE);
    glcdSetFontBgColor(COLOR_BLACK);

    char buf[32];

    if (app.clear) {
        glcdDrawRect(5, 24, 310, 1, COLOR_WHITE);

        glcdSetFont(&fontterminus24b);
        glcdSetXY(160, 0);
        glcdSetFontAlign(GLCD_ALIGN_CENTER);
        glcdWriteString("Экваториальная монтировка");

        glcdSetFont(&fontterminus22b);

        glcdSetXY(1, 26);
        glcdWriteString("Питание двигателя ");

        glcdSetXY(1, 50);
        glcdWriteString("Режим слежения ");

        glcdSetXY(1, 74);
        glcdWriteString("Объект ");

        glcdSetXY(1, 98);
        glcdWriteString("Размер шага ");

        glcdSetXY(1, 122);
        glcdWriteString("Целевая позиция ");

        glcdSetXY(1, 146);
        glcdWriteString("Текущая позиция ");

        glcdSetXY(1, 170);
        glcdWriteString("Шагов в очереди ");

        glcdSetXY(1, 194);
        glcdWriteString("Текущая скорость ");

        app.clear = false;
    }

    snprintf(buf, sizeof(buf), "%s", s->hold ? "  вкл" : "  откл");
    glcdSetFont(&fontterminus22b);
    glcdSetXY(r.w - 1, 26);
    glcdSetFontAlign(GLCD_ALIGN_RIGHT);
    glcdWriteString(buf);

    snprintf(buf, sizeof(buf), "%s", s->track ? "  вкл" : "  откл");
    glcdSetFont(&fontterminus22b);
    glcdSetXY(r.w - 1, 50);
    glcdSetFontAlign(GLCD_ALIGN_RIGHT);
    glcdWriteString(buf);

    snprintf(buf, sizeof(buf), "%s", stepperDividerName());
    glcdSetFont(&fontterminus22b);
    glcdSetXY(r.w - 1, 74);
    glcdSetFontAlign(GLCD_ALIGN_RIGHT);
    glcdWriteString(buf);

    snprintf(buf, sizeof(buf), "%8" PRId32, app.step);
    glcdSetFont(&fontterminus22b);
    glcdSetXY(r.w - 1, 98);
    glcdSetFontAlign(GLCD_ALIGN_RIGHT);
    glcdWriteString(buf);

    snprintf(buf, sizeof(buf), "%8" PRId32, s->motor[MOTOR_EQ].target);
    glcdSetFont(&fontterminus22b);
    glcdSetXY(r.w - 1, 122);
    glcdSetFontAlign(GLCD_ALIGN_RIGHT);
    glcdWriteString(buf);

    snprintf(buf, sizeof(buf), "%8" PRId32, s->motor[MOTOR_EQ].position);
    glcdSetFont(&fontterminus22b);
    glcdSetXY(r.w - 1, 146);
    glcdSetFontAlign(GLCD_ALIGN_RIGHT);
    glcdWriteString(buf);

    snprintf(buf, sizeof(buf), "%8" PRId32, s->motor[MOTOR_EQ].queue);
    glcdSetFont(&fontterminus22b);
    glcdSetXY(r.w - 1, 168);
    glcdSetFontAlign(GLCD_ALIGN_RIGHT);
    glcdWriteString(buf);

    glcdSetFont(&fontterminus22b);
    snprintf(buf, sizeof(buf), "%8" PRId32, s->motor[MOTOR_EQ].speed);
    glcdSetXY(r.w - 1, 194);
    glcdSetFontAlign(GLCD_ALIGN_RIGHT);
    glcdWriteString(buf);
}
