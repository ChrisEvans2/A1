/****************************************************/
// MSP432P401R
// 定时器32
// Bilibili：m-RNA
// E-mail:m-RNA@qq.com
// 创建日期:2021/9/8
/****************************************************/

#include "tim32.h"

void Tim32_0_Int_Init(uint32_t aar, uint8_t psc)
{
    Timer32_initModule(TIMER32_0_BASE, psc, TIMER32_32BIT, TIMER32_FREE_RUN_MODE);

    Timer32_setCount(TIMER32_0_BASE, aar);

    // Timer32_enableInterrupt(TIMER32_0_BASE);

    // MAP_Timer32_startTimer(TIMER32_0_BASE, false); //连续计数模式 false

    // MAP_Interrupt_enableInterrupt(INT_T32_INT1);
}
