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
    .backlight = true,
};

static void actionSet(ActionType type, int16_t value)
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
    utilmDelay(100);
    glcdSetBacklight(app.backlight);

    glcdSetFontColor(COLOR_WHITE);
    glcdSetFontBgColor(COLOR_BLACK);

    glcdSetFont(&fontterminus24b);
    glcdSetXY(160, 0);
    glcdSetFontAlign(GLCD_ALIGN_CENTER);
    glcdWriteString("Экваториальная монтировка");
    glcdDrawRect(0, 24, 320, 2, COLOR_WHITE);

    glcdSetFont(&fontterminus24b);
    glcdSetXY(0, 28);
    glcdWriteString("Двигатель: ");

    glcdSetFont(&fontterminus24b);
    glcdSetXY(0, 52);
    glcdWriteString("Слежение: ");

    glcdSetFont(&fontterminus24b);
    glcdSetXY(0, 76);
    glcdWriteString("Позиция: ");

    glcdSetFont(&fontterminus24b);
    glcdSetXY(0, 100);
    glcdWriteString("Очередь: ");
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
    case BTN_D2:
        break;
    case BTN_D3:
        break;
    case BTN_D4:
        actionSet(ACTION_ROTATE, -1);
        break;
    case BTN_D5:
        actionSet(ACTION_ROTATE, +1);
        break;
    case ENC_A:
        actionSet(ACTION_ENCODER, -1);
        break;
    case ENC_B:
        actionSet(ACTION_ENCODER, +1);
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
        actionSet(ACTION_STEP_RESET, 0);
        break;
    case BTN_D2:
        break;
    case BTN_D3:
        break;
    case BTN_D4:
        actionSet(ACTION_ROTATE, -200);
        break;
    case BTN_D5:
        actionSet(ACTION_ROTATE, +200);
        break;
    case ENC_A:
        break;
    case ENC_B:
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
        stepperAdd(64 * action.value);
        break;
    case ACTION_TRACK:
        stepperTrack(!s->track);
        break;
    case ACTION_STEP_RESET:
        stepperReset();
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

void appShow()
{
    char buf[32];
    Stepper *s = stepperGet();
    GlcdRect r = glcdGet()->rect;

    snprintf(buf, sizeof(buf), "%s", s->hold ? "  вкл" : "  откл");
    glcdSetFont(&fontterminus24b);
    glcdSetXY(r.w, 28);
    glcdSetFontAlign(GLCD_ALIGN_RIGHT);
    glcdWriteString(buf);

    snprintf(buf, sizeof(buf), "%s", s->track ? "  вкл" : "  откл");
    glcdSetFont(&fontterminus24b);
    glcdSetXY(r.w, 52);
    glcdSetFontAlign(GLCD_ALIGN_RIGHT);
    glcdWriteString(buf);

    snprintf(buf, sizeof(buf), "%8" PRId32, s->step);
    glcdSetFont(&fontterminus24b);
    glcdSetXY(r.w, 76);
    glcdSetFontAlign(GLCD_ALIGN_RIGHT);
    glcdWriteString(buf);

    snprintf(buf, sizeof(buf), "%8" PRId32, s->queue);
    glcdSetFont(&fontterminus24b);
    glcdSetXY(r.w, 100);
    glcdSetFontAlign(GLCD_ALIGN_RIGHT);
    glcdWriteString(buf);
}
