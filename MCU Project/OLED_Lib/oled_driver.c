#include "oled_driver.h"

vu8 OLED_ADDRESS = OLED_UP_ADDRESS;

void SelectDownOLED(void)
{
#ifdef __MSP432P401R__
	MAP_I2C_setSlaveAddress(EUSCI_BX, OLED_UP_ADDRESS >> 1);
#else
    OLED_ADDRESS = OLED_UP_ADDRESS;
#endif
}

void SelectUpOLED(void)
{
#ifdef __MSP432P401R__
	MAP_I2C_setSlaveAddress(EUSCI_BX, OLED_DOWN_ADDRESS >> 1);
#else
    OLED_ADDRESS = OLED_DOWN_ADDRESS;
#endif
}


#if (TRANSFER_METHOD == HW_IIC)
//I2C_Configuration，初始化硬件IIC引脚
void I2C_Configuration(void)
{
#ifdef __MSP432P401R__
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(
		IIC_GPIOX, IIC_SCL_Pin | IIC_SDA_Pin, GPIO_PRIMARY_MODULE_FUNCTION);
	const eUSCI_I2C_MasterConfig i2cConfig =
		{
			EUSCI_B_I2C_CLOCKSOURCE_SMCLK, // SMCLK Clock Source
			48000000,					   // SMCLK = 48MHz
			2000000,					   // Desired I2C Clock of 2MHz
			0,							   // No byte counter threshold
			EUSCI_B_I2C_NO_AUTO_STOP	   // No Autostop
		};
	MAP_I2C_initMaster(EUSCI_BX, &i2cConfig);
	MAP_I2C_setSlaveAddress(EUSCI_BX, 0x3C);
	MAP_I2C_setMode(EUSCI_BX, EUSCI_B_I2C_TRANSMIT_MODE);
	MAP_I2C_enableModule(EUSCI_BX);
#else
    MX_I2C1_Init();
#endif
}

/*
static uint32_t _OLED_IIC_sending = 0;	  // 剩余要发送的数据量
static uint8_t *_OLED_IIC_sendingPtr = 0; // 数据指针
static uint8_t buff;					  // 指令/数据缓存
*/

void WriteCmd(unsigned char cmd) //写命令
{
#ifdef __MSP432P401R__
	MAP_I2C_masterSendMultiByteStartWithTimeout(EUSCI_BX, 0x00, EUSCI_B_I2C_TIMEOUT_31_MS);
	MAP_I2C_masterSendMultiByteFinishWithTimeout(EUSCI_BX, cmd, EUSCI_B_I2C_TIMEOUT_31_MS);
#else
    HAL_I2C_Mem_Write(OLED_IIC, OLED_ADDRESS, 0x00, I2C_MEMADD_SIZE_8BIT, &cmd, 1, 10);
#endif
}

void WriteDat(unsigned char dat) //写数据
{
#ifdef __MSP432P401R__
	MAP_I2C_masterSendMultiByteStartWithTimeout(EUSCI_BX, 0x40, EUSCI_B_I2C_TIMEOUT_31_MS);
	MAP_I2C_masterSendMultiByteFinishWithTimeout(EUSCI_BX, dat, EUSCI_B_I2C_TIMEOUT_31_MS);
#else
    HAL_I2C_Mem_Write(OLED_IIC, OLED_ADDRESS, 0x40, I2C_MEMADD_SIZE_8BIT, &dat, 1, 10);
#endif

}

