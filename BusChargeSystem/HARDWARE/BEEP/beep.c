#include "beep.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//��������������
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////

//��ʼ��PF8Ϊ�����
//BEEP IO��ʼ��
void BEEP_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE); //ʹ��GPIOFʱ��

  //��ʼ����������Ӧ����GPIOF8
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      //��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     //�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;     //����
  GPIO_Init(GPIOF, &GPIO_InitStructure);             //��ʼ��GPIO

  GPIO_ResetBits(GPIOF, GPIO_Pin_8); //��������Ӧ����GPIOF8���ͣ�
}

/**
 * ��ȷ��ʾ��
 */
void OK_BEEP(void)
{
  BEEP = 1;
  delay_ms(200);
  BEEP = 0;
}

/**
 * ������ʾ��
 */
void ERR_BEEP(void)
{
  BEEP = 1;
  delay_ms(100);
  BEEP = 0;
  delay_ms(100);
  BEEP = 1;
  delay_ms(100);
  BEEP = 0;
  delay_ms(100);
  BEEP = 1;
  delay_ms(100);
  BEEP = 0;
}