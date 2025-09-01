/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "stdbool.h"
#include "math.h"
#include "delay.h"
#include "motor.h"
#include "string.h"
#include "accel_i2c.h"
#include "led.h"


// 디버깅용 전역변수 선언
 volatile int16_t live_accel_x = 0;
 volatile int16_t live_accel_y = 0;
 volatile int16_t live_accel_z = 0;
 volatile int16_t live_gyro_x = 0;
 volatile float live_rate = 0.0;
 volatile float live_pitch = 0.0;
 volatile float live_predicted_pitch = 0.0;

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

// printf로 출력할 내용을 USART로 내보내 주는 재정의 함수
#ifdef __GNUC__
/* With GCC small printf (option LD Linker->Libraries->Small printf
 * set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int  __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int  fputc(int ch, FILE *f)
#endif /* __GNUC__*/

/** @brief Retargets the C library printf function to the USART.
 *  @param None
 *  @retval None
 */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART2 and Loop
     until the end of transmission */
  if(ch == '\n')
  {
    HAL_UART_Transmit(&huart2, (uint8_t*) "\r", 1, 0xFFFF);
  }
    HAL_UART_Transmit(&huart2, (uint8_t*) &ch, 1, 0xFFFF);
}


/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */

  // pwm start
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);

//  MPU6050_Init_checking(&hi2c1);
//  MPU6050_Init(&hi2c1);


// 읽기 제어 시간적 분리
// 센서 읽기
  static uint32_t last_sensor_tick = 0;
// 모터제어
  static uint32_t last_motor_tick =0;


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(HAL_GetTick() - last_sensor_tick > 5)
	  {
		  last_sensor_tick = HAL_GetTick();
		  if(!is_calibration_complete())
		  {
			 mpu6050_communication();
		  }
		  else
		  {
			 mpu6050_communication();
			 if(is_sensor_data_ready())
			 {
				 // 테스트용 accel_x 가져오기,
				 int16_t corrected_accel_x = get_accel_x();
				 int16_t corrected_accel_y = get_accel_y();
				 int16_t corrected_accel_z = get_accel_z();
				 int16_t corrected_gyro_x  = get_gyro_x();

				 // 정확한 pitch 각도 계산(도 단위)
				 float pitch = atan2(-corrected_accel_x,
		                   sqrt(corrected_accel_y * corrected_accel_y +
		                        corrected_accel_z * corrected_accel_z)) * 180.0 / M_PI;

				 float rate = corrected_gyro_x / 131.0; // 각속도(도/초)
				 float predicted_pitch = pitch + (rate * 0.10);  // 50ms 예측

				 // 미래예측
				 int16_t future_angle = corrected_accel_x + (corrected_gyro_x /5); // 20은 튜닝 계수

				 //디버깅용 전역 변수 업데이트
				 live_accel_x = corrected_accel_x;
				 live_accel_y = corrected_accel_y;
				 live_accel_z = corrected_accel_z;
				 live_gyro_x = corrected_gyro_x;
				 live_rate = rate;
				 live_pitch = pitch;
				 live_predicted_pitch = predicted_pitch;

				 if(HAL_GetTick() - last_motor_tick >= 5)
				 {
					 last_motor_tick = HAL_GetTick();

				        if(live_predicted_pitch  > 0.0 && live_predicted_pitch < 3.0)
				        {
				            Motor_front_control(MOTOR_SPEED_50, MOTOR_SPEED_50);
				        }
				        else if(live_predicted_pitch > 3.1 && live_predicted_pitch < 9.0)
				        {
				        	 Motor_front_control(MOTOR_SPEED_60, MOTOR_SPEED_60);
				        }
				        else if(live_predicted_pitch > 9.1 && live_predicted_pitch < 15.0)
				        {
				        	 Motor_front_control(MOTOR_SPEED_90, MOTOR_SPEED_90);
				        }
				        else if(live_predicted_pitch > 15.1 && live_predicted_pitch < 30.0)
				        {
				        	 Motor_front_control(MOTOR_SPEED_99, MOTOR_SPEED_99);
				        }

				        else if(live_predicted_pitch  < 0.0 && live_predicted_pitch > -3.0)
				        {
				        	Motor_back_control(MOTOR_SPEED_50, MOTOR_SPEED_50);
				        }
				        else if(live_predicted_pitch  < -3.1 && live_predicted_pitch > -9.0)
				        {
				        	Motor_back_control(MOTOR_SPEED_50, MOTOR_SPEED_50);
				        }
				        else if(live_predicted_pitch  < -9.1 && live_predicted_pitch > -15.0)
				        {
				        	Motor_back_control(MOTOR_SPEED_50, MOTOR_SPEED_50);
				        }
				        else if(live_predicted_pitch  < -15.1 && live_predicted_pitch > -30.0)
				        {
				        	Motor_back_control(MOTOR_SPEED_50, MOTOR_SPEED_50);
				        }


				 }
				 clear_sensor_data_flag();

			 }

		  }
	  }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
