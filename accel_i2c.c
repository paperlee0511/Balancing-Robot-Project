/*
 * accel_i2c.c - 통합 상태머신으로 캘리브레이션 포함
 */

#include "accel_i2c.h"


// 통합 상태머신
typedef enum
{
    I2C_CHECK,				// 물리적 연결 확인
    I2C_INIT,				// init
    I2C_CALIBRATION,        // 캘리브레이션 상태 추가
    I2C_CALIB_READING,      // 캘리브레이션 데이터 읽는 중
    I2C_NORMAL_OPERATION,   // 정상 동작
    I2C_NORMAL_READING      // 정상 데이터 읽는 중
}I2C_STATE;

// i2c 통신 값 세팅
extern I2C_HandleTypeDef hi2c1;
I2C_STATE i2c_state = I2C_CHECK;
uint8_t mpu6050_addr = 0x68<<1;

// 비동기 통신용 변수들
uint8_t who_am_i_data = 0;
uint8_t who_am_i_reg = 0x75;
uint8_t mpu6050_sleep_out = 0x00;
uint8_t mpu6050_reg = 0x6B;
uint8_t sensor_data[14];
uint8_t accel_start_reg = 0x3B;

// 캘리브레이션 관련 변수
uint16_t calib_count = 0;	// 캘리브레이션 합 횟수

// 캘리브레이션 진행 변수
int32_t accel_x_sum = 0, accel_y_sum = 0, accel_z_sum = 0;
int32_t gyro_x_sum = 0, gyro_y_sum = 0, gyro_z_sum = 0;

// 캘리브레이션 오프셋 (최종 결과)
int16_t accel_x_offset = 0, accel_y_offset = 0, accel_z_offset = 0;
int16_t gyro_x_offset = 0, gyro_y_offset = 0, gyro_z_offset = 0;

// 데이터값 읽기 플래그
bool sensor_data_ready = false;

// 테스트용 센서값 저장용 전역 변수
static int16_t geted_accel_x =0;
static int16_t geted_accel_y =0;
static int16_t geted_accel_z =0;


static int16_t geted_gyro_x =0;




bool is_calibration_complete(void)
{
	return(i2c_state == I2C_NORMAL_OPERATION);
}

bool is_sensor_data_ready(void)
{
	return sensor_data_ready;
}

void clear_sensor_data_flag(void)
{
	sensor_data_ready = false;
}

void mpu6050_communication()
{
    switch (i2c_state)
    {
        case I2C_CHECK:		// WHO_AM_I 읽기 요청
            printf("Hardware wiring checking ...\n\r");
            HAL_I2C_Mem_Read_IT(&hi2c1, mpu6050_addr, who_am_i_reg,
                    I2C_MEMADD_SIZE_8BIT, &who_am_i_data, 1);
            break;

        case I2C_INIT:		// SLEEP 해제
            printf("MPU6050 initializing ...\n\r");
            HAL_I2C_Mem_Write_IT(&hi2c1, mpu6050_addr, mpu6050_reg,
                    I2C_MEMADD_SIZE_8BIT, &mpu6050_sleep_out, 1);
            break;

        case I2C_CALIBRATION:		// 센서 고정하고 데이터 누적 시작.
            if (calib_count == 0)
            {
                printf("Starting calibration... Keep sensor still!\n\r");
                // 합계 초기화
                accel_x_sum = accel_y_sum = accel_z_sum = 0;
                gyro_x_sum = gyro_y_sum = gyro_z_sum = 0;
                red_led_on();
                green_led_off();
            }

            printf("Calibration: %d/%d\n\r", calib_count + 1, CALIBRATION_SAMPLES);

            // 캘리브레이션용 데이터 읽기
            HAL_I2C_Mem_Read_IT(&hi2c1, mpu6050_addr, accel_start_reg,
                    I2C_MEMADD_SIZE_8BIT, sensor_data, 14);
            i2c_state = I2C_CALIB_READING;
            break;

        case I2C_CALIB_READING:
            // 콜백에서 처리됨
            break;

        case I2C_NORMAL_OPERATION:
        	// i2c가 사용중이 아닐때만 새로운 요청
        	if(hi2c1.State == HAL_I2C_STATE_READY)
        	{
                // 정상 동작 - 센서 데이터 읽기
                HAL_I2C_Mem_Read_IT(&hi2c1, mpu6050_addr, accel_start_reg,
                        I2C_MEMADD_SIZE_8BIT, sensor_data, 14);
                i2c_state = I2C_NORMAL_READING;
        	}

            break;

        case I2C_NORMAL_READING:
            // 콜백에서 처리됨
            break;

        default:
            break;
    }
}

