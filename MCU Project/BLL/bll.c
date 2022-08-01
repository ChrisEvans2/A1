#include "config.h"
#include "bsp_operation.h"
#include "bll.h"
#include "log.h"

u8 OverSamplingFlag = 0;

void Signal_F0_Measure(u32 *Captured_Value)
{
    LED_Y_On();
    log_detail("Signal F0 Measuring...\r\n");

    BSP_Timer_Start(Signal_Capture_Timer);
    *Captured_Value = BSP_Get_Signal_CCR();

    log_debug("F0 Captured Value:%u\r\n", *Captured_Value);
    log_debug("F0(Captured):%ukHz\r\n", TIMER_SOURCE_FREQ / 1000 / (*Captured_Value));
}

void Signal_Fs_Adjust(u32 Captured_Value)
{
    u32 Signal_Fs_ARR = 0;
    LED_R_On();
    log_detail("Signal Fs Adjusting...\r\n");

    Signal_Fs_ARR = Captured_Value / SIGNAL_SAMPLE_FREQ_MULTIPLE;
    OverSamplingFlag = Captured_Value <= (SIGNAL_SAMPLE_PERIOD_MIN * SIGNAL_SAMPLE_FREQ_MULTIPLE);
    if (OverSamplingFlag)
    {
        Signal_Fs_ARR += Captured_Value;
    }
    BSP_Set_Fs_ARR(Signal_Fs_ARR);

    log_Fs_data(Captured_Value, Signal_Fs_ARR, OverSamplingFlag);
}

void SignalSample_Start(u16 *Data)
{
    LED_W_Off();
    log_detail("Signal Sampling...\r\n");

    BSP_ADC_DMA_Start(Data, ADC_SAMPLING_NUM);

    log_detail("Signal Sample Completed!\r\n\r\n");
}

void Bluetooth_SendDate_To_Phone(float *NormalizedAm, float THDx, u16 *WaveData)
{
    uint8_t i;
    LED_RED_Off();
    LED_B_On();
    log_detail("Bluetooth Sending Date To Phone...\r\n");

    /* 发送THD */
    BSP_Bluetooth_Send_HalfWord(THDx * 100);

    /* 发送拟合值 */
    for (i = 0; i < 128; ++i)
    {
        BSP_Bluetooth_Send_HalfWord(WaveData[i]);
    }

    /* 发送归一化幅值 */
    for (i = 0; i < 4; ++i)
    {
        BSP_Bluetooth_Send_HalfWord(NormalizedAm[i] * 100);
    }
    log_detail("Bluetooth Sending Completed!\r\n");
    LED_W_Off();
}
