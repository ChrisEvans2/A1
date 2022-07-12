#include "bsp.h"
#include "log.h"
#include "stdlib.h"
#include "math.h"
#include "tim.h"
#include "adc.h"
#include "usart.h"
#include "dma.h"
#include "gpio.h"

#ifdef Simulation
#include "arm_math.h"

#define Simulate_WaveformDate_Period_Length SignalSampleFreq_Multiple

u32 Simulation_CCR[10] = {
    TimerSourerFreq /   1000, 
    TimerSourerFreq /  50000, 
    TimerSourerFreq / 100000, 
    
    TimerSourerFreq /  30000, 
    TimerSourerFreq / 600000, 
    TimerSourerFreq / 900000, 
    SignalSamplePeriod_MIN
};
#define Precession 10
float Simulation_NormAm[10][Precession - 1] = {
    {0.00f, 0.20f, 0.00f, 0.15f}, // ���������ź�1
    {0.00f, 0.08f, 0.15f, 0.00f}, // ���������ź�2
    {0.00f, 0.00f, 0.00f, 0.10f}, // ���������ź�3

    {0.00f, -0.1111111111f, 0.00f, 0.04f, 0.0f, -0.0204081633f, 0.0f, 0.0123456790f}, // ���ǲ�
    {0.00f, 0.3333333333f, 0.0f, 0.2f, 0.0f, 0.1428571429f, 0.0f, 0.1111111111f}, // ����
    {0.5f, 0.3333333333f, 0.25f, 0.2f, 0.1666666667f, 0.1428571429f, 0.125f, 0.1111111111f}, // ��ݲ���
    {0.0f, 0.0f, 0.0f, 0.0f}, // ���Ҳ�
};

extern u16 Compare_Min(float Mag[], u16 len);
static float OWaveDate[Simulate_WaveformDate_Period_Length];
static void Simulate_Signal_Synthesizer(float *NormAm, u16 *SimulateWaveData)
{
    u16 i;
    u16 MinIndex;

    for (int i = 0; i < Simulate_WaveformDate_Period_Length; ++i)
    {
        OWaveDate[i] = arm_sin_f32(2 * PI * i / ((float)Simulate_WaveformDate_Period_Length));
        for (int j = 0; j < Precession - 1; ++j)
        {
            OWaveDate[i] += arm_sin_f32(2 * PI * (j + 2) * i / ((float)Simulate_WaveformDate_Period_Length)) * NormAm[j];
        }
    }

    // �ҳ���С��С����λ��
    MinIndex = Compare_Min(OWaveDate, Simulate_WaveformDate_Period_Length);
    for (i = 0; i < Simulate_WaveformDate_Period_Length; ++i)
    {
        // ��С��ȫתΪΪ�������ٳ���1000��Ϊ����
        SimulateWaveData[i] = 1000 * (OWaveDate[i] - OWaveDate[MinIndex]);
        // ���1000����㶨�ģ���Ҫ̫��ͺ��ˣ�Ŀ���ǰ�С��ת��Ϊ����
    }
}

static u16 Simulation_ADC_Data[Simulate_WaveformDate_Period_Length] = {0};
void SquareWaveOut(void)
{
    u16 i;
	for(i = 0; i < Simulate_WaveformDate_Period_Length; i++)
    {
        if (i < Simulate_WaveformDate_Period_Length>>1)
            Simulation_ADC_Data[i] = 4095;
        else
            Simulation_ADC_Data[i] = 0;  
    }
}

// ���ǲ�
void TriangularWaveOut(void)
{
    u16 i, j;
	for(i = 0, j = 0; i < Simulate_WaveformDate_Period_Length; i++)
    {
        Simulation_ADC_Data[i] =  j * 2 * 4095 / Simulate_WaveformDate_Period_Length;
        
        if (i < Simulate_WaveformDate_Period_Length>>1)
            j++;
        else
            j--;       
    }
}

// ��ݲ�
void SawtoothWaveOut(void)
{
    u16 i;
	for(i = 0; i < Simulate_WaveformDate_Period_Length; i++)
    {
        Simulation_ADC_Data[i] =  i * 4096 / Simulate_WaveformDate_Period_Length;
    }
}

void SinWaveOut(void)
{
	u16 i;
	for (i = 0; i < Simulate_WaveformDate_Period_Length; i++)
	{
		Simulation_ADC_Data[i] = (u16)(2090 + 1990 * arm_sin_f32((2 * PI * i) / Simulate_WaveformDate_Period_Length));
	}
}

