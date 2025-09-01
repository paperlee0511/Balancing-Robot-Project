#ifndef INC_ACCEL_I2C_H_
#define INC_ACCEL_I2C_H_

#include "main.h"
#include "stdio.h"
#include "stdbool.h"
#include "gpio.h"
#include "led.h"


#define CALIBRATION_SAMPLES 200

void mpu6050_communication();	// 전체 과정
void process_calibration_data();	// 캘리브레이션 데이터 처리
void process_normal_data();			// 정상데이터 처리(캘리브레이션 적용)

bool is_calibration_complete(void);
bool is_sensor_data_ready(void);
void clear_sensor_data_flag(void);

// 모터 테스트용
int16_t get_accel_x();
int16_t get_accel_y();
int16_t get_accel_z();
int16_t get_gyro_x();


#endif /* INC_ACCEL_I2C_H_ */
