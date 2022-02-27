#ifndef __MAIN_H
#define __MAIN_H
#include "sys.h"

#define ID_LEN 12   // �û���ID����(�ֽ�)
#define NAME_LEN 20 // �û�������(�ֽ�)

/* ѡ�����ڵ�ģʽ */
#define D 0
#define H 1
#define M 2
#define DHM 3

/* ���岼������ */
typedef enum
{
    TRUE,
    FALSE
} Boolean;

/* ������������� */
extern u8 adminID[ID_LEN + 1];    // ����ԱID����
extern u8 cardID[ID_LEN + 1];     // ���ID����
extern u8 userName[NAME_LEN + 1]; // �û���
extern u16 balance;               // �˻����
/* keyFunȡֵ -> KEY_NULL:�޲��� KEY0_VALUE:���ж�����Ϣ KEY1_VALUE:����û��� KEY2_VALUE:ɾ���û��� */
extern u8 keyFun;

/* ���������к����ӿڶ��� */
void BCSInit(void);                    // DoorControlSystem(DCS)��ʼ��
void SysRunning(void);                 // ϵͳ�������
void ConsumeTips(void);                // ������ʾ��
void BCSRunning(void);                 // ��������
void LcdDesktop(void);                 // �û�����
void RechargeMenu(void);               // ��ʾ��ֵ�˵�
void BalanceRecharge(void);            // ����û����
u16 GetBalance();                      // ��ȡ�û����
Boolean BalancdInc(int balance);       // �����û����
Boolean SearchID(u8 *cardID);          // �����û���ID
Boolean IsEquals(u8 *arr1, u8 *arr2);  // ����Ƚ�
Boolean IsNullID(u8 *cardID, u8 size); // �ж�ID�Ƿ�Ϊ��
Boolean IsNullData(u8 *data, u8 size); // �ж������Ƿ�Ϊ��
void ReadCardTips(void);               // ������ʾ��Ϣ
char ReadCard(void);                   // ����
void ManageUserMenu(void);             // �����û��˵�
void ManageUser(void);                 // �����û�
void AddUserMenu(void);                // ��ʾ����û��˵�
void DelUserMenu(void);                // ��ʾɾ���û��˵�
void UserSignup(void);                 // �û�ע��
void AddUser(u8 *cardID);              // ����û�
Boolean AddData(u8 *cardID);           // ���ID
void DeleteUser(void);                 // ɾ���û�
void RemoveAllUser(void);              // ɾ�������û�
void RemoveUser(u8 *cardID);           // ɾ��ָ���û�
Boolean RemoveData(u8 *cardID);        // ɾ��ָ���û�����
void RemoveAllData(void);              // ɾ������ID
u8 ShowTime(u8 timeMode);              // ��ʾʱ��

#endif
