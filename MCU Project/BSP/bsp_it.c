#include "bsp_it.h"
#include "log.h"
#include "stdlib.h"

/********************************************************************************************/
/***********************************   �жϺ���  ********************************************/

vu8 Synchronization_CaptureTimerState = 0; // �����ź�ͬ��״̬
vu8 DMA_Transmit_Completed_Flag = 0;       // DMA������ɱ�־
vu16 BSP_Signal_Capture_Value = 240;       // ����ֵ

#ifdef __MSP432P401R__
void TA2_N_IRQHandler(void)
{
    // ��� CCR1 �����жϱ�־λ
    MAP_Timer_A_clearCaptureCompareInterrupt(SIGNAL_CAPTURE_TIMER, SIGNAL_CAPTURE_TIMER_REGISTER);
    if (!Synchronization_CaptureTimerState) // ��һ�β���ֵλ���ź�ͬ�� ��ʹ�ø�����
    {
        Synchronization_CaptureTimerState = 1;
        MAP_Timer_A_getCaptureCompareCount(SIGNAL_CAPTURE_TIMER, SIGNAL_CAPTURE_TIMER_REGISTER);
        return;
    }
    BSP_Signal_Capture_Value = MAP_Timer_A_getCaptureCompareCount(SIGNAL_CAPTURE_TIMER, SIGNAL_CAPTURE_TIMER_REGISTER);
}

void DMA_INT1_IRQHandler(void)
{
    MAP_DMA_clearInterruptFlag(7);

    MAP_Timer_A_stopTimer(TIMER_A0_BASE);
    MAP_Timer_A_clearInterruptFlag(TIMER_A0_BASE);
    MAP_ADC14_clearInterruptFlag(ADC_INT0);

    DMA_Transmit_Completed_Flag = 1;

    // DMA_disableChannel(7);	// dma will auto disable channel if complete
}
#else
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == *SIGNAL_SAMPLE_TIMER.Instance)
    {
        if (htim->Channel == SIGNAL_SAMPLE_TIMER_ACTIVE_CHANNEL)
        {
            if (!Synchronization_CaptureTimerState) // ��һ�β���ֵλ���ź�ͬ�� ��ʹ�ø�����
            {
                Synchronization_CaptureTimerState = 1;
                HAL_TIM_ReadCapturedValue(htim, SIGNAL_SAMPLE_TIMER_CHANNEL);
                return;
            }
            BSP_Signal_Capture_Value = HAL_TIM_ReadCapturedValue(htim, SIGNAL_SAMPLE_TIMER_CHANNEL) + 1; //����TIM_CHANNEL_1 Ҫ�ǵü�1
        }
    }
}
#endif
