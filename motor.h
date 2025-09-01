#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include "main.h"

#define MOTOR_LEFT_IN1 GPIO_PIN_6 // PC6포트
#define MOTOR_LEFT_IN2 GPIO_PIN_7 // PC7포트

#define MOTOR_RIGHT_IN3 GPIO_PIN_10 // PC10포트
#define MOTOR_RIGHT_IN4 GPIO_PIN_11 // PC11포트


#define MOTOR_LEFT_IN1_PORT GPIOC 	// PC6
#define MOTOR_LEFT_IN2_PORT GPIOC	// PC7

#define MOTOR_RIGHT_IN3_PORT GPIOC
#define MOTOR_RIGHT_IN4_PORT GPIOC

#define MOTOR_SPEED_100 100
#define MOTOR_SPEED_99 99
#define MOTOR_SPEED_90 90
#define MOTOR_SPEED_80 80
#define MOTOR_SPEED_70 70
#define MOTOR_SPEED_60 60
#define MOTOR_SPEED_50 50
#define MOTOR_SPEED_40 40
#define MOTOR_SPEED_30 30
#define MOTOR_SPEED_20 20
#define MOTOR_SPEED_10 10
#define MOTOR_SPEED_0 0

void Motor_right_control(int speed_left, int speed_right);
void Motor_left_control(int speed_left, int speed_right);
void Motor_front_control(int speed_left, int speed_right);
void Motor_back_control(int speed_left, int speed_right);
void Motor_break_control(int speed_left, int speed_right);
void Motor_control_state(void);


#endif /* INC_MOTOR_H_ */