void OLED_FILL(unsigned char BMP[])
{
	unsigned char *p;
	p = BMP;

#ifdef __MSP432P401R__
	unsigned char m, n;
	for (m = 0; m < 8; ++m)
	{
		WriteCmd(0xb0 + m); //page0-page1
		WriteCmd(0x00);		//low column start address
		WriteCmd(0x10);		//high column start address

		// 启动多字节数据发送
		MAP_I2C_masterSendMultiByteStartWithTimeout(EUSCI_BX, 0x40, EUSCI_B_I2C_TIMEOUT_31_MS);
		for (n = 0; n < 127; ++n)
		{
			MAP_I2C_masterSendMultiByteNextWithTimeout(EUSCI_BX, *p++, EUSCI_B_I2C_TIMEOUT_31_MS);
		}
		MAP_I2C_masterSendMultiByteFinishWithTimeout(EUSCI_BX, *p++, EUSCI_B_I2C_TIMEOUT_31_MS);
	}
#else
    while(*OLED_IIC.State != HAL_I2C_STATE_READY);
    HAL_I2C_Mem_Write_DMA(OLED_IIC, OLED_ADDRESS, 0x40, I2C_MEMADD_SIZE_8BIT, p, SCREEN_PAGE_NUM * SCREEN_PAGEDATA_NUM);   
#endif

}

#elif (TRANSFER_METHOD == SW_IIC)

void I2C_SW_Configuration(void)
{
	OLED_SSD1306_SCL_IO_INIT;
	OLED_SSD1306_SDA_IO_INIT;
	OLED_SDA_Set();
	OLED_SCL_Set();

	delay_ms(200);
}

//起始信号
void I2C_Start(void)
{
	OLED_SDA_Set();
	OLED_SCL_Set();
	OLED_SDA_Clr();
	OLED_SCL_Clr();
}

//结束信号
void I2C_Stop(void)
{
	OLED_SDA_Clr();
	OLED_SCL_Set();
	OLED_SDA_Set();
}

//等待信号响应
void I2C_WaitAck(void) //测数据信号的电平
{
	OLED_SDA_Set();
	OLED_SCL_Set();
	OLED_SCL_Clr();
}

//写入一个字节
void Send_Byte(uint8_t dat)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		OLED_SCL_Clr(); //将时钟信号设置为低电平
		if (dat & 0x80) //将dat的8位从最高位依次写入
		{
			OLED_SDA_Set();
		}
		else
		{
			OLED_SDA_Clr();
		}
		OLED_SCL_Set();
		OLED_SCL_Clr();
		dat <<= 1;
	}
}
/**
  * @brief  I2C_WriteByte，向OLED寄存器地址写一个byte的数据
  * @param  addr：寄存器地址
	*					data：要写入的数据
  * @retval 无
  */
void I2C_WriteByte(uint8_t addr, uint8_t data)
{
	I2C_Start();
	Send_Byte(0x78);
	I2C_WaitAck();
	Send_Byte(addr); //寄存器地址
	I2C_WaitAck();
	Send_Byte(data);
	I2C_WaitAck();
}

void WriteCmd(unsigned char cmd) //写命令
{
	I2C_Start();
	Send_Byte(0x78);
	I2C_WaitAck();
	Send_Byte(0x00);
	I2C_WaitAck();
	Send_Byte(cmd);
	I2C_WaitAck();
	I2C_Stop();
}

void WriteDat(unsigned char dat) //写数据
{
	I2C_Start();
	Send_Byte(0x78);
	I2C_WaitAck();
	Send_Byte(0x40);
	I2C_WaitAck();
	Send_Byte(dat);
	I2C_WaitAck();
	I2C_Stop();
}

void OLED_FILL(unsigned char BMP[])
{
	unsigned char *p = BMP;
	unsigned char m, n;
	for (m = 0; m < 8; m++)
	{
		WriteCmd(0xb0 + m); //page0-page1
		WriteCmd(0x00);		//low column start address
		WriteCmd(0x10);		//high column start address

		I2C_Start();
		Send_Byte(0x78);
		I2C_WaitAck();
		Send_Byte(0x40);
		I2C_WaitAck();

		for (n = 0; n < 128; n++)
		{
			Send_Byte(*p++);
			I2C_WaitAck();
		}
		I2C_Stop();
	}
	// unsigned int n;
	// unsigned char *p;
	// p = BMP;
	// WriteCmd(0xb0); //page0-page1
	// WriteCmd(0x00); //low column start address
	// WriteCmd(0x10); //high column start address

	// for (n = 0; n < 128 * 8; n++)
	// {
	// 	WriteDat(*p++); //发送数据
	// }
}
#elif (TRANSFER_METHOD == HW_SPI)

