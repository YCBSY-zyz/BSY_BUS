/**********************************************************************************
 * 文件名  ：usart.c
 * 描述    ：usart1 应用函数库          
 * 硬件连接：TXD(PB9)  -> 外部串口RXD     
 *           RXD(PB10) -> 外部串口TXD      
 *           GND	   -> 外部串口GND 
 * 库版本  ：ST_v3.5
**********************************************************************************/

#include "usart.h"
#include "bsy_os.h"
#include <stdio.h>
#include "sys.h"

#define  THREAD2 2

u8 USART1_RX_BUF[USART1_REC_MAXLEN];	//串口1数据接收缓冲区
u8 USART1_RX_CNT = 0;                   //串口接收数据计数器

u8 rec_buf[USART1_REC_MAXLEN] = {0};    //接收到的数据
u8 len = 0;                             //最终接收的数据长度                    
u8 rxlen = 0;                           //已接收到的数据长度
u8 recFlag = 0;

struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
   
int fputc(int ch, FILE *f)
{      
	USART_ClearFlag(USART1, USART_FLAG_TC);  // 关键：清除TC标志, 否则第一次打印时第一个字符丢失
    USART_SendData(USART1,(uint8_t)ch); 
    while(USART_GetFlagStatus(USART1,USART_FLAG_TC) == RESET){}
	return ch;
}




/*******************************************************************************
* 函数名  : USART1_Init_Config
* 描述    : USART1初始化配置
* 输入    : bound：波特率(常用：2400、4800、9600、19200、38400、115200等)
* 输出    : 无
* 返回    : 无 
* 说明    : 无
*******************************************************************************/
void USART1_Init_Config(u32 bound)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;

	/*使能USART1和GPIOA外设时钟*/  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);	

	/*复位串口1*/
	USART_DeInit(USART1);  

	/*USART1_GPIO初始化设置*/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;			//USART1_TXD(PA.9)     
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//设置引脚输出最大速率为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);				//调用库函数中的GPIO初始化函数，初始化USART1_TXD(PA.9)  


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				//USART1_RXD(PA.10)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//调用库函数中的GPIO初始化函数，初始化USART1_RXD(PA.10)


	/*USART1 初始化设置*/
	USART_InitStructure.USART_BaudRate = bound;										//设置波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//1个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//工作模式设置为收发模式
	USART_Init(USART1, &USART_InitStructure);										//初始化串口1

	/*Usart1 NVIC配置*/
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;	//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//从优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							//根据指定的参数初始化VIC寄存器 

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);			//使能串口1接收中断

	USART_Cmd(USART1, ENABLE);                    			//使能串口 
	USART_ClearFlag(USART1, USART_FLAG_TC);					//清除发送完成标志
}


/*******************************************************************************
* 函数名  : UART1_SendString
* 描述    : USART1发送字符串
* 输入    : *s字符串指针
* 输出    : 无
* 返回    : 无 
* 说明    : 无
*******************************************************************************/
void UART1_SendString(char* s)
{
	//USART_ClearFlag(USART1, USART_FLAG_TC);					//清除发送完成标志
	
	while((*s) != 0)//检测字符串结束符
	{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)==RESET); 
		
		USART_SendData(USART1 , *s++);//发送当前字符
		//s++;
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET);
		USART_ClearFlag(USART1, USART_FLAG_TC);					//清除发送完成标志
		
	}
	
}



/*******************************************************************************
* 函数名  : UART2_SendDatas
* 描述    : USART2发送数组数据
* 输入    : *dat 发送数据的数组指针
* 输出    : 无
* 返回    : 无 
* 说明    : 无
*******************************************************************************/
void UART1_SendDatas(u8* dat, u8 n)
{
	int i = 0;
	//USART_ClearFlag(USART2, USART_FLAG_TC);					//清除发送完成标志
	
	for(i = 0; i < n; i++)//检测字符串结束符
	{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)==RESET);
		
		USART_SendData(USART1 , dat[i]);//发送当前字符
		//s++;
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET);
		USART_ClearFlag(USART1, USART_FLAG_TC);					//清除发送完成标志
	}
}


