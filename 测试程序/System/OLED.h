#ifndef __OLED_H
#define __OLED_H

#include "stm32f10x.h"

// OLED 初始化
void OLED_Init(void);

// 清屏
void OLED_Clear(void);

// 设置光标位置 (x: 0~127列, y: 0~7页)
void OLED_SetCursor(uint8_t x, uint8_t y);

// 在指定位置显示一个字符 (6x8字体)
// x: 0~127, y: 0~7(页)
void OLED_ShowChar(uint8_t x, uint8_t y, char ch);

// 在指定位置显示字符串 (6x8字体)
void OLED_ShowString(uint8_t x, uint8_t y, const char *str);

// 在指定位置显示16x16中文字符（预定义索引）
void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t index);

// 在指定位置显示数字
// x,y: 起始坐标, num: 数字, len: 显示位数
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len);

// 在指定位置显示大号字符 (8x16字体)
void OLED_ShowChar16(uint8_t x, uint8_t y, char ch);

// 在指定位置显示大号字符串 (8x16字体)
void OLED_ShowString16(uint8_t x, uint8_t y, const char *str);

// 更新显示（将缓冲区写入OLED）
void OLED_Update(void);

// 在指定区域填充
void OLED_Fill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t fill);

#endif

