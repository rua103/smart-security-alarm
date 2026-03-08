#include "stm32f10x.h"
#include "OLED.h"
#include "OLED_Font.h"
#include "Delay.h"

// ========== 软件I2C引脚定义 ==========
// SCL = PB8, SDA = PB9
#define OLED_SCL_HIGH() GPIO_SetBits(GPIOB, GPIO_Pin_8)
#define OLED_SCL_LOW()  GPIO_ResetBits(GPIOB, GPIO_Pin_8)
#define OLED_SDA_HIGH() GPIO_SetBits(GPIOB, GPIO_Pin_9)
#define OLED_SDA_LOW()  GPIO_ResetBits(GPIOB, GPIO_Pin_9)

// SSD1306 I2C地址（8位写地址）
#define OLED_ADDRESS    0x78

// ========== 软件I2C底层 ==========
static void I2C_Delay(void)
{
	volatile uint8_t i = 30;
	while (i--);
}

static void I2C_Start(void)
{
	OLED_SDA_HIGH();
	OLED_SCL_HIGH();
	I2C_Delay();
	OLED_SDA_LOW();
	I2C_Delay();
	OLED_SCL_LOW();
}

static void I2C_Stop(void)
{
	OLED_SDA_LOW();
	OLED_SCL_HIGH();
	I2C_Delay();
	OLED_SDA_HIGH();
	I2C_Delay();
}

static void I2C_WriteByte(uint8_t data)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		if (data & 0x80)
			OLED_SDA_HIGH();
		else
			OLED_SDA_LOW();
		I2C_Delay();
		OLED_SCL_HIGH();
		I2C_Delay();
		OLED_SCL_LOW();
		data <<= 1;
	}
	// ACK（忽略应答）
	OLED_SDA_HIGH();
	I2C_Delay();
	OLED_SCL_HIGH();
	I2C_Delay();
	OLED_SCL_LOW();
}

// ========== OLED命令/数据写入 ==========
static void OLED_WriteCmd(uint8_t cmd)
{
	I2C_Start();
	I2C_WriteByte(OLED_ADDRESS);
	I2C_WriteByte(0x00);
	I2C_WriteByte(cmd);
	I2C_Stop();
}

static void OLED_WriteData(uint8_t data)
{
	I2C_Start();
	I2C_WriteByte(OLED_ADDRESS);
	I2C_WriteByte(0x40);
	I2C_WriteByte(data);
	I2C_Stop();
}

// ========== 设置光标位置 ==========
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCmd(0xB0 | Y);
	OLED_WriteCmd(0x10 | ((X & 0xF0) >> 4));
	OLED_WriteCmd(0x00 | (X & 0x0F));
}

// ========== 清屏 ==========
void OLED_Clear(void)
{
	uint8_t i, j;
	for (j = 0; j < 8; j++)
	{
		OLED_SetCursor(j, 0);
		for (i = 0; i < 128; i++)
		{
			OLED_WriteData(0x00);
		}
	}
}

// ========== 全屏填充 ==========
void OLED_Fill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t fill)
{
	uint8_t i, j;
	for (j = y1; j <= y2; j++)
	{
		OLED_SetCursor(j, x1);
		for (i = x1; i <= x2; i++)
		{
			OLED_WriteData(fill);
		}
	}
}

// ========== 显示一个字符（8x16） ==========
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i]);
	}
	OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);
	}
}

// ========== 显示字符串 ==========
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		OLED_ShowChar(Line, Column + i, String[i]);
	}
}

// ========== 次方函数 ==========
static uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

// ========== 显示数字 ==========
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

// ========== 显示带符号数字 ==========
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)
	{
		OLED_ShowChar(Line, Column, '+');
		Number1 = Number;
	}
	else
	{
		OLED_ShowChar(Line, Column, '-');
		Number1 = -Number;
	}
	for (i = 0; i < Length; i++)
	{
		OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

// ========== OLED初始化 ==========
void OLED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	uint32_t i, j;

	// 上电延时
	for (i = 0; i < 1000; i++)
	{
		for (j = 0; j < 1000; j++);
	}

	// 使能GPIOB时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	// 配置PB8(SCL), PB9(SDA)为推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	OLED_SCL_HIGH();
	OLED_SDA_HIGH();

	Delay_ms(200);

	// SSD1306初始化序列
	OLED_WriteCmd(0xAE);  // 关显示
	OLED_WriteCmd(0xD5); OLED_WriteCmd(0x80);  // 时钟
	OLED_WriteCmd(0xA8); OLED_WriteCmd(0x3F);  // 多路复用率
	OLED_WriteCmd(0xD3); OLED_WriteCmd(0x00);  // 显示偏移
	OLED_WriteCmd(0x40);  // 起始行
	OLED_WriteCmd(0xA1);  // 左右方向
	OLED_WriteCmd(0xC8);  // 上下方向
	OLED_WriteCmd(0xDA); OLED_WriteCmd(0x12);  // COM配置
	OLED_WriteCmd(0x81); OLED_WriteCmd(0xCF);  // 对比度
	OLED_WriteCmd(0xD9); OLED_WriteCmd(0xF1);  // 预充电
	OLED_WriteCmd(0xDB); OLED_WriteCmd(0x30);  // VCOMH
	OLED_WriteCmd(0xA4);  // 显示RAM内容
	OLED_WriteCmd(0xA6);  // 正常显示
	OLED_WriteCmd(0x8D); OLED_WriteCmd(0x14);  // 充电泵
	OLED_WriteCmd(0xAF);  // 开显示

	OLED_Clear();
}
