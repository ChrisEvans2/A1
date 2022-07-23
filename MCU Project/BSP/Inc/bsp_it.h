#ifndef __BSP_IT_H__
#define __BSP_IT_H__
#include "config.h"

#define CAP_TIMES 5

extern vu32 Cap_Val[]; // 调试用

extern vu8 DMA_Transmit_Completed_Flag; // DMA搬运完成标志
extern vu32 BSP_Signal_Avrg_Cap_Val;    // 平均捕获值

#endif
