#include "log.h"
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
#include "simulation.h"
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
    Simulate_Signal_Synthesizer(Data);
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


