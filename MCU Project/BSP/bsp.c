#include "config.h"
#include "bsp.h"
#include "log.h"
#include "stdlib.h"
#ifdef Simulation
#include "simulation.h"
#endif

/********************************************************************************************/
/******************************   ��ʼ���ຯ��  *********************************************/

void BSP_LED_KEY_BEEP_Init(void)
{
#ifdef __MSP432P401R__
    LED_Init(); // LED
#else
    MX_GPIO_Init();
#endif
}

void BSP_OLEDInterface_Init(void)
{
    InitGraph(); 
}

void BSP_Uart_PC_Init(void)
{
#ifdef __MSP432P401R__
    uart_init(1382400); // ��7�� �������ã����ԣ�
#else
    MX_USART1_UART_Init(); // ��7�� �������ã����ԣ�
#endif
    log_debug("Uart_PC Init Completed!\r\n");
}

void BSP_Uart_Bluetooth_Init(void)
{
#ifdef __MSP432P401R__
    usart3_init(9600);
#else
    MX_USART2_UART_Init();
#endif
    log_debug("config BSP_Uart_Bluetooth...\r\n");
}

void BSP_Sample_Timer_Init(void)
{
    log_debug("config BSP_Sample_Timer_Init...\r\n");
#ifdef __MSP432P401R__
    TimA0_Int_Init(60, 1); // ��8�� ��ʱ������ ��ADC����ʱ��Դ fs��
    TimA2_Cap_Init();      // ��8�� ��ʱ������ ������Ƚ�����Ƶ�ʣ�
#else
    MX_TIM3_Init(); // ��8�� ��ʱ������ ��ADC����ʱ��Դ fs��
    MX_TIM2_Init(); // ��8�� ��ʱ������ ������Ƚ�����Ƶ�ʣ�

    HAL_TIM_IC_Start_IT(SIGNAL_SAMPLE_TIMER, SIGNAL_SAMPLE_TIMER_CHANNEL);
    HAL_TIM_Base_Start(SIGNAL_SAMPLE_TIMER);
#endif
}

void BSP_Sample_ADC_with_DMA_Init(u16 *Addr, u16 Length)
{
#ifdef __MSP432P401R__
    adc_dma_init(Addr, Length); // ��12�� DMA
    ADC_Config();               // ��11�� ADC
#else
    MX_DMA_Init();
    MX_ADC1_Init();
#endif
    log_debug("config BSP_Sample_ADC_with_DMA_Init...\r\n");
}

/********************************************************************************************/
/***********************************   �жϺ���  ********************************************/

vu8 SignalCaptureTimerState = 0; // ����״̬
vu16 true_T = 240;               // �����Ĳ�������
#ifdef __MSP432P401R__
vu16 capTim[5];

uint16_t dt_1;
uint16_t dt_2;

// ����Ƚ��������
#define NOISE (1 << 5)

void TA2_N_IRQHandler(void)
{
    static uint8_t i = 0;
    // ��� CCR1 �����жϱ�־λ
    MAP_Timer_A_clearCaptureCompareInterrupt(CAP_TIMA_SELECTION, CAP_REGISTER_SELECTION);
    if (!SignalCaptureTimerState) // δ����ɹ�
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
                SignalCaptureTimerState = 1;
                dt_1 = capTim[3] - capTim[1];
                dt_2 = capTim[4] - capTim[2];
                if (dt_1 <= (dt_2 + NOISE) && dt_1 >= (dt_2 - NOISE))
                {
                    true_T = dt_1;
                    // printf("һ��\r\t");
                }
                else
                {
                    true_T = capTim[4] - capTim[0];
                    // printf("���\r\t");
                }
                // printf("t4:%d\r\t", capTim[4]);
                return;
            }
        }
        ++i;
    }
}
#else
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == *SIGNAL_SAMPLE_TIMER.Instance)
    {
        if (htim->Channel == SIGNAL_SAMPLE_TIMER_ACTIVE_CHANNEL)
        {
            //����PWM����
            true_T = HAL_TIM_ReadCapturedValue(htim, SIGNAL_SAMPLE_TIMER_CHANNEL) + 1; //����TIM_CHANNEL_1 Ҫ�ǵü�1
            SignalCaptureTimerState = 1;
        }
    }
}
#endif

/********************************************************************************************/
/*********************************   �����ຯ��  ********************************************/

u32 BSP_Get_Signal_CCR(void)
{
#ifdef Simulation
    return Simulation_CCR[Simulation_Times_Index];
#else

#ifdef __MSP432P401R__
    
#else
    
#endif
    
    delay_ms(19); // �źŲ������ʱ��Ҳ�� 1.4ms * 6 = 8.2ms
    // while(SignalCaptureTimerState == 0); // ���� �ٴ�ȷ��
    // SignalCaptureTimerState = 0;
    return true_T;
#endif
}

void BSP_Set_Fs_CCR(u32 Fs_CCR)
{
#ifdef __MSP432P401R__
    MAP_Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0, Fs_CCR); // ����fs
#else
    __HAL_TIM_SET_AUTORELOAD(SIGNAL_SAMPLE_TIMER, Fs_CCR);
#endif
}

void BSP_ADC_DMA_Start(u16 *Data, u16 Num)
{
#ifdef Simulation
#if 1 // ���ַ��������ź����ɷ�ʽѡ��ѡһ���ͺã�
    Simulate_Signal_Synthesizer(Data);
#else
    Simulate_Signal_WaveformData(Data); 
#endif
    
#else
#ifdef __MSP432P401R__
    MAP_DMA_setChannelTransfer(DMA_CH7_ADC14 | UDMA_PRI_SELECT, UDMA_MODE_BASIC, (void *)&ADC14->MEM[0], (void *)Data, Num);
    MAP_DMA_enableChannel(7); // ʹ��7ͨ����ADC��

    MAP_Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE); // ��ʼ���� ����ADC��ʱ����
    recv_done_flag = 0;                                       // ������ɱ�־λ����
    while (!recv_done_flag)                                   // �ȴ��������
        ;
#else
    HAL_ADC_Start_DMA(SIGNAL_SAMPLE_ADC, (u32 *)Data, Num);
    // ....

//    recv_done_flag = 0;     // ������ɱ�־λ����
//    while (!recv_done_flag) // �ȴ��������
//        ;
#endif

#endif
}

void BSP_Bluetooth_SendByte(u8 Data)
{
#if defined __MSP432P401R__
    MAP_UART_transmitData(BLUETOOTH_UART, Data);
#elif defined USE_HAL_DRIVER
    HAL_UART_Transmit(BLUETOOTH_UART, &Data, 1, 50);
#else
#error Please Transplant Function: BSP_Bluetooth_SendByte();
#endif
}

#ifdef __MSP432P401R__
void NVIC_Init(void)
{
}

void Delay_Init(void)
{
    delay_init();
}

void HAL_Init(void)
{
    NVIC_Init();
}

#endif
