/*******************************************
// 2021?? ����A??
// Bilibili��m-RNA
// E-mail:m-RNA@qq.com
// ��������:2021/11/11
*******************************************/

#include "config.h"
#include "bsp.h"
#include "bll.h"
#include "log.h"
#include "my_math.h"
#include "oled_interface.h"

u32 Signal_Captured_Value;                   // �ͻرȽ��� ��ʱ������ֵ
u16 Signal_ADC_Data[ADC_SAMPLING_NUM];       // �źŲ���ADC����
u16 WaveformData_Restored[OLED_X_MAX] = {0}; // ��ԭƽ����������

float THDx = 0.0f;                      // ʧ��Ȳ���ֵ
float NormalizedAm[4] = {0};            // ��һ����ֵ��2-5��г��
float Phase[5] = {0};                   // ��������λ��ռλ����û���ϣ�
float Amplitude_Data[ADC_SAMPLING_NUM]; // ����Ƶ�ʷ�����ֵ(FFT��)

int main(void)
{
    u16 i = 1;
    HAL_Init();
    SystemClock_Config();

    BSP_GPIO_Init();    // ��2�� GPIO����
    BSP_Uart_PC_Init(); // ��7�� �������ã����ԣ�

    BSP_OLEDInterface_Init();            // ��10�� OLED��ʾ
    OLEDInterface_Display_TiGame_Logo(); // ��ʾ Ti�͵��� Logo

    BSP_Uart_Bluetooth_Init(); // ��7�� �������� ��������

    BSP_Sample_ADC_with_DMA_Init(Signal_ADC_Data, ADC_SAMPLING_NUM); // ��11�� ADC ��12�� DMA
    BSP_Sample_Timer_Init();                                         // ��8�� ��ʱ������ ��ADC����ʱ��Դ fs��������Ƚ�����Ƶ�ʣ�

    /* ��ʼ����� ���Բ��� */
    log_debug("All Init Completed!\r\n");
    log_debug("\r\n\r\n***********************  000  ****************************\r\n\r\n");

    while (1)
    {
        Signal_F0_Measure(&Signal_Captured_Value); // ����f �Ƶ�
        Signal_Fs_Adjust(Signal_Captured_Value);   // ����fs(�ж��Ƿ���Ҫ��Ч����) ���
        SignalSample_Start(Signal_ADC_Data);       // ����ADC�ɼ�DMA���� �ص�

        CalculateAmplitude_By_FFT(Amplitude_Data, Signal_ADC_Data);                // ͨ��FFT �������Ƶ�ʷ�����ֵ �׵�
        NormalizedAm_And_CalculateTHD(Phase, NormalizedAm, &THDx, Amplitude_Data); // ��һ����ֵ �����������λ ����THDx ��ɫ
        Restore_Waveform(WaveformData_Restored, NormalizedAm, Phase);              // �ù�һ����ֵ+��������λ ��ԭ���Σ������ڶ�ΪOLED��X�ֱ���128�� Ʒ��

        OLEDInterface_Update_Data(NormalizedAm, THDx, Signal_Captured_Value); // OLED��ʾ��Ϣ���� ��ɫ
        OLEDInterface_Update_Waveform(WaveformData_Restored);                 // OLED��ʾ���θ��� ����

        Bluetooth_SendDate_To_Phone(NormalizedAm, THDx, WaveformData_Restored); // ������ͨ�����������ֻ� ��ɫ

        log_Internal_data(Signal_ADC_Data, Amplitude_Data,
                          WaveformData_Restored, NormalizedAm, Phase,
                          THDx, Signal_Captured_Value); // ��ӡ�ڲ�����

        log_debug("\r\n\r\n***********************  0%u0  ****************************\r\n\r\n", i++);

#if !Simulation
        delay_ms(100); //��ʱ100ms
    }
}
#else
        if (++Simulation_Times_Index >= Simulation_Times)
        {
                Simulation_Times_Index = 0;
            log_debug("Simulate Loop!\r\n");
        }
    }
}
#endif
