/*
 * Rotary encoder library for Arduino.
 * Port to STM32 by Zixun Chen.
 */

#ifndef _ROTARY_ENCODER_H_
#define _ROTARY_ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// Enable this to emit codes twice per step.
#define RE_HALF_STEP

// Invert the levels if pins pulled down.
// #define RE_PINS_PULL_DOWN

#define DIR_NONE 0    // No complete step yet.
#define DIR_CW   0x10 // Clockwise step.
#define DIR_CCW  0x20 // Counter-clockwise step.

typedef struct {
    GPIO_Port GPIO_A; // Specifies the A pin.
    GPIO_Pin PIN_A;
    GPIO_Port GPIO_B; // Specifies the B pin.
    GPIO_Pin PIN_B;
    uint8_t RetVal; // Return value.
    uint8_t State;
} REHandle_t;

void RotaryEncoderInit(REHandle_t *REVal);
void RotaryEncoderProcess(REHandle_t *REVal);

#ifdef __cplusplus
}
#endif

#endif // _ROTARY_ENCODER_H_

