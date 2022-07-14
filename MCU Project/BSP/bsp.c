#include "config.h"
#include "bsp.h"
#include "log.h"
#include "stdlib.h"

/********************************************************************************************/
/******************************   ��ʼ���ຯ��  *********************************************/

void BSP_GPIO_Init(void)
{
#ifdef __MSP432P401R__
    LED_Init(); // LED
#else
    MX_GPIO_Init();
#endif
}

void BSP_OLEDInterface_Init(void)
{
    log_debug("OLED Interface Init...\r\n");
    InitGraph();
}

void BSP_Uart_PC_Init(void)
{
#ifdef __MSP432P401R__
    uart_init(1382400); // ��7�� �������ã����ԣ�
#else
    MX_USART1_UART_Init(); // ��7�� �������ã����ԣ�
#endif
    log_debug("Uart PC Init Completed!\r\n");
}

void BSP_Uart_Bluetooth_Init(void)
{
    log_debug("Uart Bluetooth Init...\r\n");
#ifdef __MSP432P401R__
    usart3_init(9600);
#else
    MX_USART2_UART_Init();
#endif
}

void BSP_Sample_Timer_Init(void)
{
    log_debug("Sample Timer Init...\r\n");
#ifdef __MSP432P401R__
    TimA2_Cap_Init(TIMER_A_CLOCKSOURCE_DIVIDER_1);                           // ��8�� ��ʱ������ ������Ƚ�����Ƶ�ʣ�
    TimA0_Int_Init(TimerSourerFreq / 500000, TIMER_A_CLOCKSOURCE_DIVIDER_1); // ��8�� ��ʱ������ ��ADC����ʱ��Դ fs��
#else
    MX_TIM2_Init(); // ��8�� ��ʱ������ ������Ƚ�����Ƶ�ʣ�
    MX_TIM3_Init(); // ��8�� ��ʱ������ ��ADC����ʱ��Դ fs��

    HAL_TIM_IC_Start_IT(SIGNAL_SAMPLE_TIMER, SIGNAL_SAMPLE_TIMER_CHANNEL);
    HAL_TIM_Base_Start(SIGNAL_SAMPLE_TIMER);
#endif
}

void BSP_Sample_ADC_with_DMA_Init(u16 *Addr, u16 Length)
{
    log_debug("Sample ADC And DMA Init...\r\n");
#ifdef __MSP432P401R__
    adc_dma_init(Addr, Length); // ��12�� DMA
    ADC_Config();               // ��11�� ADC
#else
    MX_DMA_Init();
    MX_ADC1_Init();
#endif
}

/********************************************************************************************/
/***********************************   �жϺ���  ********************************************/

vu8 Synchronization_CaptureTimerState = 0; // �����ź�ͬ��״̬
vu8 DMA_Transmit_Completed_Flag = 0;       // DMA������ɱ�־
vu16 true_T = 240;                         // ����ֵ

#ifdef __MSP432P401R__
void TA2_N_IRQHandler(void)
{
    // ��� CCR1 �����жϱ�־λ
    MAP_Timer_A_clearCaptureCompareInterrupt(CAP_TIMA_SELECTION, CAP_REGISTER_SELECTION);
    if (!Synchronization_CaptureTimerState) // ��һ�β���ֵλ���ź�ͬ�� ��ʹ�ø�����
    {
        Synchronization_CaptureTimerState = 1;
        MAP_Timer_A_getCaptureCompareCount(CAP_TIMA_SELECTION, CAP_REGISTER_SELECTION);
        return;
    }
    true_T = MAP_Timer_A_getCaptureCompareCount(CAP_TIMA_SELECTION, CAP_REGISTER_SELECTION);
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
            true_T = HAL_TIM_ReadCapturedValue(htim, SIGNAL_SAMPLE_TIMER_CHANNEL) + 1; //����TIM_CHANNEL_1 Ҫ�ǵü�1
        }
    }
}
#endif

/********************************************************************************************/
/*********************************   �����ຯ��  ********************************************/
void BSP_ADC_DMA_Start(u16 *Data, u16 Num)
{
#if Simulation
#if 1 // ���ַ��������ź����ɷ�ʽѡ��ѡһ���ͺã�
    Simulate_Signal_Synthesizer(Data, Num);
#else
    Simulate_Signal_WaveformData(Data);
#endif
#else
#ifdef __MSP432P401R__
    MAP_DMA_setChannelTransfer(DMA_CH7_ADC14 | UDMA_PRI_SELECT, UDMA_MODE_BASIC, (void *)&ADC14->MEM[0], (void *)Data, Num);
    MAP_DMA_enableChannel(7); // ʹ��7ͨ����ADC��

    DMA_Transmit_Completed_Flag = 0;                          // ������ɱ�־λ����
    MAP_Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE); // ��ʼ���� ����ADC��ʱ����
    while (!DMA_Transmit_Completed_Flag)                      // �ȴ��������
        ;
#else
    HAL_ADC_Start_DMA(SIGNAL_SAMPLE_ADC, (u32 *)Data, Num);
    // ....

//    DMA_Transmit_Completed_Flag = 0;     // ������ɱ�־λ����
//    while (!DMA_Transmit_Completed_Flag) // �ȴ��������
//        ;
#endif
#endif
}

u32 BSP_Get_Signal_CCR(void)
{
#if Simulation
    if (Simulation_CCR_Data[Simulation_Times_Index] != Simulation_CCR)
        log_debug("Warning: Simulation_CCR Spilling!!!\r\n");
    return Simulation_CCR;
#else

#ifdef __MSP432P401R__

#else

#endif

    delay_ms(19); // �źŲ������ʱ��Ҳ�� 1.4ms * 6 = 8.2ms
    // while(Synchronization_CaptureTimerState == 0); // ���� �ٴ�ȷ��
    // Synchronization_CaptureTimerState = 0;
    return true_T;
#endif
}

void BSP_Set_Fs_ARR(u32 Fs_ARR)
{
#if Simulation
    Simulation_Set_Fs_ARR(Fs_ARR);
#else
#ifdef __MSP432P401R__
    MAP_Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0, Fs_ARR); // ����fs
#else
    __HAL_TIM_SET_AUTORELOAD(SIGNAL_SAMPLE_TIMER, Fs_ARR);
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
