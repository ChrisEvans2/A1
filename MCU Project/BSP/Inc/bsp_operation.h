#ifndef __BSP_OPERATION_H__
#define __BSP_OPERATION_H__
#include "config.h"

typedef enum
{
    Signal_Sample_Timer,
    Signal_Capture_Timer,
} BSP_Timer;

u32 BSP_Get_Signal_CCR(void);
void BSP_Set_Fs_ARR(u32 Fs_ARR);
void BSP_Timer_Stop(BSP_Timer Timer);
void BSP_Timer_Start(BSP_Timer Timer);
void BSP_ADC_DMA_Start(u16 *Data, u16 Num);
void BSP_Bluetooth_Send_Byte(u8 Data);
void BSP_Bluetooth_Send_HalfWord(u16 Data);

#endif
