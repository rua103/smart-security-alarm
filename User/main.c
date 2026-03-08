//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//欣蕊电子
//
//  版   本  号 : v1.0
//  作    者   : 欣蕊电子
//  生成日期   : 20241125
//  最近修改   : 
//  功能描述   : 智能安防报警系统 - 避障碍+火焰检测+LED+蜂鸣器
// 日    期   : 
// 作    者   : 欣蕊电子
// 修改内容   : 创建文件
//版权所有，盗版必究。
//Copyright(C) 欣蕊电子2024/11/25
//All rights reserved
//******************************************************************************/

#include "stm32f10x.h"                  // Device header
#include "Delay.h"

int main(void)
{
	// 使能GPIOA、GPIOB和AFIO时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);    // 【修复1】必须使能AFIO时钟
	
	// 【修复1】禁用JTAG，释放PA15和PB4引脚（保留SWD用于ST-Link烧录调试）
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// 配置PA0（双色LED红色）和PA1（双色LED绿色）为推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// 配置PA15（避障碍模块）为下拉输入 【修复3】改为IPD，与参考项目一致
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// 配置PB4（火焰传感器）为下拉输入 【修复3】改为IPD，与参考项目一致
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// 配置PB12（有源蜂鸣器）为推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// 初始化：关闭所有输出
	GPIO_ResetBits(GPIOA, GPIO_Pin_0);   // 红色LED关闭
	GPIO_ResetBits(GPIOA, GPIO_Pin_1);   // 绿色LED关闭
	GPIO_ResetBits(GPIOB, GPIO_Pin_12);  // 蜂鸣器关闭
	
	// 变量定义
	uint8_t obstacle_status = 0;  // 避障碍模块状态
	uint8_t flame_status = 0;     // 火焰传感器状态
	uint8_t alarm_status = 0;    // 报警状态
	uint8_t led_state = 0;        // LED闪烁状态
	
	while (1)
	{
		// 读取传感器状态
		obstacle_status = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15);  // 避障碍模块
		flame_status = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4);      // 火焰传感器
		
		// 判断是否报警（任一传感器触发即报警）
		// 传感器为高电平触发：检测到障碍/火焰时输出1，正常时输出0（与参考项目一致）
		alarm_status = (obstacle_status == 0) || (flame_status == 0);
		
		if (alarm_status)
		{
			// 报警状态：红色LED闪烁 + 蜂鸣器响
			// 红色LED闪烁（200ms间隔）
			if (led_state == 0)
			{
				GPIO_SetBits(GPIOA, GPIO_Pin_0);    // 红色LED亮
				GPIO_ResetBits(GPIOA, GPIO_Pin_1);  // 绿色LED灭
				led_state = 1;
			}
			else
			{
				GPIO_ResetBits(GPIOA, GPIO_Pin_0);  // 红色LED灭
				GPIO_ResetBits(GPIOA, GPIO_Pin_1);  // 绿色LED灭
				led_state = 0;
			}
			
			// 蜂鸣器持续鸣叫
			GPIO_SetBits(GPIOB, GPIO_Pin_12);
			
			// 延时200ms（闪烁间隔）
			Delay_ms(200);
		}
		else
		{
			// 正常状态：绿色LED常亮 + 蜂鸣器关闭
			GPIO_ResetBits(GPIOA, GPIO_Pin_0);  // 红色LED关闭
			GPIO_SetBits(GPIOA, GPIO_Pin_1);    // 绿色LED常亮
			GPIO_ResetBits(GPIOB, GPIO_Pin_12); // 蜂鸣器关闭
			
			led_state = 0;  // 重置LED状态
			
			// 延时10ms（主循环延时）
			Delay_ms(10);
		}
	}
}
