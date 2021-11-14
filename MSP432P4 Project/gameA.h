/*******************************************
// MSP432P401R
// 2021�� ����A��
// Bilibili��m-RNA
// E-mail:m-RNA@qq.com
// ��������:2021/11/11
*******************************************/

#ifndef __RNA_2021_GAME_A_H
#define __RNA_2021_GAME_A_H
#include "sysinit.h"

// xx����... ���ѵ�����
#define FDBS 4

extern uint16_t f0;            // ����
extern uint16_t fx[4];         // г��
extern uint16_t waveTran[128]; // ת����ʾ��

/* �ҳ�����������λ�� */
uint16_t FloatMax(float Mag[]);

/* �ҳ����ֵλ��(������) */
uint16_t FloatMax_WithWindow(float Mag[], uint16_t l, uint16_t r);

/* THDx���� */
float THDx_calculate(void);

/* ��ʾ��һ�������� */
void ShowGYH_XY(void);

/* ���¹�һ����ֵ��THD */
void UpdateGYH(float *gyh, float thd);

/* ���㡢ת������ʾ���� */
void ShowWave_AndTran(float *gyh);

/* �ȴ����� */
void WaitingAnimat(uint16_t a);

/* ����ͼ�������귽����ʾ */
void WaveBox(void);

#endif