static void Simulate_Signal_WaveformData(u16 *SimulateWaveData)
{
    switch(Simulation_Times_Index)
    {
        case 0:
            SquareWaveOut();
            break;
        case 1:
            TriangularWaveOut();
            break;
        case 2:
            SawtoothWaveOut();
            break;    
        case 3:
            SinWaveOut();
            break;
        default:
            log_debug("It is same Simulate_WaveformDate!!!");
            break;
    }
    for(u16 i = 0; i < ADC_SAMPLING_NUM / Simulate_WaveformDate_Period_Length; ++i)
    {
        for(u16 j = 0; j < Simulate_WaveformDate_Period_Length; ++j)
        {
            SimulateWaveData[j + i * Simulate_WaveformDate_Period_Length] = Simulation_ADC_Data[j];
        }
    } 
}
#endif

void BSP_Sample_ADC_with_DMA_Init(u16 *Addr, u16 Length)
{
    MX_DMA_Init();
    MX_ADC1_Init();
    log_debug("config BSP_Sample_ADC_with_DMA_Init...\r\n");
}

void BSP_Sample_Timer_Init(void)
{
    log_debug("config BSP_Sample_Timer_Init...\r\n");
    MX_TIM3_Init(); // ��8�� ��ʱ������ ��ADC����ʱ��Դ fs��
    MX_TIM2_Init(); // ��8�� ��ʱ������ ������Ƚ�����Ƶ�ʣ�
    
    HAL_TIM_IC_Start_IT(SIGNAL_SAMPLE_TIMER, SIGNAL_SAMPLE_TIMER_CHANNEL); 
    HAL_TIM_Base_Start(SIGNAL_SAMPLE_TIMER);
}

void BSP_Uart_PC_Init(void)
{
    MX_USART1_UART_Init();// ��7�� �������ã����ԣ�
    log_debug("config BSP_Uart_PC...\r\n");
}

void BSP_Uart_Bluetooth_Init(void)
{
    MX_USART2_UART_Init();
    log_debug("config BSP_Uart_Bluetooth...\r\n");
}
void BSP_LED_KEY_BEEP_Init(void)
{
    MX_GPIO_Init();
}

void BSP_LED_Init(void)
{
    // LED_Init();  // LED
    
    log_debug("config BSP_LED_Init...\r\n");
}

void BSP_BEEP_Init(void)
{
    log_debug("config BSP_BEEP_Init...\r\n");
}

void BSP_KEY_Init(void)
{
    log_debug("config BSP_KEY_Init...\r\n");
}

void BSP_Init(void)
{
    BSP_Uart_PC_Init();
    BSP_Uart_Bluetooth_Init();
    BSP_LED_Init();
    // BSP_BEEP_Init();
    // BSP_KEY_Init();
}

void BSP_Set_Fs_CCR(u32 Fs_CCR)
{
    // MAP_Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0, Fs_CCR); // ����fs
    __HAL_TIM_SET_AUTORELOAD(SIGNAL_SAMPLE_TIMER, Fs_CCR);
}

u8 SignalCaptureTimerState = 0;
u16 true_T = 240;
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == *SIGNAL_SAMPLE_TIMER.Instance)
    {
        if(htim->Channel == SIGNAL_SAMPLE_TIMER_ACTIVE_CHANNEL)
        {
			//����PWM����
            true_T = HAL_TIM_ReadCapturedValue(htim, SIGNAL_SAMPLE_TIMER_CHANNEL) + 1; //����TIM_CHANNEL_1 Ҫ�ǵü�1
            SignalCaptureTimerState = 1;
        }	
    }
}

u32 BSP_Get_Signal_CCR(void)
{
#ifdef Simulation
    return Simulation_CCR[Simulation_Times_Index];
#else
    // SignalCaptureTimerState = 0;
    HAL_Delay(19); // �źŲ������ʱ��Ҳ�� 1.4ms * 6 = 8.2ms
    // while(SignalCaptureTimerState == 0); // ���� �ٴ�ȷ��
    
    return true_T;      
#endif
}

 

void BSP_ADC_DMA_Start(u16 *Data, u16 Num)
{
#ifdef Simulation
    // Simulate_Signal_WaveformData(Data);
    for(u16 i = 0; i < Num / Simulate_WaveformDate_Period_Length; ++i)
    {
        Simulate_Signal_Synthesizer(Simulation_NormAm[Simulation_Times_Index], &Data[i * Simulate_WaveformDate_Period_Length]);
    }
#else
    HAL_ADC_Start_DMA(SIGNAL_SAMPLE_ADC, (u32 *)Data, Num);
// ....

//    recv_done_flag = 0;     // ������ɱ�־λ����
//    while (!recv_done_flag) // �ȴ��������
//        ;
#endif    
}

void NVIC_Init(void)
{
    log_debug("config NVIC...\r\n");
}



void Delay_Init(void)
{
    // delay_init();       // ��4�� �δ���ʱ
    // log_debug("config Delay_Init...\r\n");
}



//void HAL_Init(void)
//{
//    NVIC_Init();
//    Delay_Init();
//}