#define OLED_RESET_LOW() GPIO_ResetBits(SPI_RES_GPIOX, SPI_RES_PIN) //低电平复位
#define OLED_RESET_HIGH() GPIO_SetBits(SPI_RES_GPIOX, SPI_RES_PIN)

#define OLED_CMD_MODE() GPIO_ResetBits(SPI_DC_GPIOX, SPI_DC_PIN) //命令模式
#define OLED_DATA_MODE() GPIO_SetBits(SPI_DC_GPIOX, SPI_DC_PIN)	 //数据模式

#define OLED_CS_HIGH() GPIO_SetBits(SPI_CS_GPIOX, SPI_CS_Pin_X)
#define OLED_CS_LOW() GPIO_ResetBits(SPI_CS_GPIOX, SPI_CS_Pin_X)

void SPI_Configuration(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
#if (USE_HW_SPI == SPI_2)
	RCC_APB1PeriphClockCmd(SPI_RCC_APB1Periph_SPIX, ENABLE);
#elif (USE_HW_SPI == SPI_1)
	RCC_APB2PeriphClockCmd(SPI_RCC_APB2Periph_SPIX, ENABLE);
#endif
	RCC_APB2PeriphClockCmd(SPI_RCC_APB2Periph_GPIOX | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = SPI_CS_Pin_X;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SPI_CS_GPIOX, &GPIO_InitStructure);
	OLED_CS_HIGH();

	GPIO_InitStructure.GPIO_Pin = SPI_HW_ALL_PINS;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(SPI_HW_ALL_GPIOX, &GPIO_InitStructure);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //SPI_Direction_1Line_Tx;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(SPIX, &SPI_InitStructure);
	SPI_Cmd(SPIX, ENABLE);

	GPIO_InitStructure.GPIO_Pin = SPI_RES_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SPI_RES_GPIOX, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = SPI_DC_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SPI_DC_GPIOX, &GPIO_InitStructure);
	OLED_DATA_MODE();

	OLED_RESET_HIGH();
	WaitTimeMs(100);
	OLED_RESET_LOW();
	WaitTimeMs(100);
	OLED_RESET_HIGH();
}

void SPI_WriterByte(unsigned char dat)
{

	while (SPI_I2S_GetFlagStatus(SPIX, SPI_I2S_FLAG_TXE) == RESET)
	{
	};							 //检查指定的SPI标志位设置与否:发送缓存空标志位
	SPI_I2S_SendData(SPIX, dat); //通过外设SPIx发送一个数据
	while (SPI_I2S_GetFlagStatus(SPIX, SPI_I2S_FLAG_RXNE) == RESET)
	{
	};						   //检查指定的SPI标志位设置与否:接受缓存非空标志位
	SPI_I2S_ReceiveData(SPIX); //返回通过SPIx最近接收的数据
}

void WriteCmd(unsigned char cmd)
{
	OLED_CMD_MODE();
	OLED_CS_LOW();
	SPI_WriterByte(cmd);
	OLED_CS_HIGH();
	OLED_DATA_MODE();
}

void WriteDat(unsigned char dat)
{
	OLED_DATA_MODE();
	OLED_CS_LOW();
	SPI_WriterByte(dat);
	OLED_CS_HIGH();
	OLED_DATA_MODE();
}

void OLED_FILL(unsigned char BMP[])
{
	u8 i, j;
	unsigned char *p;
	p = BMP;

	for (i = 0; i < 8; i++)
	{
		WriteCmd(0xb0 + i); //page0-page1
		WriteCmd(0x00);		//low column start address
		WriteCmd(0x10);

		for (j = 0; j < 128; j++)
		{
			WriteDat(*p++);
		}
	}
}

