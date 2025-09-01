
#include "main.h"
#include "motor.h"
#include "tim.h"

extern TIM_HandleTypeDef htim4;
// 모터의 상태 변화 만를기
typedef enum
{
	STOP,
	FRONT,
	BACK,
	RIGHT,
	LEFT
}MOTOR_STATE;

MOTOR_STATE motor_state = STOP;
static int32_t motor_tick = 0;

void Motor_control_state(void)
{
	switch (motor_state)
	{
		case STOP:
			if(HAL_GetTick() - motor_tick > 30)
			{
				motor_tick = HAL_GetTick();
				Motor_break_control(MOTOR_SPEED_0, MOTOR_SPEED_0);
				motor_state = FRONT;
			}
			break;

		case FRONT:
			// 이전 상태가 유지됨 halgettikc 초 만큼
			if(HAL_GetTick() - motor_tick > 30)
			{
				motor_tick = HAL_GetTick();
				Motor_front_control(MOTOR_SPEED_80, MOTOR_SPEED_80);
				motor_state = STOP;
			}
			break;

		case BACK:
			if(HAL_GetTick() - motor_tick > 30)
			{
				motor_tick = HAL_GetTick();
				Motor_back_control(MOTOR_SPEED_80, MOTOR_SPEED_80);
			}
			motor_state = STOP;
			break;

		case RIGHT:
			break;

		case LEFT:
			break;

		default:
			motor_state = STOP;
			break;
	}

}

static void motor_set_speed(uint16_t speed_left, uint16_t speed_right)
{
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, speed_left);  // 왼쪽 모터 속도
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, speed_right);  // 오른쪽 모터 속도
}

static void motor_set_direction(GPIO_PinState l_in1, GPIO_PinState l_in2, GPIO_PinState r_in3, GPIO_PinState r_in4)
{
	// HAL_GPIO_WritePin(GPIOx, GPIO_Pin, PinState)
	HAL_GPIO_WritePin(MOTOR_LEFT_IN1_PORT, MOTOR_LEFT_IN1, l_in1);
	HAL_GPIO_WritePin(MOTOR_LEFT_IN2_PORT, MOTOR_LEFT_IN2, l_in2);
	HAL_GPIO_WritePin(MOTOR_RIGHT_IN3_PORT, MOTOR_RIGHT_IN3, r_in3);
	HAL_GPIO_WritePin(MOTOR_RIGHT_IN4_PORT, MOTOR_RIGHT_IN4, r_in4);

}


// Right Turn // 왼쪽이 정방향, 오른쪽이 역방향
void Motor_right_control(int speed_left, int speed_right)
{
	motor_set_speed(speed_left, speed_right);
	motor_set_direction(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET);
}

// Left Turn // 왼쪽이 역방향, 오른쪽이 정방향
void Motor_left_control(int speed_left, int speed_right)
{
	motor_set_speed(speed_left, speed_right);
	motor_set_direction(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET);
}

// Drive
void Motor_front_control(int speed_left, int speed_right)
{
	motor_set_speed(speed_left, speed_right);
	motor_set_direction(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET);
}

// Reverse
void Motor_back_control(int speed_left, int speed_right)
{
	motor_set_speed(speed_left, speed_right);
	motor_set_direction(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET);
}

// Break
void Motor_break_control(int speed_left, int speed_right)
{
	motor_set_speed(speed_left, speed_right);
	motor_set_direction(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET);
}

