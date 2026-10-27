/*****************************************************
* 文件名  ：BSY_OS.c
* 描述    ：BSY_OS 实时操作系统内核          
* 硬件连接：使用SysTick定时器做时基定时，周期1ms， 1000Hz(必须)
* 版本：    V1.0
*
* GitHub:  YCBSY-zyz
* 作者： 张永正   yc_bsy@126.com

*****************************************************/

#include "bsy_os.h"
#include "stdio.h"

Stud_Type   *root;		//链表根节点

u8 cur_thread = 0;      //当前运行线程
u16 cnts = 0;           //定时器计数值
u16 delay_cnt = 0;      //delay_ms专用计数值


/*******************************************************************************
* 函数名  : Bsy_Os_Init
* 描述    : BSY_OS内核及系统滴答时钟SysTick初始化
* 输入    : 无
* 输出    : 无
* 返回    : 无 
* 说明    : 1)、SystemFrequency / 1000		1ms中断一次
*			计算方法:(SystemFrequency / Value)个系统时钟节拍中断一次
*******************************************************************************/
void Bsy_Os_Init(void)
{
	//内核时钟节拍为1MS，即1000Hz频率，用于任务状态切换及延时处理
    while(SysTick_Config(SystemCoreClock / 1000));	//初始化并使能系统滴答时钟,返回1表示计数设置太大			 
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;		//使能滴答定时器
    Task_Init();                                    //创建根节点           
}

void Task_Schedule(void){
    
    //任务优先级高的执行，执行完成后再次查找最高的优先级且就绪的任务并开始执行，非顺序执行
    Stud_Type *sp;
    sp = root->next;
    while(sp){
        //检查是否就绪，按照优先级高到低查找,执行完成后则重新开始，防止有高优先级的任务延误
        if(sp->task->R_STATUS == OK){           //如果已就绪，则执行该任务
            cur_thread = sp->task->LEVEL;       //或取当前运行线程号
            sp->task->R_STATUS = RUN;           //设置为运行态
            sp->task->pf(sp->task->pmt);        //执行该函数
            sp->task->R_STATUS = STOP;          //设置为停止态
            break;                              //结束本次循环，每次只执行一个任务
        }else{
            sp = sp->next;                      //指向下一个任务
        }
    }
}

//内核初始化
void Task_Init(void)
{
	Stud_Type *p;
	p = (Stud_Type *)malloc(sizeof(Stud_Type));
    p->id = 0;
	p->pre = NULL;
	p->next = NULL;
	p->task = NULL;
	root = p;
}



//查找第一个大于当前id的节点或如果没有则返回最后一个节点
Stud_Type * Search_Node(Stud_Type *p , uint8_t id)
{
	Stud_Type *h;
	h = p->next;
	while(h){
		if(h->id > id){	//找到第一个比id大的节点
			return h;
		}
		if(h->next == NULL) return h;	//判断是否是到最后的节点，如果到最后也没找到，返回最后一个节点
		else h = h->next;
	}
	return NULL;
}


//查找某一个节点是否存在，存在返回节点，不存在返回NULL
Stud_Type * Get_Node(uint8_t level)
{
	Stud_Type *h;
	h = root->next;
	while(h){
		if(h->id > level){	//找到第一个比id大的节点
			return h;
		}
		h = h->next;
	}
	return NULL;
}




/********************************************
*   创建函数TaskCreat
*   RUN_Handle_Type  handle 任务句柄
*   
*   函数类型为无返回值的函数
********************************************/
void Task_Add(RUN_Handle_Type *handle)
{
    Stud_Type *c, *n;
    if(root->next == NULL){     //先判断根节点是否为空，就是当前没有节点
        n = (Stud_Type *)malloc(sizeof(Stud_Type));		//申请内存
		n->id = handle->LEVEL;			//添加任务id
		n->task = handle;				//添加任务句柄
		root->next = n;                 //设置根节点的next是新节点的地址
        n->pre = root;                  //新节点的pre指向root
        n->next = NULL;				    //设置新节点的next为NULL
    }else{      //有节点
        c = Search_Node(root , handle->LEVEL);			//从根节点开始查找，查找第一个大于当前id的节点，没有就返回最后一个节点
        if(c == NULL) printf("Search node error!\r\n");
        else{
            n = (Stud_Type *)malloc(sizeof(Stud_Type));		//申请内存
            n->id = handle->LEVEL;			//添加任务id
            n->task = handle;				//添加任务句柄
            if(c->id < handle->LEVEL){		//没有大于当前的id的节点，则追加到最后节点
                n->next = NULL;				//设置新节点的next为NULL
                c->next = n;				//设置查找的节点的next是新节点的地址
                n->pre = c;                 //新节点的pre指向最后一个节点
            }else if(c->id > handle->LEVEL){		//有大于当前的id的节点，则插入到当前节点之前
                n->pre = c->pre;
                c->pre->next = n;			//当前节点的上一个节点的next指向新节点
                n->next = c;			    //新节点的next指向当前节点
                c->pre = n;
            }
        }     
    }
	
}


//删除任务节点
void Task_Del(Stud_Type *sp)
{
	sp->next->pre = sp->pre;
	sp->pre->next = sp->next;
	free(sp);
}



//对任务延时，时间信号及任务状态进行管理
void Task_Manager(void){
    
	Stud_Type *sp;
	sp = root->next;	//指向首节点
	while(sp){
		//任务定时及任务状态处理
		if(!sp->task->LOCK){                                       //如果没有上锁，则执行下面的延时
            if(sp->task->NMS > 0) sp->task->NMS--;            //延时减一
            if(sp->task->NMS == 0){
                sp->task->R_STATUS = OK;                           //如果减到0，延时时间到设置状态为就绪态OK
                sp->task->NMS = sp->task->R_CYCLE;            //加载延时周期
            }
        }else{
            sp->task->NMS = sp->task->R_CYCLE;                 //如果上锁，加载延时周期，再次运行时重新开始
        }
		sp = sp->next;				//处理下一个节点
	}
}

/******************************************
*   任务中可变延时函数Task_Delay_ms
*   level  该任务的优先级
*   nms 延时时间多少ms
******************************************/
void Task_Delay_ms(u16 nms){
    Stud_Type *sp;
    sp = root->next;
    
    while(sp){
        if(sp->id == cur_thread){        //找到对应的节点
            sp->task->NMS = nms;    //改变该节点的延时时间
            break;
        }
        sp = sp->next;
    }
}

//延时函数，阻塞函数，尽量不要在运行中使用
//可以在初始化时使用，也就是while循环之前使用
void  delay_ms(u16 ms)
{
    delay_cnt = ms;   //设置延时值
    while(delay_cnt != 0);
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	Task_Manager();
    if(delay_cnt > 0) delay_cnt--;      //delay_ms专用计数器
    
}




