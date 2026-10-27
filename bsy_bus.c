#include "bsy_bus.h"



/**
 * @brief       余128的校验
 * @param       buf  需要计算的数据
 * @param1      len  需要计算的数据长度,包含协议头与协议尾
 * @note
 *              适用于bsybus通信协议
 *              不计算协议头，协议尾与校验位
 * @retval      返回余数结果;
 */
uint8_t mod128(uint8_t *buf, uint8_t len)
{
    int i = 0;    
    uint8_t modval = 0;
    
    for(i = 1; i < len - 2; i++){   //对协议头及协议尾以及校验位以外的数据进行计算
        modval += buf[i];           //累加,溢出不做任何处理  
        modval = modval % 128;      //与128取余运算
    }
    return modval;                  //返回余数就是校验值
}

/**
 * @brief       校验协议数据与128的余数
 * @param       buf  需要计算的数据
 * @param1      len  需要计算的数据长度,包含协议头与协议尾
 * @note
 *              适用于bsybus通信协议
 *              不计算协议头，协议尾与校验位
 * @retval      返回结果0 校验失败， 1 校验成功;
 */
uint8_t check_mod128(uint8_t *buf, uint8_t len)
{
    int i = 0; 
    uint8_t     modvals = 0;
    
    if(buf[len - 2] == 0xff)  return 1;     //校验位是0xff，代表不校验，一般用于测试
    for(i = 1; i < len - 2; i++){   //对协议头及协议尾以及余数位以外的数据进行计算
        modvals += buf[i];         //累加  
        modvals = modvals % 128;    //128取余运算
    }
    if(buf[len - 2] == modvals)  return 1;      //校验成功返回1
    return 0;
}


/**
 * @brief       清零数据
 * @param       buf  需要清零的数据
 * @param       len  需要清零的数据长度
 * @note
 * @retval      无;
 */
void clear_buf(uint8_t *buf, uint8_t len)
{
    int i = 0;
    for(i = 0; i < len; i++){   //对协议头及协议尾以及余数位以外的数据进行计算
        buf[i] = 0;             //清零  
    }
}



/**
 * @brief       bsy_bus协议中断接收数据处理函数
 * @param       recType  保存接收数据的结构体
 * @param       rec     每次中断接收到的数据
 * @note
 *              接收到完整数据后，保存数据到用户数据区，方便下次接收，中间可以不用设置发送时间间隔
 * @retval      无;
 */
void BSY_BUS_recProcess(UART_REC_Type *recType, uint8_t  rec)
{
    uint16_t  lens = 0;
    uint16_t  i = 0;
    
    if(recType->recFlag == 0){
        recType->cnt = 0;
        if(rec == 0x5b){
            recType->recFlag = 1;                   //接收到协议头
            recType->buf[0] = rec;                  //保存数据
            recType->cnt = 1;                       //计数器置1，指向下一个数据保存位置
        }
    }else if(recType->recFlag == 1){
        recType->buf[recType->cnt] = rec;           //保存数据
        recType->cnt++;                             //计数器加1
        if(recType->cnt > 6){                       //检查接收到的最小数据是否完成
            lens = recType->buf[3] + 6;              //计算接收数据总的数据长度
            if(lens > MAXBUF){                      //接收数据的数据段长度大于最大长度，则数据错误
                recType->cnt = 0;                   //清零计数器
                recType->recFlag = 0;               //清零接收状态，重新接收
                return;
            }
            if(recType->cnt == lens){               //判断接收数据长度及协议尾是否正确
                if (rec == 0x5d) {
                    recType->recFlag = 2;                   //接收完成
                }else{
                    recType->cnt = 0;                   //清零计数器
                    recType->recFlag = 0;               //清零接收状态，重新接收
                } 
            }
        }
    }
    
    if(recType->recFlag == 2){                          //接收完整数据后处理
        
        for(i = 0; i < lens; i++){
            recType->data[i] = recType->buf[i];     //拷贝接收数据到用户数据存储区
        }
        recType->len = recType->cnt;                //标记接收数据长度，完整数据包，包含协议头及协议尾
        recType->rec_ok = 1;                        //标记接收到新的数据，在前台处理完数据后及时清零
        
        recType->cnt = 0;                   //清零计数器
        recType->recFlag = 0;               //清零接收状态，重新接收 
    }
}






