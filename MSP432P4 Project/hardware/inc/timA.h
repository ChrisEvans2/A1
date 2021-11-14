/****************************************************/
// MSP432P401R
// ��ʱ��A
// Bilibili��m-RNA
// E-mail:m-RNA@qq.com
// ��������:2021/8/26
/****************************************************/

#ifndef __RNA_TIMA_H
#define __RNA_TIMA_H
#include "sysinit.h"

#define CAP_TIMA_SELECTION TIMER_A2_BASE                         //������Ķ�ʱ��
#define CAP_REGISTER_SELECTION TIMER_A_CAPTURECOMPARE_REGISTER_2 //������Ķ�ʱ��ͨ��
#define CAP_CCR_NUM 2                                            //������Ķ�ʱ��ͨ��
#define CAP_PORT_PIN GPIO_PORT_P5, GPIO_PIN7                     //������ĸ�������

extern uint8_t TIMA2_CAP_STA;
extern uint16_t true_T;

void TimA2_Cap_Init(void);
void TimA0_Int_Init(uint16_t ccr0, uint16_t psc);

#endif
