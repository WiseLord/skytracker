#ifndef FONT7SEG_H
#define FONT7SEG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "display/colors.h"

#define BIT_A       (1<<0)
#define BIT_B       (1<<1)
#define BIT_C       (1<<2)
#define BIT_D       (1<<3)
#define BIT_E       (1<<4)
#define BIT_F       (1<<5)
#define BIT_G       (1<<6)
#define BIT_P       (1<<7)

#define CH_0        (BIT_A | BIT_B | BIT_C | BIT_D | BIT_E | BIT_F)
#define CH_1        (BIT_B | BIT_C)
#define CH_2        (BIT_A | BIT_B | BIT_D | BIT_E | BIT_G)
#define CH_3        (BIT_A | BIT_B | BIT_C | BIT_D | BIT_G)
#define CH_4        (BIT_B | BIT_C | BIT_F | BIT_G)
#define CH_5        (BIT_A | BIT_C | BIT_D | BIT_F | BIT_G)
#define CH_6        (BIT_A | BIT_C | BIT_D | BIT_E | BIT_F | BIT_G)
#define CH_7        (BIT_A | BIT_B | BIT_C)
#define CH_8        (BIT_A | BIT_B | BIT_C | BIT_D | BIT_E | BIT_F | BIT_G)
#define CH_9        (BIT_A | BIT_B | BIT_C | BIT_D | BIT_F | BIT_G)
#define CH_A        (BIT_A | BIT_B | BIT_C | BIT_E | BIT_F | BIT_G)
#define CH_B        (BIT_C | BIT_D | BIT_E | BIT_F | BIT_G)
#define CH_C        (BIT_A | BIT_D | BIT_E | BIT_F)
#define CH_D        (BIT_B | BIT_C | BIT_D | BIT_E | BIT_G)
#define CH_E        (BIT_A | BIT_D | BIT_E | BIT_F | BIT_G)
#define CH_F        (BIT_A | BIT_E | BIT_F | BIT_G)
#define CH_EMPTY    (0)
#define CH_MINUS    (BIT_G)

typedef uint8_t tFont7seg[];

extern const tFont7seg font_7seg_13;
extern const tFont7seg font_7seg_11;
extern const tFont7seg font_7seg_10;
extern const tFont7seg font_7seg_8;
extern const tFont7seg font_7seg_7;
extern const tFont7seg font_7seg_6;
extern const tFont7seg font_7seg_5;
extern const tFont7seg font_7seg_4;
extern const tFont7seg font_7seg_3;

void font7segLoad(const uint8_t *font);
void font7segWriteChar(char code);
void font7segWriteString(const char *string);

#ifdef __cplusplus
}
#endif

#endif // FONT7SEG_H
