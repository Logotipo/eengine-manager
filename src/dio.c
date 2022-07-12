#include <stm32f10x_gpio.h>
#include "usart.h"
#include "settings.h"

void InitGPIO(void){
	//configure out PINs
	GPIO_InitTypeDef GPIO_InitStructure;
#if !SVPWM
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 |
								  GPIO_Pin_5 | GPIO_Pin_11;
#else
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
#endif
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	////////////////
	//configure input PINs
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	////////////////
	//configure exti (overload signals)
	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = EXTI_Line11 | EXTI_Line12 | EXTI_Line13;// A, B, C phases
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;//from 0 to 1
	EXTI_Init(&EXTI_InitStruct);
	////////////////
	//configure exti interrupts
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	////////////////
}

void EXTI9_5_IRQHandler(void){
	if(EXTI_GetITStatus(EXTI_Line11) == SET){
		EXTI_ClearITPendingBit(EXTI_Line11);
		USART_SendOverLoad(1);// phase A
	}
	else if(EXTI_GetITStatus(EXTI_Line12) == SET){
		EXTI_ClearITPendingBit(EXTI_Line12);
		USART_SendOverLoad(2);// phase B
	}
	else if(EXTI_GetITStatus(EXTI_Line13) == SET){
		EXTI_ClearITPendingBit(EXTI_Line13);
		USART_SendOverLoad(3);// phase C
	}
}
