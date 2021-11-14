/****************************************************/
// MSP432P401R
// ��ʱ��A
// Bilibili��m-RNA
// E-mail:m-RNA@qq.com
// ��������:2021/8/26
/****************************************************/

#include "timA.h"
#include "usart.h"
#include "adc.h"
#include "led.h"
#include "usart3.h"

/*****************************************************************************************************************/
/*******************************             ADC TIMA0                 *******************************************/

void TimA0_Int_Init(uint16_t ccr0, uint16_t psc)
{
    const Timer_A_UpModeConfig upConfig =
        {
            TIMER_A_CLOCKSOURCE_SMCLK, // smCLK Clock Source
            psc,                       // smCLK/48 = 1Mhz
            ccr0,
            TIMER_A_TAIE_INTERRUPT_DISABLE,      // Disable Timer ISR
            TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE, // Disable CCR0
            TIMER_A_DO_CLEAR                     // Clear Counter
        };
    const Timer_A_CompareModeConfig compareConfig =
        {
            TIMER_A_CAPTURECOMPARE_REGISTER_1,        // Use CCR1
            TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE, // Disable CCR interrupt
            TIMER_A_OUTPUTMODE_SET_RESET,             // Toggle output but
            24                                        //  Period
        };
    MAP_Timer_A_configureUpMode(TIMER_A0_BASE, &upConfig); //ѡ��ʱ��A1 TIMER_A0
    MAP_Timer_A_initCompare(TIMER_A0_BASE, &compareConfig);

    MAP_Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE); //��������
}

/*****************************************************************************************************************/
/******************************************     CAP TIMA2          ***********************************************/

void TimA2_Cap_Init(void)
{
    // 1.�������
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(CAP_PORT_PIN, GPIO_PRIMARY_MODULE_FUNCTION);

    /* ��ʱ�����ò��� �ֱ��� 0.33us*/
    Timer_A_ContinuousModeConfig continuousModeConfig = {
        TIMER_A_CLOCKSOURCE_SMCLK,      // SMCLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_16, // SMCLK/16 = 3MHz
        TIMER_A_TAIE_INTERRUPT_DISABLE, // ������ʱ������ж�
        TIMER_A_DO_CLEAR                // Clear Counter
    };
    // 3.����ʱ����ʼ��Ϊ��������ģʽ
    MAP_Timer_A_configureContinuousMode(CAP_TIMA_SELECTION, &continuousModeConfig);

    // 4.���ò�׽ģʽ�ṹ�� */
    const Timer_A_CaptureModeConfig captureModeConfig_TA2 = {
        CAP_REGISTER_SELECTION,                      //�����������
        TIMER_A_CAPTUREMODE_RISING_AND_FALLING_EDGE, //�����½��ز���
        TIMER_A_CAPTURE_INPUTSELECT_CCIxA,           //CCIxA:�ⲿ��������  ��CCIxB:���ڲ�ACLK����(�ֲ�)
        TIMER_A_CAPTURE_SYNCHRONOUS,                 //ͬ������
        TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE,     //����CCRN�����ж�
        TIMER_A_OUTPUTMODE_OUTBITVALUE               //���λֵ
    };
    // 5.��ʼ����ʱ���Ĳ���ģʽ
    MAP_Timer_A_initCapture(CAP_TIMA_SELECTION, &captureModeConfig_TA2);

    // 6.ѡ������ģʽ������ʼ����
    MAP_Timer_A_startCounter(CAP_TIMA_SELECTION, TIMER_A_CONTINUOUS_MODE);

    // 7.����жϱ�־λ
    //MAP_Timer_A_clearInterruptFlag(CAP_TIMA_SELECTION);                                   //�����ʱ������жϱ�־λ
    MAP_Timer_A_clearCaptureCompareInterrupt(CAP_TIMA_SELECTION, CAP_REGISTER_SELECTION); //��� CCR1 �����жϱ�־λ
    MAP_Interrupt_enableInterrupt(INT_TA2_N);                                             //������ʱ��A2�˿��ж�
}
// 10.��дTIMA ISR ��������

// TIMA2_CAP_STA ����״̬
uint8_t TIMA2_CAP_STA = 0;
uint16_t capTim[5];

uint16_t dt_1;
uint16_t dt_2;
uint16_t true_T = 60; // �����Ĳ�������

// ����Ƚ��������
#define NOISE 120

void TA2_N_IRQHandler(void)
{
    static uint8_t i = 0;
    // ��� CCR1 �����жϱ�־λ
    MAP_Timer_A_clearCaptureCompareInterrupt(CAP_TIMA_SELECTION, CAP_REGISTER_SELECTION);
    if (!TIMA2_CAP_STA) // δ����ɹ�
    {
        if (i == 0)
        {
            MAP_Timer_A_clearTimer(CAP_TIMA_SELECTION);
        }
        else
        {
            capTim[i - 1] = MAP_Timer_A_getCaptureCompareCount(CAP_TIMA_SELECTION, CAP_REGISTER_SELECTION);
            if (i > 6)
            {
                // clear CCR1 and stop Timer
                MAP_Timer_A_stopTimer(CAP_TIMA_SELECTION);
                MAP_Timer_A_clearTimer(CAP_TIMA_SELECTION);
                i = 0;
                TIMA2_CAP_STA = 1;
                dt_1 = capTim[3] - capTim[1];
                dt_2 = capTim[4] - capTim[2];
                if (dt_1 <= (dt_2 + NOISE) && dt_1 >= (dt_2 - NOISE))
                {
                    true_T = dt_1;
                    //printf("һ��\r\t");
                }
                else
                {
                    true_T = capTim[4] - capTim[0];
                    //printf("���\r\t");
                }
                //printf("t4:%d\r\t", capTim[4]);
                return;
            }
        }
        ++i;
    }
}
