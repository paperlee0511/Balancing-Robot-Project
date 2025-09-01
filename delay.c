

#include "delay.h"


// 논블러킹으로 만들어 보기
volatile uint8_t delay_us_flag =0;

// 딜레이 시작 함수(비동기 시작) ,  Output Compare 모드로 작동
// delay_us_flag는 딜레이 완료 여부를 비동기적으로 표시하는 플래그

// us마이크로초 후 tim3의 ch1 비교일치 인터럽트를 발생시키는 설정을 수행
void delay_us(uint16_t us)
{
	delay_us_flag =0;	// 딜레이가 아직 완료되지 않았음을 표시하기 위해 초기화
	//tim3의 현재 카운터 값을 0으로 초기화, 이후 카운터가 us에 도달하면 인터럽트 발생
	__HAL_TIM_SET_COUNTER(&htim3, 0);
	// tim3의 ch1에 대한 비교 레지스터 ccr1에 us값을 설정, 타이머가 0->1->2... ->us까지 증가하면 ccr1 == cnt 일치 발생
	// 이 일치가 발생하면 인터럽트 요청 발생
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, us); //us 후에 인터럽트 발생
	// Output Compare(OC) 모드에서 타이머 인터럽트를 Enable
	HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1);	// OC 인터럽트 시작
}

// 인터럽트 콜백 함수
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM3)
	{
		delay_us_flag =1;	// 완료 플래그 설정
		HAL_TIM_OC_Stop_IT(htim, TIM_CHANNEL_1);

	}
}

//딜레이 상태 확인

//void delay_us(uint16_t us)
//{
//	__HAL_TIM_SET_COUNTER(&htim3, 0); 	// 카운터값을 셋 하는 함수
//	while((__HAL_TIM_GET_COUNTER(&htim3)) < us);	// 카운터... 뭘 하는거였지 // 이것이 딜레이 만드는 함수
//
//}

