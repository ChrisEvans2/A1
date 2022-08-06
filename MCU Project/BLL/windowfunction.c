#include "windowfunction.h"
#include "arm_math.h"

static float Win_Function_Buf[ADC_SAMPLING_NUM];

static void boxcar(u16 Length) // 矩形窗（不加窗）
{
    for (u16 i = 0; i < Length; i++)
    {
        Win_Function_Buf[i] = 1;
    }
}

static void triang(u16 Length) // 三角窗
{
    for (u16 i = 0; i < Length / 2; i++)
    {
        Win_Function_Buf[Length - i - 1] = Win_Function_Buf[i] = 2 * i / (float)Length;
    }
}

static void hanning(u16 Length) // 汉明窗
{
    for (u16 i = 0; i < Length; i++)
    {
        Win_Function_Buf[i] = 0.5f * (1 - arm_cos_f32(2 * PI * i / (Length + 1)));
    }
}

static void hamming(u16 Length) // 海明窗
{
    for (u16 i = 0; i < Length; i++)
    {
        Win_Function_Buf[i] = 0.54f - 0.46f * arm_cos_f32(2 * PI * i / (Length - 1));
    }
}

static void blackman(u16 Length) // 布莱克曼窗
{
    for (u16 i = 0; i < Length; i++)
    {
        Win_Function_Buf[i] = 0.42f - 0.5f * arm_cos_f32(2 * PI * i / (Length - 1)) + 0.08f * arm_cos_f32(4 * PI * i / (Length - 1));
    }
}

static void flattop(u16 Length) // 平顶窗
{
    for (u16 i = 0; i < Length; i++)
    {
        Win_Function_Buf[i] = (1 - 1.93f * arm_cos_f32(2 * PI * i / (Length - 1)) + 1.29f * arm_cos_f32(4 * PI * i / (Length - 1)) - 0.388f * arm_cos_f32(6 * PI * i / (Length - 1)) + 0.0322f * arm_cos_f32(8 * PI * i / (Length - 1))) / 4.634f;
    }
}

/* 窗函数初始化 */
void Window_Function_Init(Window_Function_Type WinFun, u16 Length)
{
    switch (WinFun)
    {
    case WithoutWinFun:
    case Boxcar:
        boxcar(Length); // 矩形窗（不加窗）
        break;
    case Triang:
        triang(Length); // 三角窗
        break;
    case Hanning:
        hanning(Length); // 汉明窗
        break;
    case Hamming:
        hamming(Length); // 海明窗
        break;
    case Blackman:
        blackman(Length); // 布莱克曼窗
        break;
    case Flattop:
        flattop(Length); // 平顶窗
        break;
    default:
        boxcar(Length); // 默认不加窗
        break;
    }
}

float Window_Function_Add(u16 Data, u16 Index)
{
    return Data *= Win_Function_Buf[Index];
}
