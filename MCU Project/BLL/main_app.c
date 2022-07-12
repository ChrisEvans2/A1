/*******************************************
// 2021?? ����A??
// Bilibili��m-RNA
// E-mail:m-RNA@qq.com
// ��������:2021/11/11
*******************************************/

#include "config.h"
#include "bsp.h"
#include "bll.h"
#include "bll_oled.h"
#include "log.h"

void log_Internal_data(void);

u32 Signal_Captured_Value;                   // �ͻرȽ��� ��ʱ������ֵ
u16 Signal_ADC_Data[ADC_SAMPLING_NUM];       // �źŲ���ADC����
u16 WaveformData_Restored[OLED_X_MAX] = {0}; // ��ԭƽ����������

float THDx = 0.0f;                  // ʧ��Ȳ���ֵ
float NormalizedAm[5] = {0};        // ��һ����ֵ��2-5��г��
float FFT_Output[ADC_SAMPLING_NUM]; // FFT�������

int main(void)
{
    u16 i = 1;
    HAL_Init();
    SystemClock_Config();

    BSP_LED_KEY_BEEP_Init();
    BSP_Uart_PC_Init(); // ��7�� �������ã����ԣ�
    // InitGraph(); // OLED
    // OLEDInterface_Display_TiGame_Logo(); //  ��ʾ Ti�͵��� Logo

    BSP_Uart_Bluetooth_Init();                                       // ��7�� �������� ��������
    BSP_Sample_ADC_with_DMA_Init(Signal_ADC_Data, ADC_SAMPLING_NUM); // ��11�� ADC ��12�� DMA
    BSP_Sample_Timer_Init();                                         // ��8�� ��ʱ������ ��ADC����ʱ��Դ fs��������Ƚ�����Ƶ�ʣ�

    /* ��ʼ����� ���Բ��� */
    log_debug("All Init Completed!\r\n");
    printf("\r\n\r\n***********************  010  ****************************\r\n\r\n");

    while (1)
    {
        Signal_F0_Measure(&Signal_Captured_Value); // ����f �Ƶ�
        Signal_Fs_Adjust(Signal_Captured_Value);   // ����fs ���
        SignalSample_Start(Signal_ADC_Data);       // ����ADC�ɼ�DMA���� �ص�

        SignalSample_FFT_to_Am(Signal_ADC_Data, FFT_Output);            // ͨ��FFT �������Ƶ�ʷ�����ֵ �׵�
        NormalizedAm_And_CalculateTHD(FFT_Output, NormalizedAm, &THDx); // ��һ����ֵ �� ����THDx ��ɫ

        // OLEDInterface_Update_Data(NormalizedAm, THDx, Signal_Captured_Value);        // ����OLED�ϵ���Ϣ ��ɫ
        Transform_NormalizedAm_To_WaveformData(NormalizedAm, WaveformData_Restored); // ����һ����ֵת��Ϊ�������ݣ������ڶ�ΪOLED��X�ֱ���128�� Ʒ��
        // OLEDInterface_Update_Waveform(WaveformData_Restored);                        // ���������ݴ��� ��OLED�ڶ�Ӧλ�û������� ����
        Bluetooth_SendDate_To_Phone(NormalizedAm, THDx, WaveformData_Restored); // ������ͨ�����������ֻ� ��ɫ

        log_Internal_data(); // �ڲ�����
        delay_ms(100);       //��ʱ100ms

#ifdef DEBUG
        printf("\r\n\r\n***********************  0%u0  ****************************\r\n\r\n", ++i);
#endif

#ifdef Simulation
        if (++Simulation_Times_Index >= Simulation_Times)
            break;
    }
    log_debug("Simulate Finish!\r\n");
    while (1)
        ;
#else
    }
#endif
}

void log_Internal_data(void)
{
    u16 i;
    printf("\r\n\r\n***********************  ***  ****************************\r\n\r\n");

    log_indata("ADC Sampling Data(A Periods):\r\n");
    for (u16 i = 0; i <= SignalSampleFreq_Multiple; ++i)
    {
        log_indata("%u\r\n", Signal_ADC_Data[i]);
    }

    log_indata("\r\n*********************\r\n");

    log_indata("Am Data(a half):\r\n");
    for (i = 0; i < (ADC_SAMPLING_NUM >> 1); ++i)
    {
        // log_indata("[%03d] %.3f\r\n", i, Output[i]);
        log_indata("%.3f\r\n", FFT_Output[i]);
    }
    log_indata("\r\n");

    log_indata("\r\n*********************\r\n");

    log_indata("Waveform Data:\r\n");
    for (i = 0; i < OLED_X_MAX; ++i)
    {
        log_indata("%u\r\n", WaveformData_Restored[i]);
    }
}
