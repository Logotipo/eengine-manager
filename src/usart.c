#include <stm32f10x_usart.h>
#include <stm32f10x_GPIO.h>
#include <stm32f10x_rcc.h>
#include <string.h>
#include "settings.h"

extern double set_motor_speed;
extern double set_motor_angle;
extern uint8_t manage_mode;
extern uint8_t hold_reset_overloaded_channel;

static char rxBuffer[5];
uint8_t rxBufferOffset = 0;

void UART_Init(void){
	//enable clocking USART and PORTA
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);
	//NVIC enable
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	/////////////
	//configure PINs
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;//TX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	////////////////
	//configure UART
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//receive and transmit
	USART_Init(USART1, &USART_InitStructure);
	////////////////
	USART_Cmd(USART1, ENABLE);
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//enable iterrupt to receive
}

void USART_TxHandler(void){
	if(rxBuffer[0] == 'B' && rxBuffer[1] == 'L' && rxBuffer[2] == 'K'){
		GPIO_WriteBit(GPIOA, GPIO_Pin_14, Bit_SET);//unlock overloaded channel
		hold_reset_overloaded_channel = HOLD_RESET_OVR_TACTS;
	}
	else if(rxBuffer[0] == 'S' && rxBuffer[1] == 'P' && rxBuffer[2] == 'D')
		set_motor_speed = (rxBuffer[3]/255) * MAX_SPEED;
	else if(rxBuffer[0] == 'M' && rxBuffer[1] == 'M' && rxBuffer[2] == 'D')
		manage_mode = rxBuffer[3];
	else if(rxBuffer[0] == 'A' && rxBuffer[1] == 'N' && rxBuffer[2] == 'G')
		set_motor_angle = (rxBuffer[3]/255) * D_PI;
	
	rxBufferOffset = 0;
	memset((void *)rxBuffer, 0, sizeof(rxBuffer));//reset rx buffer
}

//USART iterrupt handler
void USART1_IRQHandler(void){
	
	if((USART1->SR & USART_FLAG_RXNE) != RESET){
		uint8_t symbol = USART_ReceiveData(USART1);//get symbol
		if(symbol != 0)
			rxBuffer[rxBufferOffset++] = symbol;
		else
			USART_TxHandler();
	}
}
static inline void SendDoubleData(double data){
	char *pointer = (char *)&data;
	for(uint8_t i = 0; i < sizeof(double); i++)
		USART_SendData(USART1, *(pointer++));
}
void USART_SendSpeed(double speed){
	const char command[] = "SPD\0";
	uint8_t i = 0;
	while(command[i] != 0){
		USART_SendData(USART1, command[i]);
		i++;
	}
	SendDoubleData(speed);
}
void USART_SendAngle(double angle){
	const char command[] = "ANG\0";
	uint8_t i = 0;
	while(command[i] != 0){
		USART_SendData(USART1, command[i]);
		i++;
	}
	SendDoubleData(angle);
}
///
/// channel:
/// 1 - A, 2 - B, 3 - C
void USART_SendOverLoad(uint8_t channel){
	const char command[] = "OVR\0";
	uint8_t i = 0;
	while(command[i] != 0){
		USART_SendData(USART1, command[i]);
		i++;
	}
	USART_SendData(USART1, channel);
}
