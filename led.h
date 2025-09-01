#ifndef INC_LED_H_
#define INC_LED_H_

#include "main.h"
#include "gpio.h"

#define LED_PORT 	GPIOC
#define GREEN		GPIO_PIN_1
#define RED			GPIO_PIN_2
#define ON			GPIO_PIN_SET
#define OFF			GPIO_PIN_RESET

void red_led_on();
void green_led_on();
void red_led_off();
void green_led_off();


#endif /* INC_LED_H_ */
