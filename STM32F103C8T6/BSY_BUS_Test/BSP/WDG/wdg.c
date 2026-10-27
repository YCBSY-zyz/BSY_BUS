#include "wdg.h"
#include "bsy_os.h"




//wdg喂狗线程优先级
#define   THREAD1  1




/***************************************************
* 初始化独立看门狗
* prer:分频数:0~7(只有低3位有效!)
* 分频因子=4*2^prer.但最大值只能是256!
* rlr:重装载寄存器值:低11位有效.
* 时间计算(大概):Tout=((4*2^prer)*rlr)/40 (ms).
***************************************************/
void IWDG_Init(u8 prer,u16 rlr) 
{	
 	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  //使能对寄存器IWDG_PR和IWDG_RLR的写操作
	
	IWDG_SetPrescaler(prer);  //设置IWDG预分频值:设置IWDG预分频值为64
	
	IWDG_SetReload(rlr);  //设置IWDG重装载值
	
	IWDG_ReloadCounter();  //按照IWDG重装载寄存器的值重装载IWDG计数器
	
	IWDG_Enable();  //使能IWDG
}


//喂独立看门狗
void IWDG_Feed(void)
{   
 	IWDG_ReloadCounter();//reload										   
}



//Wdg 运行程序
void Wdg_Run(void *pmt)
{
    pmt = pmt;
    IWDG_ReloadCounter();//reload
}

//创建任务结构体函数
RUN_Handle_Type WDG_Task_Handle;


//创建LED运行任务
void Task_Wdg(void)
{
    WDG_Task_Handle.R_CYCLE = 1000;      //1000ms读一次数据
    WDG_Task_Handle.NMS = 1000;          //装载延时计数值
    WDG_Task_Handle.R_STATUS = STOP;     //设置状态为停止态
    WDG_Task_Handle.LEVEL = THREAD1;     //设置该任务优先级为5
    WDG_Task_Handle.LOCK = false;       //0任务锁打开，可以执行该任务
    WDG_Task_Handle.pf = Wdg_Run;    //任务回调函数
    Task_Add(&WDG_Task_Handle);    
}





