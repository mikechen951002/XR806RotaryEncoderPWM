/*
 * Copyright (C) 2017 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "main.h"

static REHandle_t REVal={
    .GPIO_A=GPIO_PORT_A,
    .PIN_A=GPIO_PIN_12,
    .GPIO_B=GPIO_PORT_A,
    .PIN_B=GPIO_PIN_13
};
static const uint8_t STEPMAX=10, STEPMIN=0;
static uint8_t step=5;
static void RotaryScan(void);

#define PWM_CHANNEL   PWM_GROUP1_CH2
#define PWM_MODE      PWM_CYCLE_MODE
#define PWM_GPIO_PORT GPIO_PORT_A
#define PWM_GPIO_PIN  GPIO_PIN_21
#define PWM_GPIO_MODE GPIOA_P21_F4_PWM2_ECT2
static int max_duty_ratio;
static void PWMCycleModeSet(void);
static HAL_Status PWMDutyRatioSet(int val);

#define TASK_RE_PRIO     1
#define TASK_RE_STK_SIZE 200
static TaskHandle_t TaskRE_Handler=NULL;
static void TaskCreation(void);
static void TaskRE(void *pvParameters);

int main(void)
{
    printf("Rotary encoder & PWM demo.\r\n");
    RotaryEncoderInit(&REVal);
    PWMCycleModeSet();
    PWMDutyRatioSet(max_duty_ratio*step/STEPMAX);
    TaskCreation();
    return 0;
}

static void TaskCreation(void)
{
    BaseType_t xRet = NULL;
    taskENTER_CRITICAL();
    xRet = xTaskCreate((TaskFunction_t )TaskRE, (const char *)"TaskRE", (uint16_t)TASK_RE_STK_SIZE, 
                       (void *)NULL, (UBaseType_t)TASK_RE_PRIO, (TaskHandle_t *)&TaskRE_Handler);
    if(pdPASS == xRet) {
        printf("TaskRE created!\r\n");
    }
    taskEXIT_CRITICAL();
}

static void TaskRE(void *pvParameters)
{
    while (1) {
        RotaryScan();
        vTaskDelay(10);
    }
}

static void RotaryScan(void)
{
	RotaryEncoderProcess(&REVal);
	if(DIR_CW == REVal.RetVal) {
        if(step<STEPMAX) {
            step++;
            PWMDutyRatioSet(max_duty_ratio*step/STEPMAX);
            printf("%d  ", step);
        }
    } else if(DIR_CCW == REVal.RetVal) {
        if(step>STEPMIN) {
            step--;
            PWMDutyRatioSet(max_duty_ratio*step/STEPMAX);
            printf("%d  ", step);
        }
    }
}

static void PWMCycleModeSet(void)
{
    GPIO_InitParam io_param = {0};
	HAL_Status status = HAL_ERROR;
	PWM_ClkParam clk_param = {0};
	PWM_ChInitParam ch_param = {0};

    io_param.driving = GPIO_DRIVING_LEVEL_1;
    io_param.mode = PWM_GPIO_MODE;
    io_param.pull = GPIO_PULL_NONE;
    HAL_GPIO_Init(PWM_GPIO_PORT, PWM_GPIO_PIN, &io_param);

	clk_param.clk = PWM_CLK_HOSC;
	clk_param.div = PWM_SRC_CLK_DIV_1;
	status = HAL_PWM_GroupClkCfg(PWM_CHANNEL / 2, &clk_param);
	if (status != HAL_OK) {
		printf("%s(): %d, PWM group clk config error\n", __func__, __LINE__);
	}

	ch_param.hz = 1000;
	ch_param.mode = PWM_MODE;
	ch_param.polarity = PWM_HIGHLEVE;
	max_duty_ratio = HAL_PWM_ChInit(PWM_CHANNEL, &ch_param);
	if (max_duty_ratio == -1) {
		printf("%s(): %d, PWM ch init error\n", __func__, __LINE__);
    }
    
	status = HAL_PWM_ChSetDutyRatio(PWM_CHANNEL, max_duty_ratio / 2);
	if (status != HAL_OK) {
		printf("%s(): %d, PWM set duty ratio error\n", __func__, __LINE__);
    }
    
	status = HAL_PWM_EnableCh(PWM_CHANNEL, PWM_MODE, 1);
	if (status != HAL_OK) {
		printf("%s(): %d, PWM ch enable error\n", __func__, __LINE__);
	}
}

static HAL_Status PWMDutyRatioSet(int val)
{
    return HAL_PWM_ChSetDutyRatio(PWM_CHANNEL, val);
}