#endif //(TRANSFER_METHOD ==HW_IIC)

void OLED_Init(void)
{
	SelectDownOLED();
	WriteCmd(0xAE); //display off
	WriteCmd(0x20); //Set Memory Addressing Mode
//	WriteCmd(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0x00); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0); //Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(0xc8); //Set COM Output Scan Direction
	WriteCmd(0x00); //---set low column address
	WriteCmd(0x10); //---set high column address
	WriteCmd(0x40); //--set start line address
	WriteCmd(0x81); //--set contrast control register
	WriteCmd(0xff); //亮度调节 0x00~0xff
	WriteCmd(0xa1); //--set segment re-map 0 to 127
	WriteCmd(0xa6); //--set normal display
	WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	WriteCmd(0x3F); //
	WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCmd(0xd3); //-set display offset
	WriteCmd(0x00); //-not offset
	WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0); //--set divide ratio
	WriteCmd(0xd9); //--set pre-charge period
	WriteCmd(0x22); //
	WriteCmd(0xda); //--set com pins hardware configuration
	WriteCmd(0x12);
	WriteCmd(0xdb); //--set vcomh
	WriteCmd(0x20); //0x20,0.77xVcc
	WriteCmd(0x8d); //--set DC-DC enable
	WriteCmd(0x14); //
	WriteCmd(0xaf); //--turn on oled panel
	OLED_CLS();

	SelectUpOLED();
	WriteCmd(0xAE); //display off
	WriteCmd(0x20); //Set Memory Addressing Mode
//	WriteCmd(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0x00); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0); //Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(0xc8); //Set COM Output Scan Direction
	WriteCmd(0x00); //---set low column address
	WriteCmd(0x10); //---set high column address
	WriteCmd(0x40); //--set start line address
	WriteCmd(0x81); //--set contrast control register
	WriteCmd(0xff); //亮度调节 0x00~0xff
	WriteCmd(0xa1); //--set segment re-map 0 to 127
	WriteCmd(0xa6); //--set normal display
	WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	WriteCmd(0x3F); //
	WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCmd(0xd3); //-set display offset
	WriteCmd(0x00); //-not offset
	WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0); //--set divide ratio
	WriteCmd(0xd9); //--set pre-charge period
	WriteCmd(0x22); //
	WriteCmd(0xda); //--set com pins hardware configuration
	WriteCmd(0x12);
	WriteCmd(0xdb); //--set vcomh
	WriteCmd(0x20); //0x20,0.77xVcc
	WriteCmd(0x8d); //--set DC-DC enable
	WriteCmd(0x14); //
	WriteCmd(0xaf); //--turn on oled panel
	OLED_CLS();
}

void OLED_CLS(void) //清屏
{
#ifdef __MSP432P401R__
	unsigned char m, n;
	for (m = 0; m < 8; m++)
	{
		WriteCmd(0xb0 + m); //page0-page1
		WriteCmd(0x00);		//low column start address
		WriteCmd(0x10);		//high column start address
		for (n = 0; n < 128; n++)
		{
			WriteDat(0x00);
		}
	}    
#else
    extern unsigned char ScreenBuffer[SCREEN_PAGE_NUM][SCREEN_COLUMN];
    uint16_t i;
    for (i = 0; i < SCREEN_PAGE_NUM * SCREEN_COLUMN; ++i)
    {
        ScreenBuffer[0][i] = 0;
    }
	OLED_FILL(ScreenBuffer[0]);    
#endif


}

void OLED_ON(void)
{
	WriteCmd(0X8D); //设置电荷泵
	WriteCmd(0X14); //开启电荷泵
	WriteCmd(0XAF); //OLED唤醒
}

void OLED_OFF(void)
{
	WriteCmd(0X8D); //设置电荷泵
	WriteCmd(0X10); //关闭电荷泵
	WriteCmd(0XAE); //OLED休眠
}
