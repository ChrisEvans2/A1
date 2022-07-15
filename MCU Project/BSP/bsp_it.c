#include "bsp_it.h"
#include "bsp_operation.h"

#define Cap_Times 4
static vu32 Internal_Cap_Register = 0; // ����ֵ
static vu8 CapTimer_SyncState = 0;     // �����ź�ͬ��״̬

/********************************************************************************************/
/***********************************   �жϺ���  ********************************************/

vu8 DMA_Transmit_Completed_Flag = 0; // DMA������ɱ�־
vu16 BSP_Signal_Capture_Value = 240; // ƽ������ֵ

#ifdef __MSP432P401R__
void TA2_N_IRQHandler(void)
{
    // ��� CCR1 �����жϱ�־λ
    MAP_Timer_A_clearCaptureCompareInterrupt(SIGNAL_CAPTURE_TIMER, SIGNAL_CAPTURE_TIMER_REGISTER);

    CapTimer_SyncState++;
    if (CapTimer_SyncState == 1) // ��һ�β���ֵλ���ź�ͬ�� ��ʹ�ø�����
    {
        MAP_Timer_A_getCaptureCompareCount(SIGNAL_CAPTURE_TIMER, SIGNAL_CAPTURE_TIMER_REGISTER); // ����ֵ����
        MAP_Timer_A_clearTimer(SIGNAL_CAPTURE_TIMER);                                            //��ն�ʱ�� ���´�0����
        Internal_Cap_Register = 0;
        return;
    }
    if (CapTimer_SyncState <= Cap_Times) //
    {
        Internal_Cap_Register += MAP_Timer_A_getCaptureCompareCount(SIGNAL_CAPTURE_TIMER, SIGNAL_CAPTURE_TIMER_REGISTER);
        return;
    }
    Internal_Cap_Register += MAP_Timer_A_getCaptureCompareCount(SIGNAL_CAPTURE_TIMER, SIGNAL_CAPTURE_TIMER_REGISTER);
    BSP_Timer_Stop(Signal_Capture_Timer);

    CapTimer_SyncState = 0;
    BSP_Signal_Capture_Value = Internal_Cap_Register / Cap_Times;
}

void DMA_INT1_IRQHandler(void)
{
    MAP_DMA_clearInterruptFlag(7);
    MAP_DMA_disableChannel(7); // dma will auto disable channel if complete
    // MAP_ADC14_clearInterruptFlag(ADC_INT0);

    BSP_Timer_Stop(Signal_Sample_Timer);
    DMA_Transmit_Completed_Flag = 1;
}
#else
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == *SIGNAL_CAPTURE_TIMER.Instance)
    {
        if (htim->Channel == SIGNAL_CAPTURE_TIMER_ACTIVE_CHANNEL)
        {
            CapTimer_SyncState++;
            if (CapTimer_SyncState == 1) // ��һ�β���ֵλ���ź�ͬ�� ��ʹ�ø�����
            {
                HAL_TIM_ReadCapturedValue(htim, SIGNAL_CAPTURE_TIMER_CHANNEL); // ����ֵ����
                Internal_Cap_Register = 0;
                return;
            }
            if (CapTimer_SyncState <= Cap_Times) //
            {
                Internal_Cap_Register += HAL_TIM_ReadCapturedValue(htim, SIGNAL_CAPTURE_TIMER_CHANNEL) + 1; //����TIM_CHANNEL_1 Ҫ�ǵü�1
                return;
            }
            Internal_Cap_Register += HAL_TIM_ReadCapturedValue(htim, SIGNAL_CAPTURE_TIMER_CHANNEL) + 1; //����TIM_CHANNEL_1 Ҫ�ǵü�1
            BSP_Timer_Stop(Signal_Capture_Timer);

            BSP_Signal_Capture_Value = Internal_Cap_Register / Cap_Times; //����TIM_CHANNEL_1 Ҫ�ǵü�1
        }
    }
}

void DMA_ADC_Transmit_Complete_Callback(DMA_HandleTypeDef *_hdma)
{
    if (_hdma->Instance == *DMA_ADC.Instance)
    {
        BSP_Timer_Stop(Signal_Sample_Timer);
        DMA_Transmit_Completed_Flag = 1;
    }
}

#endif
