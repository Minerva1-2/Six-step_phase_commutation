/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2026-06-02 14:34:07
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2026-06-04 12:12:38
 * @FilePath: \MDK-ARMd:\cubemx\project\keil\MotorDriver_F405RGT6\Core\Inc\bldc.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __BLDC_H
#define __BLDC_H

#include "gpio.h"
#include "stdint.h"
#include "tim.h"

#define RUN 				(1U)
#define STOP				(0U)
#define HIGH 				(1U)
#define LOW					(0U)
#define DRV8301_GPIO		GPIOB
#define DRV8301_GPIO_PIN	GPIO_PIN_5	
#define DRV8301_FAULT_PIN	GPIO_PIN_7	
#define DRV8301_ENABLE		HAL_GPIO_WritePin(DRV8301_GPIO, DRV8301_GPIO_PIN, GPIO_PIN_SET)
#define DRV8301_DISABLE		HAL_GPIO_WritePin(DRV8301_GPIO, DRV8301_GPIO_PIN, GPIO_PIN_RESET)
#define DRV8301_FAULT		HAL_GPIO_ReadPin(DRV8301_GPIO, DRV8301_FAULT_PIN)
#define MAX_PWM_DUTY		(8400U)
#define BLDC_START_DUTY		(2520U)

// 三相桥臂
#define UH_Pin 				GPIO_PIN_8
#define UH_GPIO_Port 		GPIOA
#define UL_Pin 				GPIO_PIN_13
#define UL_GPIO_Port 		GPIOB

#define VH_Pin 				GPIO_PIN_9
#define VH_GPIO_Port 		GPIOA
#define VL_Pin 				GPIO_PIN_14
#define VL_GPIO_Port 		GPIOB

#define WH_Pin 				GPIO_PIN_10
#define WH_GPIO_Port 		GPIOA
#define WL_Pin 				GPIO_PIN_15
#define WL_GPIO_Port 		GPIOB

// 霍尔传感器
#define HALL_A_Pin 			GPIO_PIN_6
#define HALL_A_GPIO_Port 	GPIOC
#define HALL_B_Pin 			GPIO_PIN_7
#define HALL_B_GPIO_Port 	GPIOC
#define HALL_C_Pin 			GPIO_PIN_8
#define HALL_C_GPIO_Port 	GPIOC

typedef struct {
	uint8_t state;
	uint8_t hall;
	uint8_t last_hall;
	uint32_t duty;
}MOTOR_OBJ;

typedef enum
{
    PHASE_FLOAT = 0,
    PHASE_HIGH_PWM,
    PHASE_LOW_ON
} PhaseState;

uint8_t Get_HALL_State(void);
static void Motor_Commutation(uint8_t hall_state, uint32_t pwm_duty);
void Motor_OnHallChanged();
void motor_init(void);
MOTOR_OBJ* Get_Motor_struct(void);

#endif
