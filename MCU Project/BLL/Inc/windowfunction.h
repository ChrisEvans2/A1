/*
 * @Author: m-RNA m-RNA@qq.com
 * @Date: 2022-08-06 23:46:21
 * @LastEditors: m-RNA m-RNA@qq.com
 * @LastEditTime: 2022-08-07 00:35:59
 * @FilePath: \MDK-ARMe:\RNA_2021_Game_A\MCU Project\BLL\Inc\windowfunction.h
 * @Description: �Ӵ�������ļ�
 */

#ifndef __BLL_WINDOWFUNCTION_H__
#define __BLL_WINDOWFUNCTION_H__
#include "config.h"

typedef enum
{
    WithoutWinFun, // ���Ӵ�
    Boxcar,        // ���δ������Ӵ���
    Triang,        // ���Ǵ�
    Hanning,       // ������
    Hamming,       // ������
    Blackman,      // ����������
    Flattop,       // ƽ����
} Window_Function_Type;

/**
 * @description: ��������ʼ��
 * @param WinFun ����������
 *      @arg WithoutWinFun ���Ӵ�
 *      @arg Boxcar   ���δ������Ӵ���
 *      @arg Triang   ���Ǵ�
 *      @arg Hanning  ������
 *      @arg Hamming  ������
 *      @arg Blackman ����������
 *      @arg Flattop  ƽ����
 * @param Length ���ĳ���
 */
void Window_Function_Init(Window_Function_Type WinFun, u16 Length);

/**
 * @description: �Ӵ�����
 * @param Data  �Ӵ���ԭʼ����
 * @param Index ��������
 * @return float ���ݳ��Դ������Ӧ������ֵ
 */
float Window_Function_Add(u16 Data, u16 Index);

#endif
