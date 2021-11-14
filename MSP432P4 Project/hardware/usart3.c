//////////////////////////////////////////////////////////////////////////////////
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//����3��������
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/3/29
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////

/****************************************************/
// MSP432P401R
// ����2����
// Bilibili��m-RNA
// E-mail:m-RNA@qq.com
/****************************************************/

#include "usart3.h"
#include "delay.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "timA.h"
#include "baudrate_calculate.h"

/* ���͸��ֻ� */
void BluetoothSendDate(float *gyh, float THDx, uint16_t *waveBin)
{
	uint8_t i;
	/* ����THD */
	MAP_UART_transmitData(HC_05_USART_BASE, (uint16_t)(THDx * 100) >> 8);
	MAP_UART_transmitData(HC_05_USART_BASE, (uint16_t)(THDx * 100) & 0xff);

	/* �������ֵ */
	for (i = 0; i < 128; ++i)
	{
		MAP_UART_transmitData(HC_05_USART_BASE, waveBin[i] >> 8);
		MAP_UART_transmitData(HC_05_USART_BASE, waveBin[i] & 0xff);
	}

	/* ���͹�һ����ֵ */
	for (i = 0; i < 4; ++i)
	{
		MAP_UART_transmitData(HC_05_USART_BASE, (uint16_t)(gyh[i] * 100) >> 8);
		MAP_UART_transmitData(HC_05_USART_BASE, (uint16_t)(gyh[i] * 100) & 0xff);
	}

}

//���ڽ��ջ�����
uint8_t USART3_TX_BUF[USART3_MAX_SEND_LEN]; //���ͻ���,���USART3_MAX_SEND_LEN�ֽ�


//��ʼ��IO ����3
//pclk1:PCLK1ʱ��Ƶ��(Mhz)
//bound:������
void usart3_init(uint32_t baudRate)
{
#ifdef EUSCI_A_UART_7_BIT_LEN
	//�̼���v3_40_01_02
	//Ĭ��SMCLK 48MHz ������ 115200
	const eUSCI_UART_ConfigV1 uartConfig =
		{
			EUSCI_A_UART_CLOCKSOURCE_SMCLK,				   // SMCLK Clock Source
			312,										   // BRDIV = 312
			8,											   // UCxBRF = 8
			1,											   // UCxBRS = 1
			EUSCI_A_UART_NO_PARITY,						   // No Parity
			EUSCI_A_UART_LSB_FIRST,						   // MSB First
			EUSCI_A_UART_ONE_STOP_BIT,					   // One stop bit
			EUSCI_A_UART_MODE,							   // UART mode
			EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION, // Oversampling
			EUSCI_A_UART_8_BIT_LEN						   // 8 bit data length
		};
	eusci_calcBaudDividers((eUSCI_UART_ConfigV1 *)&uartConfig, baudRate); //���ò�����
#else
	//�̼���v3_21_00_05
	//Ĭ��SMCLK 48MHz ������ 115200
	const eUSCI_UART_Config uartConfig =
		{
			EUSCI_A_UART_CLOCKSOURCE_SMCLK,				   // SMCLK Clock Source
			312,										   // BRDIV = 312
			8,											   // UCxBRF = 8
			1,											   // UCxBRS = 1
			EUSCI_A_UART_NO_PARITY,						   // No Parity
			EUSCI_A_UART_LSB_FIRST,						   // MSB First
			EUSCI_A_UART_ONE_STOP_BIT,					   // One stop bit
			EUSCI_A_UART_MODE,							   // UART mode
			EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION, // Oversampling
		};
	eusci_calcBaudDividers((eUSCI_UART_Config *)&uartConfig, baudRate); //���ò�����
#endif
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(HC_05_USART_PORT_PIN, GPIO_PRIMARY_MODULE_FUNCTION);
	MAP_UART_initModule(HC_05_USART_BASE, &uartConfig);
	MAP_UART_enableModule(HC_05_USART_BASE);
}

//����3,printf ����
//ȷ��һ�η������ݲ�����USART3_MAX_SEND_LEN�ֽ�
void u3_printf(char *fmt, ...)
{
	uint16_t i, j;
	va_list ap;
	va_start(ap, fmt);
	vsprintf((char *)USART3_TX_BUF, fmt, ap);
	va_end(ap);
	i = strlen((const char *)USART3_TX_BUF); //�˴η������ݵĳ���
	for (j = 0; j < i; j++)					 //ѭ����������
	{
		MAP_UART_transmitData(HC_05_USART_BASE, USART3_TX_BUF[j]);
	}
}
