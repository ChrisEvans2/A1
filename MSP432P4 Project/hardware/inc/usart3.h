#ifndef __USART3_H
#define __USART3_H
#include "sysinit.h"
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

#define HC_05_USART_BASE EUSCI_A2_BASE
#define HC_05_USART_PORT_PIN GPIO_PORT_P3, GPIO_PIN2 | GPIO_PIN3

#define USART3_MAX_SEND_LEN 600 //����ͻ����ֽ���
#define USART3_RX_EN 1          //0,������;1,����.

extern uint8_t USART3_TX_BUF[USART3_MAX_SEND_LEN]; //���ͻ���,���USART3_MAX_SEND_LEN�ֽ�
extern volatile uint16_t USART3_RX_STA;            //��������״̬

void usart3_init(uint32_t baudRate); //����2��ʼ��
void u3_printf(char *fmt, ...);
void BluetoothSendDate(float *gyh, float THDx, uint16_t *waveBin);

#endif
