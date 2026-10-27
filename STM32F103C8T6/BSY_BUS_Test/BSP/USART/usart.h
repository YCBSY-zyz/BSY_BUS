#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"

#define USART1_REC_MAXLEN 128	//最大接收数据长度



void USART1_Init_Config(u32 bound);
void UART1_SendString(char* s);

//void UART1_SendLR(void);
//void UART2_SendLR(void);

//串口1发送回车换行
#define UART1_SendLR() UART1_SendString("\r\n")

void UART1_SendDatas(u8* dat, u8 n);
void USART1_Receive_Data(u8 *buf, u8 *len);
void Task_Uart(void);


#endif


