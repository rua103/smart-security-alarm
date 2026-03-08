#ifndef __DHT11_H
#define __DHT11_H
#include "stm32f10x.h"

// IO口操作宏定义
#define DHT11_DQ_OUT_PP()   {GPIOA->CRL&=0xFFFFF0FF;GPIOA->CRL|=0x00000010;} // PA2推挽输出
#define DHT11_DQ_IN_FLOATING() {GPIOA->CRL&=0xFFFFF0FF;GPIOA->CRL|=0x00000040;} // PA2浮空输入

#define DHT11_DQ_OUT(x)     GPIO_WriteBit(GPIOA, GPIO_Pin_2, (BitAction)(x))
#define DHT11_DQ_IN         GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2)

// DHT11函数声明
uint8_t DHT11_Init(void);
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi);

#endif

