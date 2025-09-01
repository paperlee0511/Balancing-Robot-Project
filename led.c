#include "led.h"


void red_led_on()
{
	HAL_GPIO_WritePin(LED_PORT, RED, ON);
}

void green_led_on()
{
	HAL_GPIO_WritePin(LED_PORT, GREEN, ON);
}


void red_led_off()
{
	HAL_GPIO_WritePin(LED_PORT, RED, OFF);
}

void green_led_off()
{
	HAL_GPIO_WritePin(LED_PORT, GREEN, OFF);
}

