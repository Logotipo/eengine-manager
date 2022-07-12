#ifndef USART_H
#define USART_H
#include <stdint.h>

void UART_Init(void);

void USART_SendSpeed(double speed);
void USART_SendAngle(double angle);
void USART_SendOverLoad(uint8_t channel);

#endif
