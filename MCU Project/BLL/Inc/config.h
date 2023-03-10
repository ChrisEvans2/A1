#ifndef __CONFIG_H
#define __CONFIG_H

#define SIMULATION (0u) // 仿真输入信号

#define ENABLE_LOG_DETAIL (1u) // 打印过程细节
#define ENABLE_LOG_DEBUG (1u)  // 打印调试信息
#define ENABLE_LOG_DRAW (1u)   // 打印内部数据去画波形

#define ADC_SAMPLING_NUM 1024u          // ADC采样点数
#define SIGNAL_SAMPLE_FREQ_MULTIPLE 16u // 采样频率设定为信号基波频率的几倍（Fs = ? F0）
#define SIGNAL_SAMPLE_PERIOD_MIN (TIMER_SOURCE_FREQ / SIGNAL_SAMPLE_FREQ_MAX)
#define FX_VPP_MULTIPLE 10 // 计算的到的幅值乘以的倍数

#ifdef __MSP432P401R__
#include "sysinit.h"
#elif defined USE_HAL_DRIVER
#include "main.h"
#else
#error Doesn't contain top-level header file
#endif

#if defined __MSP432P401R__
#define TIMER_SOURCE_DIV TIMER_A_CLOCKSOURCE_DIVIDER_1 // 定时器时钟源分频
#define TIMER_SOURCE_FREQ (48000000u / TIMER_SOURCE_DIV)
#define SIGNAL_SAMPLE_FREQ_MAX 1000000u
#define ADC_MAX 16384u
#define ADC_RF_V_MV 2500u

#elif defined __STM32F1xx_HAL_H
#define TIMER_SOURCE_FREQ 48000000u
#define SIGNAL_SAMPLE_FREQ_MAX 1000000u
#define ADC_MAX 4096u
#define ADC_RF_V_MV 3300u

#elif defined STM32G431xx
#define TIMER_SOURCE_FREQ 170000000u
#define SIGNAL_SAMPLE_FREQ_MAX 2000000u
#define ADC_MAX 4096u
#define ADC_RF_V_MV 3300u

#endif

#if defined __MSP432P401R__
#include "timA.h"
#include "delay.h"
#include "adc.h"
#include "usart.h"
#include "usart2.h"
#include "led.h"
#define BLUETOOTH_UART EUSCI_A2_BASE
#define SIGNAL_SAMPLE_TIMER TIMER_A0_BASE
#define SIGNAL_CAPTURE_TIMER TIMER_A2_BASE                              //在这里改定时器
#define SIGNAL_CAPTURE_TIMER_REGISTER TIMER_A_CAPTURECOMPARE_REGISTER_2 //在这里改定时器通道
#define SIGNAL_CAPTURE_TIMER_PORT_PIN GPIO_PORT_P5, GPIO_PIN7           //在这里改复用引脚

#if (ADC_SAMPLING_NUM > 1024u)
// MSP432的DMA没开启DMA乒乓模式时，DMA一次最多搬运1024个点数。
#warning MSP32P4: If DMA ping-pong mode is not enabled, DMA can carry 1024 points at most.
#endif
#elif defined USE_HAL_DRIVER
#include "tim.h"
#include "adc.h"
#include "usart.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "bsp_led.h"
extern DMA_HandleTypeDef hdma_adc1;

#define delay_ms(MS) HAL_Delay(MS)
#define DMA_ADC &hdma_adc1
#define SIGNAL_SAMPLE_ADC &hadc1
#define SIGNAL_SAMPLE_TIMER &htim3
#define SIGNAL_CAPTURE_TIMER &htim2
#define SIGNAL_CAPTURE_TIMER_CHANNEL TIM_CHANNEL_1
#define SIGNAL_CAPTURE_TIMER_ACTIVE_CHANNEL HAL_TIM_ACTIVE_CHANNEL_1
#define OLED_IIC &hi2c1
#define BLUETOOTH_UART &huart2

#else
#endif

#if (TIMER_SOURCE_FREQ >= 0xFFFF * 1000)
#warning In this version, it is better for TIMER_SOURCE_FREQ to be Lower than 65535000.
#endif

#include "oled_config.h"

#if SIMULATION
#include "simulation.h"
#endif

#endif