/*******************************************************************************
* 函数名  : USART1_IRQHandler
* 描述    : 串口1中断服务程序
* 输入    : 无
* 返回    : 无 
* 说明    : 1)、只启动了USART1中断接收，未启动USART1中断发送。
*           2)、接收到0x0d 0x0a(回车、"\r\n")代表帧数据接收完成
*******************************************************************************/
void USART1_IRQHandler(void)                	
{
	u8 Res = 0;
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){
		//USART_ClearITPendingBit(USART1,USART_IT_RXNE);		//后加，清除中断标志位
		Res = USART_ReceiveData(USART1);
		if(USART1_RX_CNT < 64)
		{
			USART1_RX_BUF[USART1_RX_CNT] = Res;		//记录接收到的值
			USART1_RX_CNT++;						//接收数据增加1 
		} 
	} 
	
} 


//USART1查询接收到的数据
//buf:接收缓存首地址
//len:读到的数据长度
/****************************
void USART1_Receive_Data(u8 *buf, u8 *len)
{
	u8 i = 0;
	*len = 0;				//默认为0
	delay_ms(10);		//等待10ms,连续超过10ms没有接收到一个数据,则认为接收结束
	if(rxlen == USART1_RX_CNT && rxlen)//接收到了数据,且接收完成了
	{
		for(i = 0; i < rxlen; i++)
		{
			buf[i] = USART1_RX_BUF[i];	
		}		
		*len = USART1_RX_CNT;	//记录本次数据长度
		USART1_RX_CNT = 0;		//清零
	}
    rxlen = USART1_RX_CNT;      //记录已接收数据长度
}
**********************************/


//Uart运行程序，5MS扫描一次；处理数据接收及收到数据后的处理任务
//完全不用使用delay_ms函数，提高运行效率，如果数据量比较大，调整扫描周期就可以了 So easy!
void Uart_Run(void *pmt)
{
    pmt = pmt;
    u8 i = 0;
    rxlen = USART1_RX_CNT;          //记录已接收到的数据长度
    //5MS 扫描检测一次接收到的数据长度，如果数据长度与上次一样，则认为接收完成
    if(rxlen == USART1_RX_CNT && rxlen){        //接收到新数据且长度不为0,  
        for(i = 0; i < USART1_REC_MAXLEN; i++)
		{
			rec_buf[i] = 0;	                    //清数据缓存
		}
        
        for(i = 0; i < rxlen; i++)
		{
			rec_buf[i] = USART1_RX_BUF[i];	    //拷贝数据
		}
        
        len = USART1_RX_CNT;	//记录本次数据长度
        USART1_RX_CNT = 0;		//清零
        recFlag = 1;            //标记已接收到新数据
    }
    rxlen = USART1_RX_CNT;          //记录已接收到的数据长度
    
    if(recFlag != 0){
        UART1_SendDatas(rec_buf, len);
        //-------------再此处理新接收到的数据----------------
        recFlag = 0;                //数据已处理，清除接收标识
    }
}


//创建任务结构体函数
RUN_Handle_Type UART_Task_Handle;


//创建UART运行任务
void Task_Uart(void)
{
    UART_Task_Handle.R_CYCLE = 5;           //5ms读一次数据
    UART_Task_Handle.NMS = 5;               //装载延时计数值
    UART_Task_Handle.R_STATUS = STOP;       //设置状态为停止态
    UART_Task_Handle.LEVEL = THREAD2;       //设置该任务优先级为5
    UART_Task_Handle.LOCK = false;          //0任务锁打开，可以执行该任务
    UART_Task_Handle.pf = Uart_Run;         //任务回调函数
    Task_Add(&UART_Task_Handle);            //添加任务到任务列表
}





