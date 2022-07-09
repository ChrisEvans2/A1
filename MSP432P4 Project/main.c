/*******************************************
// MSP432P401R
// 2021�� ����A��
// Bilibili��m-RNA
// E-mail:m-RNA@qq.com
// ��������:2021/11/11
*******************************************/

#include "sysinit.h"
#include "usart.h"
#include "led.h"
#include "timA.h"
#include "delay.h"
#include "baseboard.h"
#include "arm_math.h"
#include "arm_const_structs.h"
#include "adc.h"
#include "usart3.h"
#include "gameA.h"

// �Ƿ���� ������ DEBUG
#define DEBUG

float fft_outputbuf[ADC_SAMPLING_NUMBER * MM];           // FFT�������
static float fft_inputbuf[ADC_SAMPLING_NUMBER * 2 * MM]; // FFT��������

int main(void)
{
    uint16_t i;
    uint16_t j;
    uint8_t key_val; // ������ֵ
    char strBuf[9];  // OLED_printf�ݴ�

    float THDx;                  // THDx
    float gyh[4] = {0, 0, 0, 0}; //��һ����ֵ

    /***   �����ʼ������   ***/
    SysInit();       // ��3�� ʱ�����ã�48M��
    delay_init();    // ��4�� �δ���ʱ
    BaseBoardInit(); // ��2�� GPIO (KEY LED BEEP OLED)

    BEEP = 0; // �򿪷�����

    /**  ��ʾ Ti�͵��� Logo  **/
    DrawBitmap(0, 0, TiLOGO, 128, 64);   // Ti logo
    UpdateScreen();                      // ������Ļ
    SelectDownOLED();                    // ѡ������
    ClearScreen();                       // ����
    DrawBitmap(0, 0, GameLOGO, 128, 64); // ���� logo
    UpdateScreen();                      // ������Ļ

    uart_init(1382400);    // ��7�� �������� �����ԣ�
    usart3_init(9600);     // ��7�� �������� ��������
    TimA0_Int_Init(60, 1); // ��8�� ��ʱ������ ��ADC����ʱ��Դ fs��
    TimA2_Cap_Init();      // ��8�� ��ʱ������ ������Ƚ�����Ƶ�ʣ�
    adc_dma_init(1024);    // ��12�� DMA
    BEEP = 1;              // �ر� ������
    ADC_Config();          // ��11�� ADC

    /* ��ʼ����� ��ʾ���Բ��� */
    printf("Hello,MSP432!\r\n");
    DrawString(0, 0, "OK");
    UpdateScreen();

    MAP_Interrupt_enableMaster(); // �������ж�
    while (1)
    {
        key_val = KEY_Scan(0); //ɨ���ֵ

        switch (key_val)
        {
        case KEY1_OnBoard_PRES: // KEY1 ���£� ��ʼһ������

            /*****************************   ����f������fs   ******************************/

            LED_B = 0;         // ������
            LED_G = 1;         // ���̵�
            TIMA2_CAP_STA = 0; // ������ɱ�־λ����

            MAP_Timer_A_clearTimer(CAP_TIMA_SELECTION);                            // ����ʱ������
            MAP_Timer_A_startCounter(CAP_TIMA_SELECTION, TIMER_A_CONTINUOUS_MODE); // ��ʼ����f

            delay_ms(200); // �ȴ�f�������

            MAP_Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0, true_T); // ����fs

            WaitingAnimat(1); // �ȴ��������ȴ���ʱ��A�ȶ���(...)

            LED_G = 0; //���̵�

            /********************************   ����DMA����   ********************************/

            MAP_DMA_setChannelTransfer(DMA_CH7_ADC14 | UDMA_PRI_SELECT, UDMA_MODE_BASIC, (void *)&ADC14->MEM[0], (void *)adc_inputbuf, 1024);
            MAP_DMA_enableChannel(7); // ʹ��7ͨ����ADC��

            MAP_Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE); // ��ʼ���� ����ADC��ʱ����

            recv_done_flag = 0;     // ������ɱ�־λ����
            while (!recv_done_flag) // �ȴ��������
                ;

            /**********************************   FFT ����   **********************************/
            LED_RED = 1; // �����

            /**  ������ת��Ϊ����  **/
            for (j = 0; j < MM; ++j)
            {
                for (i = 0; i < ADC_SAMPLING_NUMBER; ++i)
                {
                    fft_inputbuf[2 * i] = adc_inputbuf[j][i]; // ʵ��ΪADC
                    fft_inputbuf[2 * i + 1] = 0;              // �鲿Ϊ0
                }
            }

            arm_cfft_f32(&arm_cfft_sR_f32_len1024, fft_inputbuf, 0, 1);               // FFT����
            arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, ADC_SAMPLING_NUMBER * MM); //��������������ģ�÷�ֵ

            LED_RED = 0; // �غ��

            /********************      �ҳ�����г��λ�� �����һ����ֵ      *******************/

            /* �ҳ�����λ�� */
            f0 = FloatMax(fft_outputbuf);

            for (i = 0; i < 4; ++i)
            {
                /**   �ҳ�г��λ��   **/
                fx[i] = FloatMax_WithWindow(fft_outputbuf, f0 * (i + 2) - (FDBS / 2), f0 * (i + 2) + (FDBS / 2)); // �Ż������㷨 ����׼ȷ

                /**  �����һ����ֵ  **/
                gyh[i] = floor(fft_outputbuf[fx[i]] / fft_outputbuf[f0] * 100.0f) / 100.0f; // ����ȡ�� ����С
            }

            /****************************   THD��������ʾ��OLED   ****************************/

            THDx = THDx_calculate(); // ����THDx
            UpdateGYH(gyh, THDx);    // ���µ���Ļ

            /*********************   ���㲨�κ���ʾ��OLED�뷢�����ֻ�   *********************/

            ShowWave_AndTran(gyh); //���㡢ת������ʾ����

            BEEP = 0;                               // ��ʼ����
            BluetoothSendDate(gyh, THDx, waveTran); // �������ݸ��ֻ�
            BEEP = 1;                               // ֹͣ����

