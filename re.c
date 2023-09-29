/* Rotary encoder handler for arduino.
 *
 * Copyright 2011 Ben Buxton. Licenced under the GNU GPL Version 3.
 * Contact: bb@cactii.net
 *
 * Port to STM32 by Zixun Chen.
 */

#include "re.h"

/*
 * The below state table has, for each state (row), the new state
 * to set based on the next encoder output. From left to right in,
 * the table, the encoder outputs are 00, 01, 10, 11, and the value
 * in that position is the new state to set.
 */

#define R_START 0x0

#ifdef RE_HALF_STEP
// Use the half-step state table (emits a code at 00 and 11)
#define R_CCW_BEGIN 0x1
#define R_CW_BEGIN 0x2
#define R_START_M 0x3
#define R_CW_BEGIN_M 0x4
#define R_CCW_BEGIN_M 0x5
const unsigned char ttable[6][4] = {
    // R_START (00)
    {R_START_M,           R_CW_BEGIN,    R_CCW_BEGIN,  R_START},
    // R_CCW_BEGIN
    {R_START_M | DIR_CCW, R_START,       R_CCW_BEGIN,  R_START},
    // R_CW_BEGIN
    {R_START_M | DIR_CW,  R_CW_BEGIN,    R_START,      R_START},
    // R_START_M (11)
    {R_START_M,           R_CCW_BEGIN_M, R_CW_BEGIN_M, R_START},
    // R_CW_BEGIN_M
    {R_START_M,           R_START_M,     R_CW_BEGIN_M, R_START | DIR_CW},
    // R_CCW_BEGIN_M
    {R_START_M,           R_CCW_BEGIN_M, R_START_M,    R_START | DIR_CCW},
};
#else
// Use the full-step state table (emits a code at 00 only)
#define R_CW_FINAL 0x1
#define R_CW_BEGIN 0x2
#define R_CW_NEXT 0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT 0x6

const unsigned char ttable[7][4] = {
  // R_START
  {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},
  // R_CW_FINAL
  {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},
  // R_CW_BEGIN
  {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},
  // R_CW_NEXT
  {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},
  // R_CCW_BEGIN
  {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},
  // R_CCW_FINAL
  {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW},
  // R_CCW_NEXT
  {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},
};
#endif

static uint8_t ReadPinLevel(GPIO_Port GPIOx, GPIO_Pin PINy);

void RotaryEncoderInit(REHandle_t *REVal) {
    GPIO_InitParam GPIO_InitVal={0};
    // Initialise the GPIO pins
    GPIO_InitVal.driving=GPIO_DRIVING_LEVEL_1;
    GPIO_InitVal.mode=GPIOx_Pn_F0_INPUT;
    GPIO_InitVal.pull=GPIO_PULL_NONE;
    HAL_GPIO_Init(REVal->GPIO_A, REVal->PIN_A, &GPIO_InitVal);
    HAL_GPIO_Init(REVal->GPIO_B, REVal->PIN_B, &GPIO_InitVal);
    // Initialise state.
    REVal->State=R_START;
}

void RotaryEncoderProcess(REHandle_t *REVal) {
    // Grab state of input pins.
    uint8_t pinstate;
    pinstate=(ReadPinLevel(REVal->GPIO_B, REVal->PIN_B)<<1) | 
              ReadPinLevel(REVal->GPIO_A, REVal->PIN_A);
    // Determine new state from the pins and state table.
    REVal->State=ttable[REVal->State & 0xf][pinstate];
    // Return emit bits, ie the generated event.
    REVal->RetVal=REVal->State & 0x30;
}

static uint8_t ReadPinLevel(GPIO_Port GPIOx, GPIO_Pin PINy)
{
    GPIO_PinState RDPin;
    RDPin=HAL_GPIO_ReadPin(GPIOx, PINy);
    #ifdef RE_PINS_PULL_DOWN
    // Invert the levels if pins pulled down.
    if(GPIO_PIN_HIGH==RDPin) {
        return 0;
    } else {
        return 1;
    }
    #else // RE_PINS_PULL_DOWN
    if(GPIO_PIN_HIGH==RDPin) {
        return 1;
    } else {
        return 0;
    }
    #endif // RE_PINS_PULL_DOWN
}

