#include "DHT11.h"
#include "Delay.h"

// DHT11初始化
uint8_t DHT11_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // 使能GPIOA时钟

	// 配置PA2为推挽输出，用于发送启动信号
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	DHT11_DQ_OUT(1); // 拉高DQ线

	Delay_ms(100); // 等待DHT11上电稳定

	// 发送复位信号
	DHT11_DQ_OUT_PP(); // 设置为输出模式
	DHT11_DQ_OUT(0);   // 拉低DQ线
	Delay_ms(18);      // 保持低电平18ms
	DHT11_DQ_OUT(1);   // 拉高DQ线
	Delay_us(30);      // 保持高电平20-40us

	DHT11_DQ_IN_FLOATING(); // 设置为浮空输入模式，等待DHT11响应

	// 检测DHT11响应
	uint8_t retry = 0;
	while (DHT11_DQ_IN && retry < 100) // DHT11拉低40-80us
	{
		retry++;
		Delay_us(1);
	}
	if (retry >= 100) return 1; // 无响应

	retry = 0;
	while (!DHT11_DQ_IN && retry < 100) // DHT11拉高40-80us
	{
		retry++;
		Delay_us(1);
	}
	if (retry >= 100) return 1; // 无响应

	return 0; // 响应成功
}

// 从DHT11读取一个字节
static uint8_t DHT11_Read_Byte(void)
{
	uint8_t i, data = 0;
	DHT11_DQ_IN_FLOATING(); // 设置为输入模式

	for (i = 0; i < 8; i++)
	{
		uint8_t retry = 0;
		while (DHT11_DQ_IN && retry < 100) // 等待50us低电平开始信号
		{
			retry++;
			Delay_us(1);
		}
		retry = 0;
		while (!DHT11_DQ_IN && retry < 100) // 等待26-28us高电平
		{
			retry++;
			Delay_us(1);
		}
		Delay_us(40); // 延时40us，判断是0还是1

		data <<= 1;
		if (DHT11_DQ_IN) data |= 1; // 如果此时为高电平，则为1
	}
	return data;
}

// 读取DHT11数据
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi)
{
	uint8_t buf[5];
	uint8_t i;

	DHT11_DQ_OUT_PP(); // 设置为输出模式
	DHT11_DQ_OUT(1);   // 拉高DQ线

	if (DHT11_Init() == 0) // 重新发送启动信号并检测响应
	{
		for (i = 0; i < 5; i++) // 读取40位数据
		{
			buf[i] = DHT11_Read_Byte();
		}
		if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4]) // 校验和正确
		{
			*humi = buf[0]; // 湿度整数部分
			*temp = buf[2]; // 温度整数部分
			return 0; // 读取成功
		}
	}
	return 1; // 读取失败
}

