#include "config.h"
#include "bsp.h"
#include "bll.h"
#include "bll_oled.h"
#include "log.h"

u32 Signal_Captured_Value; // �ͻرȽ��� ��ʱ������ֵ
u16 Signal_ADC_Data[ADC_SAMPLING_NUM]; // �źŲ���ADC����
u16 WaveformData[128] = {0}; // ��ԭƽ����������

float THDx = 0.0f; // ʧ��Ȳ���ֵ
float NormalizedAm[4] = {0}; // ��һ����ֵ��2-5��г��
float FFT_Output[ADC_SAMPLING_NUM]; // FFT�������

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    
    BSP_LED_KEY_BEEP_Init();
    BSP_Uart_PC_Init(); // ��7�� �������ã����ԣ�
    // InitGraph(); // OLED
    // OLEDInterface_Display_TiGame_Logo(); //  ��ʾ Ti�͵��� Logo 

    BSP_Uart_Bluetooth_Init(); // ��7�� �������� ��������
    BSP_Sample_ADC_with_DMA_Init(Signal_ADC_Data, ADC_SAMPLING_NUM);    // ��11�� ADC ��12�� DMA
    BSP_Sample_Timer_Init(); // ��8�� ��ʱ������ ��ADC����ʱ��Դ fs��������Ƚ�����Ƶ�ʣ�
    
    /* ��ʼ����� ���Բ��� */
    log_debug("All Init Completed!\r\n");
    while (1)
    {
        Signal_F0_Measure(&Signal_Captured_Value); // ����f �Ƶ�
        Signal_Fs_Adjust(Signal_Captured_Value);   // ����fs ���
        SignalSample_Start(Signal_ADC_Data);       // ����ADC�ɼ�DMA���� �ص�
        
        SignalSample_FFT_to_Am(Signal_ADC_Data, FFT_Output); // ͨ��FFT �������Ƶ�ʷ�����ֵ �׵�
        NormalizedAm_And_CalculateTHD(FFT_Output, NormalizedAm, &THDx); // ��һ����ֵ �� ����THDx ��ɫ
        
        // OLEDInterface_Update_Data(NormalizedAm, THDx, Signal_Captured_Value); // ����OLED�ϵ���Ϣ ��ɫ
        Transform_NormalizedAm_To_WaveformData(NormalizedAm, WaveformData); // ����һ����ֵת��Ϊ�������ݣ������ڶ�ΪOLED��X�ֱ���128�� Ʒ��
        // OLEDInterface_Update_Waveform(WaveformData); // ���������ݴ��� ��OLED�ڶ�Ӧλ�û������� ����
        
        Bluetooth_SendDate_To_Phone(NormalizedAm, THDx, WaveformData); // ������ͨ�����������ֻ� ��ɫ

        delay_ms(100); //��ʱ100ms
    }
}