// 캘리브레이션 데이터 처리
void process_calibration_data()
{
    int16_t accel_x = (int16_t)((sensor_data[0] << 8) | sensor_data[1]);
    int16_t accel_y = (int16_t)((sensor_data[2] << 8) | sensor_data[3]);
    int16_t accel_z = (int16_t)((sensor_data[4] << 8) | sensor_data[5]);

    int16_t gyro_x = (int16_t)((sensor_data[8] << 8) | sensor_data[9]);
    int16_t gyro_y = (int16_t)((sensor_data[10] << 8) | sensor_data[11]);
    int16_t gyro_z = (int16_t)((sensor_data[12] << 8) | sensor_data[13]);

    // 합계에 추가
    accel_x_sum += accel_x;
    accel_y_sum += accel_y;
    accel_z_sum += accel_z;

    gyro_x_sum += gyro_x;
    gyro_y_sum += gyro_y;
    gyro_z_sum += gyro_z;

    calib_count++;

    if (calib_count >= CALIBRATION_SAMPLES)
    {
        // 캘리브레이션 완료 - 평균 계산
        accel_x_offset = accel_x_sum / CALIBRATION_SAMPLES;
        accel_y_offset = accel_y_sum / CALIBRATION_SAMPLES;
        accel_z_offset = (accel_z_sum / CALIBRATION_SAMPLES) - 16384; // Z축은 중력 보정

        gyro_x_offset = gyro_x_sum / CALIBRATION_SAMPLES;
        gyro_y_offset = gyro_y_sum / CALIBRATION_SAMPLES;
        gyro_z_offset = gyro_z_sum / CALIBRATION_SAMPLES;

        printf("Calibration Complete!\n\r");
        printf("Accel Offset: X=%d, Y=%d, Z=%d\n\r",
               accel_x_offset, accel_y_offset, accel_z_offset);
        printf("Gyro Offset: X=%d, Y=%d, Z=%d\n\r",
               gyro_x_offset, gyro_y_offset, gyro_z_offset);

        i2c_state = I2C_NORMAL_OPERATION;
        calib_count = 0; // 리셋
        red_led_off();
        green_led_on();
    }
    else
    {
        i2c_state = I2C_CALIBRATION; // 다음 샘플
    }
}

// 정상 데이터 처리 (캘리브레이션 적용)
void process_normal_data()
{
    int16_t accel_x_raw = (int16_t)((sensor_data[0] << 8) | sensor_data[1]);
    int16_t accel_y_raw = (int16_t)((sensor_data[2] << 8) | sensor_data[3]);
    int16_t accel_z_raw = (int16_t)((sensor_data[4] << 8) | sensor_data[5]);

    int16_t gyro_x_raw = (int16_t)((sensor_data[8] << 8) | sensor_data[9]);
    int16_t gyro_y_raw = (int16_t)((sensor_data[10] << 8) | sensor_data[11]);
    int16_t gyro_z_raw = (int16_t)((sensor_data[12] << 8) | sensor_data[13]);

    // 캘리브레이션 적용
    int16_t accel_x = accel_x_raw - accel_x_offset;
    int16_t accel_y = accel_y_raw - accel_y_offset;
    int16_t accel_z = accel_z_raw - accel_z_offset;

    int16_t gyro_x = gyro_x_raw - gyro_x_offset;
    int16_t gyro_y = gyro_y_raw - gyro_y_offset;
    int16_t gyro_z = gyro_z_raw - gyro_z_offset;


    printf("Calibrated - Accel: X=%d, Y=%d, Z=%d\n\r", accel_x, accel_y, accel_z);
    printf("Calibrated - Gyro: X=%d, Y=%d, Z=%d\n\r", gyro_x, gyro_y, gyro_z);

    //테스트용 accel_x
    geted_accel_x = accel_x_raw - accel_x_offset;
    geted_accel_y = accel_y_raw - accel_y_offset;
    geted_accel_z = accel_z_raw - accel_z_offset;


    geted_gyro_x = gyro_x_raw - gyro_x_offset;

    sensor_data_ready = true;
    i2c_state = I2C_NORMAL_OPERATION; // 다음 읽기
}

// 테스트용 accel_x 반환
int16_t get_accel_x(){return geted_accel_x;}
int16_t get_accel_y(){return geted_accel_y;}
int16_t get_accel_z(){return geted_accel_z;}
int16_t get_gyro_x(){return geted_gyro_x;}

// I2C 메모리 읽기 완료 콜백
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        switch (i2c_state)
        {
            case I2C_CHECK:
                printf("MPU6050 detected : WHO_AM_I : 0x%02X\n\r", who_am_i_data);
                if (who_am_i_data == 0x68)
                {
                    i2c_state = I2C_INIT;
                }
                else
                {
                    printf("MPU6050 check failed\n\r");
                    i2c_state = I2C_CHECK;
                }
                break;

            case I2C_CALIB_READING:
                process_calibration_data();
                break;

            case I2C_NORMAL_READING:
                process_normal_data();
                break;

            default:
                break;
        }
    }
}

// I2C 메모리 쓰기 완료 콜백
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        switch (i2c_state)
        {
            case I2C_INIT:
                printf("MPU6050, I2C INIT COMPLETE\n\r");
                i2c_state = I2C_CALIBRATION; // 초기화 후 캘리브레이션 시작
                break;

            default:
                break;
        }
    }
}

// I2C 에러 콜백
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        printf("I2C Error in state: %d\n\r", i2c_state);

        switch (i2c_state)
        {
            case I2C_CHECK:
                i2c_state = I2C_CHECK;
                break;

            case I2C_INIT:
                i2c_state = I2C_INIT;
                break;

            case I2C_CALIB_READING:
            	printf("i2c_calib_reading_error\n");
                i2c_state = I2C_CALIBRATION; // 다시 시도
                break;

            case I2C_NORMAL_READING:
            	printf("i2c_normal_reading_error\n");
                i2c_state = I2C_NORMAL_OPERATION; // 다시 시도

                break;

            default:
                break;
        }
    }
}
