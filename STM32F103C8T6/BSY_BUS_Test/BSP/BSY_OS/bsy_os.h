#ifndef  __BSY_OS_H
#define  __BSY_OS_H

#include "sys.h"
#include "stdbool.h"
#include "stdlib.h"

//定义运行状态
typedef enum{
    STOP = 0,           //停止态
    OK = 1,             //就绪态
    RUN = 2             //运行态
}RunSta_Type;


//运行任务结构体定义
typedef struct{
    u16                 R_CYCLE;    //执行周期，每r_cycle个ms执行一次,默认参数，不可变
    u16                 NMS;        //延时减计数值，值可变，动态加载R_CYCLE或V_NMS
    RunSta_Type         R_STATUS;   //运行状态
    u8                  LEVEL;      //运行级别 0 --- 255，越小优先级越高，不能重复顺序排列，最大值根据实际调整
    bool                LOCK;       //运行锁，false 禁止运行， true 允许运行
    void  *pmt;                     //要传入的参数
    void (*pf)(void *pmt);          //运行任务函数，无参无返回。                             
}RUN_Handle_Type;



//程序双向链表数据结构定义
typedef struct node{
	uint8_t id;					//节点id，等于线程LEVEL			
	RUN_Handle_Type *task;		//任务句柄
	struct node *pre;			//前一个节点地址
	struct node *next;			//后一个节点地址
}Stud_Type;







void Bsy_Os_Init(void);
void  delay_ms(u16 ms);

//添加任务到任务列表
void Task_Add(RUN_Handle_Type *handle);
Stud_Type * Search_Node(Stud_Type *p , uint8_t id);
//任务初始化
void Task_Init(void);
void Task_Del(Stud_Type *sp);
Stud_Type * Get_Node(uint8_t level);
void Task_Delay_ms(u16 nms);
void Task_Schedule(void);



#endif

