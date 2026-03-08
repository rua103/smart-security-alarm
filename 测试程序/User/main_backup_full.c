////////////////////////////////////////////////////////////////////////////////
// 智能安防报警系统
// 功能：避障碍传感器 + 火焰传感器 + DHT11温湿度传感器 + 双色LED + 蜂鸣器
//
// 引脚分配：
//   PA0  - 红色LED
//   PA1  - 绿色LED
//   PA2  - DHT11数据线（温湿度传感器）
//   PA15 - 避障碍传感器（低电平=检测到障碍）
//   PB4  - 火焰传感器（低电平=检测到火焰）
//   PB12 - 有源蜂鸣器
//   PC13 - 板载蓝色LED（心跳指示）
//
// 报警逻辑：
//   正常状态：绿灯常亮，蜂鸣器不响，蓝灯定期闪烁显示温度
//   报警状态：红灯闪烁(200ms)，蜂鸣器响
//   触发条件：避障碍被遮挡 OR 检测到火焰 OR 温度超过阈值
//
// 开机指示：
//   蓝灯亮 → DHT11初始化 → 绿灯闪3次=成功 / 红灯闪3次=失败
//   正常运行后，蓝灯每隔一段时间闪烁显示温度（十位+个位）
////////////////////////////////////////////////////////////////////////////////

#include "stm32f10x.h"
#include "Delay.h"
#include "DHT11.h"

// 温度报警阈值（摄氏度），超过此值触发报警
#define TEMP_ALARM_THRESHOLD  35

// 蓝灯闪N次（用来显示数字）
static void blink_blue(uint8_t n)
{
	uint8_t i;
	for (i = 0; i < n; i++)
	{
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);  // 亮
		Delay_ms(150);
		GPIO_SetBits(GPIOC, GPIO_Pin_13);    // 灭
		Delay_ms(150);
	}
}

int main(void)
{
	uint8_t obstacle;          // 避障碍传感器状态
	uint8_t flame;             // 火焰传感器状态
	uint8_t temp = 0;          // 温度
	uint8_t humi = 0;          // 湿度
	uint8_t alarm = 0;         // 报警标志
	uint8_t led_state = 0;     // LED闪烁状态
	uint16_t dht_counter = 0;  // DHT11读取计数器
	uint8_t dht_ok = 0;        // DHT11是否初始化成功
	uint16_t show_counter = 0; // 温度显示计数器
	uint8_t i;
	
	// ====== 1. 使能时钟 ======
	RCC_APB2PeriphClockCmd(
		RCC_APB2Periph_GPIOA | 
		RCC_APB2Periph_GPIOB | 
		RCC_APB2Periph_GPIOC | 
		RCC_APB2Periph_AFIO, 
		ENABLE);
	
	// 禁用JTAG，释放PA15和PB4（保留SWD调试）
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	// ====== 2. 配置GPIO ======
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// PA0(红LED), PA1(绿LED) - 推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// PB12(蜂鸣器) - 推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// PC13(板载蓝灯) - 推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	// PA15(避障碍传感器) - 上拉输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// PB4(火焰传感器) - 上拉输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// ====== 3. 初始化 ======
	GPIO_ResetBits(GPIOA, GPIO_Pin_0);   // 红灯灭
	GPIO_ResetBits(GPIOA, GPIO_Pin_1);   // 绿灯灭
	GPIO_ResetBits(GPIOB, GPIO_Pin_12);  // 蜂鸣器停
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);  // 蓝灯亮 = 正在初始化
	
	// 初始化DHT11
	Delay_ms(1000);  // DHT11上电稳定
	dht_ok = (DHT11_Init() == 0) ? 1 : 0;
	
	GPIO_SetBits(GPIOC, GPIO_Pin_13);  // 蓝灯灭
	Delay_ms(300);
	
	// 显示DHT11初始化结果
	if (dht_ok)
	{
		// 成功 → 绿灯闪3次
		for (i = 0; i < 3; i++)
		{
			GPIO_SetBits(GPIOA, GPIO_Pin_1);
			Delay_ms(250);
			GPIO_ResetBits(GPIOA, GPIO_Pin_1);
			Delay_ms(250);
		}
	}
	else
	{
		// 失败 → 红灯闪3次
		for (i = 0; i < 3; i++)
		{
			GPIO_SetBits(GPIOA, GPIO_Pin_0);
			Delay_ms(250);
			GPIO_ResetBits(GPIOA, GPIO_Pin_0);
			Delay_ms(250);
		}
	}
	
	Delay_ms(1500);  // 等DHT11准备好
	
	// ====== 4. 主循环 ======
	while (1)
	{
		// --- 读取传感器 ---
		obstacle = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15);
		flame = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4);
		
		// --- 读取DHT11（每约2秒读一次）---
		dht_counter++;
		if (dht_counter >= 10 && dht_ok)  // 10次循环 × 200ms ≈ 2秒
		{
			dht_counter = 0;
			DHT11_Read_Data(&temp, &humi);
		}
		
		// --- 判断报警条件 ---
		alarm = 0;
		
		// 条件1：避障碍传感器被遮挡（低电平触发）
		if (obstacle == 0)
		{
			alarm = 1;
		}
		
		// 条件2：火焰传感器检测到火焰（低电平触发）
		if (flame == 0)
		{
			alarm = 1;
		}
		
		// 条件3：温度过高
		if (dht_ok && temp >= TEMP_ALARM_THRESHOLD)
		{
			alarm = 1;
		}
		
		// --- 执行报警/正常动作 ---
		if (alarm)
		{
			// ===== 报警状态 =====
			// 红灯(PA0)闪烁 + 蓝灯同步闪烁
			if (led_state == 0)
			{
				GPIO_SetBits(GPIOA, GPIO_Pin_0);      // 红灯亮
				GPIO_SetBits(GPIOC, GPIO_Pin_13);      // 蓝灯灭
				led_state = 1;
			}
			else
			{
				GPIO_ResetBits(GPIOA, GPIO_Pin_0);     // 红灯灭
				GPIO_ResetBits(GPIOC, GPIO_Pin_13);     // 蓝灯亮
				led_state = 0;
			}
			GPIO_ResetBits(GPIOA, GPIO_Pin_1);     // 绿灯灭
			GPIO_SetBits(GPIOB, GPIO_Pin_12);      // 蜂鸣器响
			
			show_counter = 0;  // 报警时重置温度显示计数
			Delay_ms(200);
		}
		else
		{
			// ===== 正常状态 =====
			GPIO_SetBits(GPIOA, GPIO_Pin_1);       // 绿灯(PA1)亮
			GPIO_ResetBits(GPIOA, GPIO_Pin_0);     // 红灯(PA0)灭
			GPIO_ResetBits(GPIOB, GPIO_Pin_12);    // 蜂鸣器停
			GPIO_ResetBits(GPIOC, GPIO_Pin_13);    // 蓝灯亮
			led_state = 0;
			
			// 每约10秒用蓝灯闪烁显示一次温度
			show_counter++;
			if (show_counter >= 50 && dht_ok && temp > 0)  // 50×200ms = 10秒
			{
				show_counter = 0;
				
				// 蓝灯闪烁显示温度：十位 → 停1秒 → 个位
				GPIO_SetBits(GPIOC, GPIO_Pin_13);  // 先灭
				Delay_ms(500);
				blink_blue(temp / 10);   // 十位
				Delay_ms(800);
				blink_blue(temp % 10);   // 个位
				Delay_ms(500);
			}
			else
			{
				Delay_ms(200);
			}
		}
	}
}

