#include "bsp_it.h"
#include "bsp_operation.h"

static vu32 Cap_Val[2] = {0};           // 捕获值
static vu8 CapTimer_SyncState = 0;      // 捕获信号同步状态
static vu8 CapTimer_Spilling_Times = 0; // 捕获溢出次数

/********************************************************************************************/
/***********************************   中断函数  ********************************************/

vu8 DMA_Transmit_Completed_Flag = 0; // DMA搬运完成标志
vu32 BSP_Signal_Avrg_Cap_Val = 0;    // 平均捕获值

#ifdef __MSP432P401R__

void TA2_N_IRQHandler(void)
{
    // 清除 CCR1 更新中断标志位
    MAP_Timer_A_clearCaptureCompareInterrupt(SIGNAL_CAPTURE_TIMER, SIGNAL_CAPTURE_TIMER_REGISTER);
    BITBAND_PERI(TIMER_A_CMSIS(SIGNAL_CAPTURE_TIMER)->CCTL[(SIGNAL_CAPTURE_TIMER_REGISTER >> 1) - 1], TIMER_A_CCTLN_COV_OFS) = 0;

    CapTimer_SyncState++;
    if (CapTimer_SyncState == 1) // 第一次捕获用于信号同步
    {
        Timer32_startTimer(TIMER32_0_BASE, true); // 一次计数
    }
    else if (CapTimer_SyncState >= CAP_TIMES)
    {
        BSP_Signal_Avrg_Cap_Val = (0xFFFFFFFF - Timer32_getValue(TIMER32_0_BASE)) / (CAP_TIMES - 1);

        BSP_Timer_Stop(Signal_Capture_Timer);
        CapTimer_SyncState = 0;
    }
}

void DMA_INT1_IRQHandler(void)
{
    MAP_DMA_clearInterruptFlag(7);

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
            if (CapTimer_SyncState == 1) // 第一次捕获值位于信号同步 不使用该数据
            {
                HAL_TIM_ReadCapturedValue(htim, SIGNAL_CAPTURE_TIMER_CHANNEL); // 将该值读走
            }
            else
            {
                BSP_Signal_Avrg_Cap_Val = HAL_TIM_ReadCapturedValue(htim, SIGNAL_CAPTURE_TIMER_CHANNEL) + 1; //※是TIM_CHANNEL_1 要记得加1
                BSP_Timer_Stop(Signal_Capture_Timer);
                CapTimer_SyncState = 0;
            }
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
