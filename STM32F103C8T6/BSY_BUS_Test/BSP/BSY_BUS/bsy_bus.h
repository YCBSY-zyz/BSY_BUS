#ifndef __BSY_BUS_H_
#define __BSY_BUS_H_

#include <sys.h>


//定义数据段的有效数据长度，本协议数据段长度只有1位，因此最大值255
//最好使用时最大用多少设置多少
#define  MAX_DATA_LEN       16

#define  MAXBUF        MAX_DATA_LEN + 6




typedef struct{
    uint8_t  data[MAXBUF];      //接收到的数据
    uint16_t  len;              //已完成本次接收的数据长度
    uint8_t  rec_ok;            //有接收到的新数据，1 有数据， 0 无数据
    uint8_t  buf[MAXBUF];       //数据保存缓存
    uint16_t  cnt;              //接收数据长度
    uint8_t  recFlag;           //接收状态 0 无数据， 1 接收中, 2 接收完成
}UART_REC_Type;




//获取余128校验值，返回校验值
uint8_t mod128(uint8_t *buf, uint8_t len);


//检验余128校验值，返回校验结果，成功返回1，失败返回0
uint8_t check_mod128(uint8_t *buf, uint8_t len);


//清零缓存
void clear_buf(uint8_t *buf, uint8_t len);


//接收BSY_BUS协议格式数据处理程序
void BSY_BUS_recProcess(UART_REC_Type *recType, uint8_t  rec);

#endif