// ����DEBUG ������ӡ�ڲ���Ϣ
#ifdef DEBUG
            printf("\r\nADC��������:\r\n");
            for (j = 0; j < MM; ++j)
            {
                for (i = 0; i < ADC_SAMPLING_NUMBER; ++i)
                {
                    printf("%d\r\n", adc_inputbuf[j][i]);
                }
            }

            printf("\r\nFFT�����ֵ����:\r\n");
            for (i = 0; i < ADC_SAMPLING_NUMBER; ++i)
            {
                printf("[%d]:%.3f\r\n", i, fft_outputbuf[i]);
            }

            printf("\r\n�������ڣ�%.2fus\r\n", true_T / 3.0f);
            printf("����λ�ã�%d\r\n", f0);
            for (i = 0; i < 4; ++i)
            {
                printf("%dгλ�ã�%d\r\n", i + 2, f0);
            }

            printf("\r\nTHD��%2.2f\r\n", THDx);

            printf("��һ����ֵ��\r\n");
            printf("��:1.00\r\n");
            for (i = 0; i < 4; ++i)
            {
                printf("%d:%1.2f\r\n", i + 2, gyh[i]);
            }
#endif
            break;

        case KEY2_PRES:   // ���Ե�Ƭ���Ƿ���
            LED_RED ^= 1; // ��ת��Ƶ�ƽ
            break;

        case KEY3_PRES:   // ���Թ���Ƚ����Ƿ���������
            LED_RED ^= 1; // ��ת��Ƶ�ƽ
            TIMA2_CAP_STA = 0;
            MAP_Timer_A_clearTimer(CAP_TIMA_SELECTION);
            MAP_Timer_A_startCounter(CAP_TIMA_SELECTION, TIMER_A_CONTINUOUS_MODE);
            snprintf(strBuf, 9, "T:%4dus", true_T / 3); // ��������
            DrawString(80, 0, strBuf);
            UpdateScreen();

            break;

        case KEY4_PRES:   // ���������Ƿ���ȷ����
            LED_RED ^= 1; // ��ת��Ƶ�ƽ
            MAP_UART_transmitData(HC_05_USART_BASE, 1920);
            break;
        }

        key_val = NOT_PRES; //�����ֵ
        delay_ms(10);       //��ʱ10ms
    }
}
