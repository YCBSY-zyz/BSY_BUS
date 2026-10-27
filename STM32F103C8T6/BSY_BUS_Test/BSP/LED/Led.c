/**********************************************************************************
 * 文件名  ：led.c
 * 描述    ：led 应用函数库         
 * 硬件连接：  PB5 -> LED0     
 *               
 * 库版本  ：ST_v3.5
**********************************************************************************/

#include "led.h"
#include "bsy_os.h"
#include "stdio.h"


#define  THREAD0 0



u8  ledflag = 0;


/*******************************************************************************
* 函数名  : Led_Config
* 描述    : LED0
* 输入    : 无
* 输出    : 无
* 返回    : 无 
* 说明    : LED0设置的引脚为:PB5
*******************************************************************************/
void Led_Config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;				//定义一个GPIO_InitTypeDef类型的GPIO初始化结构体
	
	RCC_APB2PeriphClockCmd(LED_RCC, ENABLE);			//使能GPIOB的外设时钟	
	
	GPIO_InitStructure.GPIO_Pin = LED0;	//选择要初始化的GPIOB引脚(PB5)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//设置引脚工作模式为通用推挽输出 		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//设置引脚输出最大速率为50MHz
	GPIO_Init(LED_PORT, &GPIO_InitStructure);			//调用库函数中的GPIO初始化函数，初始化GPIOB中的PB5引脚

	LED0_OFF();										//关闭LED0  					 
}

/*点亮LED0.PB5*/
void LED0_ON(void) 
{
	GPIO_SetBits(LED_PORT,LED0);
}

/*关闭LED0.PB5*/
void LED0_OFF(void)
{
	GPIO_ResetBits(LED_PORT,LED0);          //GPIO_SetBits
}


u8  cnt = 0;

//LED运行程序
void Led_Run(void *pmt)
{
    //pmt = pmt;
    
    if(cnt >= 20){              //每闪烁10次，改变一次闪烁频率  5Hz/1Hz/0.5Hz
        cnt = 0;
        ledflag++;
        if(ledflag > 2)   ledflag = 0; 
    }else{
        cnt++;    
    }
    
    if((cnt % 2) != 0) LED0_ON();       //LED亮
    else    LED0_OFF();                 //LED灭
    
    if(ledflag == 0){           //如果状态过多采用状态机的方式，每一个动作一个状态，每运行一次执行一个状态，并且可以改变运行周期
        //此处使用默认扫描周期100ms   频率5HZ
    }else if(ledflag == 1){
        Task_Delay_ms(500);        //此处动态使用500MS扫描周期， 频率1HZ
    }else if(ledflag == 2){
        Task_Delay_ms(1000);       //此处动态使用1000MS扫描周期， 频率0.5HZ
    }
}

//创建任务结构体函数
RUN_Handle_Type LED_Task_Handle;


//创建LED运行任务
void Task_Led(void)
{
    LED_Task_Handle.R_CYCLE = 100;      //100ms读一次数据
    LED_Task_Handle.NMS = 100;          //装载延时计数值
    LED_Task_Handle.R_STATUS = STOP;    //设置状态为停止态
    LED_Task_Handle.LEVEL = THREAD0;    //设置该任务优先级为5
    LED_Task_Handle.LOCK = false;       //0任务锁打开，可以执行该任务
    LED_Task_Handle.pf = Led_Run;       //任务回调函数
    //LED_Task_Handle.pmt = ?;          //需要参数时，可再此传入
    Task_Add(&LED_Task_Handle);    
}


//删除任务，不能在本任务运行时将本任务删除
void  Del_Task_Led (void)
{
    Stud_Type * s;                  //定义链表指针
    s = Get_Node(THREAD0);          //查找当前线程的任务指针
    if(s != NULL){                  //如果当前指针存在
        Task_Del(s);                //将当前任务删除
    }
}

//是否禁止该任务运行  0  允许运行（unlock）， 1 禁止运行（lock）
void Locked_Task_Led (u8 sta)
{
    if(sta != 0)  LED_Task_Handle.LOCK = 1;
    else LED_Task_Handle.LOCK = 0;
}


