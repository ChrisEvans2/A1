#include "bsp_init.h"
#include "log.h"

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
    TimA2_Cap_Init(TIMER_A_CLOCKSOURCE_DIVIDER_1);                            // ��8�� ��ʱ������ ������Ƚ�����Ƶ�ʣ�
    TimA0_Base_Init(TimerSourerFreq / 500000, TIMER_A_CLOCKSOURCE_DIVIDER_1); // ��8�� ��ʱ������ ��ADC����ʱ��Դ fs��
#else
    MX_TIM2_Init(); // ��8�� ��ʱ������ ������Ƚ�����Ƶ�ʣ�
    MX_TIM3_Init(); // ��8�� ��ʱ������ ��ADC����ʱ��Դ fs��

    HAL_TIM_IC_Start_IT(SIGNAL_CAPTURE_TIMER, SIGNAL_CAPTURE_TIMER_CHANNEL);
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
