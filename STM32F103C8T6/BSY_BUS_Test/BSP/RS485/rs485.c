#include "sys.h"		    
#include "rs485.h"	 
#include "bsy_os.h"
#include "bsy_bus.h"



#define     THREAD3     3



//定义485接收数据结构体
UART_REC_Type  RS485_RecData;


//注：这里只发送数据不接收数据
#ifdef EN_USART2_RX   	//如果使能了接收

void USART2_IRQHandler(void)
{
	u8 res;	    
 
 	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //接收到数据
	{	  
		res =USART_ReceiveData(USART2); 	            //读取接收到的数据
        
        BSY_BUS_recProcess(&RS485_RecData, res);        //使用BSY_BUS协议处理接收到的数据
	}  											 
} 
#endif

//初始化IO 串口2
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率	  
void RS485_Init(u32 bound)
{  
    GPIO_InitTypeDef GPIO_InitStructure;
  	USART_InitTypeDef USART_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//使能GPIOA,G时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART2时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				 //PA4端口配置 CS1_485
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	//PA2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  
    
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,ENABLE);//复位串口2
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,DISABLE);//停止复位
 
	

	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8位数据长度
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;///奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式

    USART_Init(USART2, &USART_InitStructure); ; //初始化串口
    
    
 #ifdef EN_USART2_RX		  	//如果使能了接收  
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; //使能串口2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //先占优先级3级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; //从优先级2级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
 
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
#endif

    USART_Cmd(USART2, ENABLE);                      //使能串口 
    RS485_TX_EN = 0;				                //设置为接收模式
    
}



//RS485发送len个字节.
//buf:发送区首地址
//len:发送的字节数(为了和本代码的接收匹配,这里建议不要超过64个字节)
void RS485_Send_Data(u8 *buf,u8 len)
{
	u8 t;
    
	RS485_TX_EN = 1;				//设置为发送模式
    
  	for(t = 0; t < len; t++)		//循环发送数据
	{		   
        while((USART2->SR & USART_FLAG_TXE) == RESET);
        USART2->DR = buf[t];
        while((USART2->SR & USART_FLAG_TC) == RESET);
	}	   
	RS485_TX_EN = 0;				//设置为接收模式	
}






//RS485运行程序，10MS扫描一次；处理数据接收及收到数据后的处理任务
//使用BSY_BUS协议接收数据
void RS485_Run(void *pmt)
{
    pmt = pmt;
    
    if(RS485_RecData.rec_ok != 0){      //检查有没有新收到的数据
        if(check_mod128(RS485_RecData.data, RS485_RecData.len) != 0){    //校验收到的数据
            //接收数据校验正确
            /***  拿到数据后在此处处理你要做的事情  Do something......  ***/
            RS485_Send_Data(RS485_RecData.data,RS485_RecData.len);   //返回收到的数据
        }
        RS485_RecData.rec_ok = 0;       //数据已处理，清除接收标志
    }
    
}


//创建任务结构体函数
RUN_Handle_Type RS485_Task_Handle;


//创建UART运行任务
void Task_RS485(void)
{
    RS485_Task_Handle.R_CYCLE = 10;           //10ms读一次数据
    RS485_Task_Handle.NMS = 10;               //装载延时计数值
    RS485_Task_Handle.R_STATUS = STOP;       //设置状态为停止态
    RS485_Task_Handle.LEVEL = THREAD3;       //设置该任务优先级为3
    RS485_Task_Handle.LOCK = false;          //0任务锁打开，可以执行该任务
    RS485_Task_Handle.pf = RS485_Run;         //任务回调函数
    Task_Add(&RS485_Task_Handle);            //添加任务到任务列表
}

















