#include "bldc.h"
#include "usbd_cdc_if.h"

uint8_t g_hall_state;

static MOTOR_OBJ WR_36SV61;

//关闭通道
static void Phase_Off(uint32_t ch)
{
	 __HAL_TIM_SET_COMPARE(&htim1, ch, 0);
	
    HAL_TIM_PWM_Stop(&htim1, ch);
    HAL_TIMEx_PWMN_Stop(&htim1, ch);
}
//设置上桥臂PWM
static void Phase_HighPWM(uint32_t ch, uint16_t duty)
{
    __HAL_TIM_SET_COMPARE(&htim1, ch, duty);

    HAL_TIMEx_PWMN_Stop(&htim1, ch);
    HAL_TIM_PWM_Start(&htim1, ch);
}
//设置下桥臂常开
static void Phase_LowOn(uint32_t ch)
{
    /*
     * PWM1 模式下，CCR = 0 时 OCREF 基本为 0，
     * 互补输出 CHxN 为有效状态，可近似作为低边常开。
     * 实际电平还要结合驱动器输入极性确认。
     */
	__HAL_TIM_SET_COMPARE(&htim1, ch, MAX_PWM_DUTY);
	
	HAL_TIM_PWM_Stop(&htim1, ch);
    HAL_TIMEx_PWMN_Start(&htim1, ch);
}
//设置某一相的值
static void SetPhaseState(uint32_t ch, PhaseState state, uint16_t duty)
{
    switch (state)
    {
    case PHASE_HIGH_PWM:
        Phase_HighPWM(ch, duty);
        break;

    case PHASE_LOW_ON:
        Phase_LowOn(ch);
        break;

    case PHASE_FLOAT:
    default:
        Phase_Off(ch);
        break;
    }
}

/**
功能：获取霍尔信号
*/
uint8_t Get_HALL_State(void)
{
	uint8_t hall_1;
	uint8_t hall_2;
	uint8_t hall_3;
	
	hall_1 = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6); //U
	hall_2 = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_7); //V
	hall_3 = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8); //W
	
	g_hall_state = (hall_1 << 2) | (hall_2 << 1) | hall_3;
	
	return g_hall_state;
}
/**
功能：实现六步换相
*/
static void Motor_Commutation(uint8_t hall_state, uint32_t pwm_duty)
{
	//电机没有运行
	if (WR_36SV61.state != RUN)
	{
		CDC_Transmit_FS((uint8_t *)"motor stop...\r\n", 15);
		return;
	}
	
	if (hall_state >= 8)
	{
		hall_state = 0;
	}

	//uint8_t out = hall_comm_table[state];
	WR_36SV61.duty = (pwm_duty >= MAX_PWM_DUTY) ? MAX_PWM_DUTY : pwm_duty;
	
	//设置PWM占空比
	switch (hall_state)
	{
		case 4: // Hall_UVW = 001: U+ V- W floating
			SetPhaseState(TIM_CHANNEL_1, 	PHASE_HIGH_PWM, 	WR_36SV61.duty); // U+
			SetPhaseState(TIM_CHANNEL_2, 	PHASE_LOW_ON, 		WR_36SV61.duty); // V-
			SetPhaseState(TIM_CHANNEL_3, 	PHASE_FLOAT,    	WR_36SV61.duty); // W floating
			break;

		case 5: // Hall_UVW = 101: U+ V floating W-
			SetPhaseState(TIM_CHANNEL_1, 	PHASE_HIGH_PWM, 	WR_36SV61.duty); // U+
			SetPhaseState(TIM_CHANNEL_2, 	PHASE_FLOAT,    	WR_36SV61.duty); // V floating
			SetPhaseState(TIM_CHANNEL_3, 	PHASE_LOW_ON, 		WR_36SV61.duty); // W-
			break;

		case 1: // Hall_UVW = 100: U floating V+ W-
			SetPhaseState(TIM_CHANNEL_1, 	PHASE_FLOAT,    	WR_36SV61.duty); // U floating
			SetPhaseState(TIM_CHANNEL_2, 	PHASE_HIGH_PWM, 	WR_36SV61.duty); // V+
			SetPhaseState(TIM_CHANNEL_3, 	PHASE_LOW_ON, 		WR_36SV61.duty); // W-
			break;

		case 3: // Hall_UVW = 110: U- V+ W floating
			SetPhaseState(TIM_CHANNEL_1, 	PHASE_LOW_ON, 		WR_36SV61.duty); // U-
			SetPhaseState(TIM_CHANNEL_2, 	PHASE_HIGH_PWM, 	WR_36SV61.duty); // V+
			SetPhaseState(TIM_CHANNEL_3, 	PHASE_FLOAT,    	WR_36SV61.duty); // W floating
			break;

		case 2: // Hall_UVW = 010: U- V floating W+
			SetPhaseState(TIM_CHANNEL_1, 	PHASE_LOW_ON, 		WR_36SV61.duty); // U-
			SetPhaseState(TIM_CHANNEL_2, 	PHASE_FLOAT,    	WR_36SV61.duty); // V floating
			SetPhaseState(TIM_CHANNEL_3, 	PHASE_HIGH_PWM, 	WR_36SV61.duty); // W+
			break;

		case 6: // Hall_UVW = 011: U floating V- W+
			SetPhaseState(TIM_CHANNEL_1, 	PHASE_FLOAT,    	WR_36SV61.duty); // U floating
			SetPhaseState(TIM_CHANNEL_2, 	PHASE_LOW_ON, 		WR_36SV61.duty); // V-
			SetPhaseState(TIM_CHANNEL_3, 	PHASE_HIGH_PWM, 	WR_36SV61.duty); // W+
			break;

		default:
			SetPhaseState(TIM_CHANNEL_1, 	PHASE_FLOAT, 		WR_36SV61.duty);
			SetPhaseState(TIM_CHANNEL_2, 	PHASE_FLOAT, 		WR_36SV61.duty);
			SetPhaseState(TIM_CHANNEL_3, 	PHASE_FLOAT, 		WR_36SV61.duty);
			break;
    }
	
}

void Motor_OnHallChanged()
{
    if (WR_36SV61.state != RUN)
    {
        return;
    }

    WR_36SV61.hall = Get_HALL_State();
	Motor_Commutation(WR_36SV61.hall, WR_36SV61.duty);
}

// 电机停止
static void Motor_Stop(void) {
	DRV8301_DISABLE;
	WR_36SV61.hall = 0;
	WR_36SV61.duty = 0;
	Motor_Commutation(WR_36SV61.hall, WR_36SV61.duty);

    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
	
	WR_36SV61.state = STOP;
}

// 电机启动
static void Motor_Start(void) {
	DRV8301_ENABLE;
	HAL_Delay(5);

	WR_36SV61.state = RUN;
	WR_36SV61.hall = Get_HALL_State();
	WR_36SV61.duty = BLDC_START_DUTY;
	
	Motor_Commutation(WR_36SV61.hall, WR_36SV61.duty);
}
void motor_init(void)
{
	// 检测DRV8301故障（低电平=故障）
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == 0)
	{
		Motor_Stop();
		CDC_Transmit_FS((uint8_t *)"DRV8301 error\r\n", 15);
		
		return;
	}

	Motor_Start();
}

MOTOR_OBJ* Get_Motor_struct(void)
{
	return &WR_36SV61;
}
