#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "beep.h"
#include "lcd.h"
#include "key.h"
#include "malloc.h"
#include "w25qxx.h"
#include "RC522.h"
#include "spi.h"
#include "string.h"
#include "timer.h"
#include "main.h"
#include "exti.h"
// #include "sram.h"
// #include "usmart.h"
// #include "sdio_sdcard.h"
// #include "ff.h"
// #include "exfuns.h"
// #include "fontupd.h"
// #include "text.h"
// #include "rtc.h"

/** RFID���߷�ʽ
	PC10 -- CLK
	PC11 -- MISO
	PC12 -- MOSI
	PA4  -- NSS
	PA6  -- RST
*/
/**
 * @author DXR
 * @create 2021/12/2 13:56
 * <p>
 * ����Э��(δ����У��λ)��
 * 0������ͷ     1Byte  0x5B ��'['
 * 1����������   1Byte  * @ $ = ( )
 * 2�����ݶ�     xByte  x <= 200Byte
 * x+2������β   1Byte  0x5D ��']'
 * <p>
 * �����ʽ(��һ���ֽ�Ϊ�������ͣ����Ϊ���ݶ�)��
 * ����ID��*ID
 * ��ѯ�û�����@ID  ���ض���ID���û���
 * ��ѯ��$ID    ���ض�ӦID�����
 * ������=ID,���
 * ע���û���(ID,�û���,���
 * ���ݶ������ж��� �� , �ָ�
 * �磺[(ID,�û���,���]  ��������Ϊ:(  ���ݶ�Ϊ:ID,�û���,���  ��Ϊע���û�
 * ɾ���û����ݣ�)ID
 * ��������ͺ����������ɹ�������ͬ�������͵ķ��� ���򷵻� '~'+������Ϣ
 */
u8 adminID[ID_LEN + 1] = "7155135179";	// ����Ա����(�ѽ�ʮ������ת��ʮ����) ����ɾ���û���Ȩ�޿��� �ɸ������������޸Ĺ���Ա����
u8 cardID[ID_LEN + 1];					// �����ͨ�û�ID���� +1��Ϊ�˴��'\0'
u8 userName[NAME_LEN + 1];				// �û��� +1��Ϊ�˴��'\0'
u16 balance;							// �˻����
u8 keyFun;								// �ȳ�ʼ��û���κΰ������µ�״̬
const u8 DATA_SIZE = ID_LEN + NAME_LEN; // ���ݴ�С

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����ϵͳ�ж����ȼ�����2
	delay_init(168);								//��ʼ����ʱ����
	LED_Init();										//��ʼ��LED
	BEEP_Init();									//��ʼ��������
	KEY_Init();										//��ʼ������
	EXTIX_Init();									// ��ʼ���ⲿ�ж� ���ڶ�ȡ����ֵ
	W25QXX_Init();									//��ʼ��W25Q128
	uart_init(115200);								//����1��ʼ��������Ϊ115200
	LCD_Init();										// LCD��ʼ��
	RC522_SPI3_Init();
	MFRC522_Initializtion();
	POINT_COLOR = RED; // ��������Ϊ��ɫ
	LCD_ShowString(30, 110, 200, 16, 16, "System loading...");
	BCSInit();	  // BusChargeSystem(BCS)��ʼ��
	SysRunning(); // �������
}

/**
 * @brief �������
 */
void SysRunning(void)
{
	while (1)
	{
		LcdDesktop();
		switch (keyFun)
		{
		case KEY0_VALUE:
			LCD_Clear(BLUE);
			BCSRunning();	   //  ϵͳ����
			keyFun = KEY_NULL; // ��ΪKEY_NULL -> ��ֹ�´�ѭ���������ô˹���
			break;
		case KEY1_VALUE:
			LCD_Clear(BLUE);
			ManageUser();	   // �����û�
			keyFun = KEY_NULL; // ��ΪKEY_NULL -> ��ֹ�´�ѭ���������ô˹���
			break;
		case KEY2_VALUE:
			LCD_Clear(BLUE);
			BalanceRecharge(); // ����ֵ
			keyFun = KEY_NULL;
			break;
		}
	}
}

/**
 * @brief ��ʾLCD����
 */
void LcdDesktop(void)
{
	// ϵͳ��Ϣ
	LCD_DrawRectangle(20, 20, 220, 116); // ����һ������ο�
	LCD_DrawRectangle(22, 22, 218, 114); // ����һ���ھ��ο�
	LCD_ShowString(30, 30, 200, 16, 16, "BusChargeSystem(BCS)  ");
	LCD_ShowString(30, 50, 200, 16, 16, "   @Version: 2.0      ");
	LCD_ShowString(30, 70, 200, 16, 16, "   @Author: DXR       ");
	LCD_ShowString(30, 90, 200, 16, 16, "   @Date: 2021/12/1   ");
	// ������Ϣ
	LCD_ShowString(20, 130, 200, 16, 16, "=========================");
	LCD_ShowString(20, 150, 200, 16, 16, "*       <Desktop>       *");
	LCD_ShowString(20, 170, 200, 16, 16, "=========================");
	LCD_ShowString(20, 190, 200, 16, 16, "*    KEY0 <Running>     *");
	LCD_ShowString(20, 210, 200, 16, 16, "*    KEY1 <Manage User> *");
	LCD_ShowString(20, 230, 200, 16, 16, "*    KEY2 <Recharge>    *");
	LCD_ShowString(20, 250, 200, 16, 16, "=========================");
}

void ConsumeTips()
{
	LCD_DrawRectangle(20, 130, 220, 230); // ����һ������ο�
	LCD_DrawRectangle(22, 132, 218, 228); // ����һ���ھ��ο�
	LCD_ShowString(30, 150, 200, 16, 16, "    ====(((())))====   ");
}

/**
 * @brief BusChargeSystem(BCS)����
 */
void BCSRunning(void)
{
	u8 status;
	u8 tempCardID[ID_LEN] = {0x0};
	u8 consume = 0;
	LCD_Clear(BLUE);
	while (1)
	{
		ConsumeTips();
		status = ReadCard();
		if (status == MI_OK)
		{
			balance = GetBalance(); // ��ȡ��ǰ�û������
			/* �����һ��ˢ����ID����һ��ˢ����ID����ͬ ��ô����Ϊ���µ��û��������� */
			if (IsEquals(cardID, tempCardID) == FALSE)
			{
				if (consume > 0)
				{
					/* ����������дһ���������Ѽ�¼��������  */
				}
				consume = 0; // ������ѽ������
			}
			/* ���������0 �ſ������� */
			if (balance > 0)
			{
				LCD_ShowNum(75, 180, ++consume, 3, 24); // ��ʾ���ѽ��
				LCD_ShowString(115, 180, 200, 16, 24, "RMB");
				/* �����ɹ���ʾ */
				OK_BEEP();
				OK_LED();
				BalancdInc(-1); // ���-1
				LCD_Clear(BLUE);
				memcpy(tempCardID, cardID, ID_LEN); // ��������ѵ�ID���Ƶ���ʱ�洢��
			}
			else /* ���� */
			{
				LCD_ShowString(40, 180, 200, 16, 16, "Error:Balance is null!");
				/* ����������ʾ */
				ERR_BEEP();
				ERR_LED();
				LCD_Clear(BLUE);
			}
		}
		/* �û���ǿ���˳�����ģʽ */
		if (keyFun == KEYUP_VALUE)
		{
			LCD_Clear(BLUE);
			LCD_ShowString(40, 110, 200, 16, 16, "Tips: Exiting...");
			// //ERR_BEEP();
			delay_ms(1000);
			LCD_Clear(BLUE);
			break;
		}
		// LCD_Clear(BLUE);
	}
}

/**
 * @brief ��ȡ�û����
 * @return u8 ���ص�ǰID��Ӧ���û����
 */
u16 GetBalance()
{
	u16 bal = 0;
	u16 i;
	u8 buf[DATA_SIZE];
	sprintf((char *)buf, "[$%s]", cardID);
	printf(buf);	// ���ʹ������� [$ID]
	delay_ms(1000); // ��ʱ��Ϊ���ȶ�ͨ��
	while (USART_RX_STA & 0x8000 == 0)
	{
		/* �ȴ����ݽ������ */
	}
	USART_RX_STA = 0;
	if (USART_RX_BUF[1] == '$')
	{
		/* ���û�����ȡ���� */
		for (i = 2; USART_RX_BUF[i] != '.'; i++)
		{
			bal = bal * 10 + USART_RX_BUF[i] - 48;
		}
		return bal;
	}
	LCD_Clear(BLUE);
	LCD_ShowString(30, 110, 220, 16, 16, "Error:Not fuond user! ");
	ERR_BEEP();
	delay_ms(1000);
	LCD_Clear(BLUE);
	return 0;
}

/**
 * @brief ���ӻ���ٵ�ǰ�û����
 * @param bal �ɽ�����ֵ�͸�ֵ ��ֵΪ��ֵ ��ֵΪ�۷�
 * @return �ɹ�����TRUE ����FALSE
 */
Boolean BalancdInc(int bal)
{
	u8 sendBuf[DATA_SIZE];

	sprintf((char *)sendBuf, "[=%s,%d]", cardID, bal);
	printf(sendBuf); // ���ʹ������� [=ID,���]
	delay_ms(1000);	 // ��ʱ��Ϊ���ȶ�ͨ��
	while (USART_RX_STA & 0x8000 == 0)
	{
		/* �ȴ����ݽ������ */
	}
	USART_RX_STA = 0;
	if (USART_RX_BUF[1] == '=')
	{
		return TRUE;
	}
	return FALSE;
}

/**
 * @brief ��ʾ��ֵ�˵�
 */
void RechargeMenu(void)
{
	LCD_ShowString(20, 130, 200, 16, 16, "=========================");
	LCD_ShowString(20, 150, 200, 16, 16, "*       <Recharge>      *");
	LCD_ShowString(20, 170, 200, 16, 16, "=========================");
	LCD_ShowString(20, 190, 200, 16, 16, "*      KEY1: 100 RMB    *");
	LCD_ShowString(20, 210, 200, 16, 16, "*      KEY2: 200 RMB    *");
	LCD_ShowString(20, 230, 200, 16, 16, "*      KEYUP: Exit      *");
	LCD_ShowString(20, 250, 200, 16, 16, "=========================");
}

/**
 * @brief ����ֵ
 */
void BalanceRecharge(void)
{
	u8 status;
	keyFun = KEY_NULL; // �ȳ�ʼ��û���κΰ������µ�״̬
	LCD_Clear(BLUE);
	while (1)
	{
		RechargeMenu(); // ��ʾ��ֵ�˵�
		/* KEY_UP ���� ���˳� */
		if (KEYUP_VALUE == keyFun)
		{
			LCD_Clear(BLUE);
			return;
		}

		if (keyFun == KEY1_VALUE || keyFun == KEY2_VALUE)
		{
			LCD_Clear(BLUE);
			ReadCardTips();		 // ��ʾ������Ϣ
			status = ReadCard(); // ����
			if (status == MI_OK)
			{
				/* ��ֵ 100 �� 200 RMB */
				if (BalancdInc(keyFun * 100) == TRUE)
				{
					LCD_Clear(BLUE);
					LCD_ShowString(30, 110, 220, 16, 16, "OK:Recharge succeed!");
					LCD_ShowNum(75, 180, keyFun * 100, 3, 24);
					OK_BEEP();
					LCD_ShowString(115, 180, 200, 16, 24, "RMB");
					delay_ms(2000);
					LCD_Clear(BLUE);
					keyFun = KEY_NULL;
				}
				else
				{
					LCD_Clear(BLUE);
					LCD_ShowString(30, 110, 220, 16, 16, "Error:Not fuond user! ");
					ERR_BEEP();
					delay_ms(1000);
					LCD_Clear(BLUE);
					keyFun = KEY_NULL;
				}
			}
			else
			{
				keyFun = KEY_NULL; // �˳�
			}
		}
	}
}

/**
 * @brief �����û�����ID
 * @param cardID[u8*] �û���ID
 * @return Boolean ���ڷ���TRUE ���򷵻�FALSE
 */
Boolean SearchID(u8 *cardID)
{
	u8 buf[DATA_SIZE];
	sprintf((char *)buf, "[*%s]", cardID);
	printf(buf);	// ���ʹ������� [*ID]
	delay_ms(1000); // ��ʱ��Ϊ���ȶ�ͨ��
	while (USART_RX_STA & 0x8000 == 0)
	{
		/* �ȴ����ݽ������ */
	}
	if (USART_RX_BUF[1] == '*')
	{
		OK_LED();
		return TRUE;
	}
	ERR_LED();
	return FALSE;
}

/**
 * @brief �Ƚ����������Ƿ����
 * @param arr1[u8*]
 * @param arr2[u8*]
 * @return Boolean ��ȷ���TRUE ���򷵻�FALSE
 */
Boolean IsEquals(u8 *arr1, u8 *arr2)
{
	u8 i;
	u8 ans = 0;
	u8 len = strlen((char *)arr1);
	if (len != strlen((char *)arr2))
	{
		return FALSE;
	}
	/* ͨ���������У��ÿһλ */
	for (i = 0; i < len; i++)
	{
		ans |= arr1[i] ^ arr2[i];
	}
	/* �������ans����0 ��˵������������ȷ���TRUE ������ȷ���FALSE */
	return ans == 0 ? TRUE : FALSE;
}

/**
 * @brief ������ʾ��Ϣ
 */
void ReadCardTips(void)
{
	// LCD_Clear(BLUE);
	LCD_ShowString(20, 130, 200, 16, 16, "=========================");
	LCD_ShowString(20, 150, 200, 16, 16, "*    ====(((())))====   *");
	LCD_ShowString(20, 170, 200, 16, 16, "*        Reading...     *");
	LCD_ShowString(20, 190, 200, 16, 16, "*       KEYUP:Exit      *");
	LCD_ShowString(20, 210, 200, 16, 16, "=========================");
}

/**
 * @brief RFID����
 * @return char ����״̬
 */
char ReadCard(void)
{
	char status;
	u8 cardIDTemp[4];
	u8 cardType[4];
	// ReadCardTips(); // ������ʾ��Ϣ
	while (1)
	{
		status = PcdRequest(0x52, cardType); // ��λӦ��
		status = PcdAnticoll(cardIDTemp);	 /*����ײ*/
		status = PcdSelect(cardIDTemp);		 //ѡ��
		if (status == MI_OK)
		{
			break;
		}
		/* ��ǿ���˳� */
		if (keyFun == KEYUP_VALUE)
		{
			// LCD_Clear(BLUE);
			return MI_ERR;
		}
	}
	/* �Ѷ�����ID����תΪһ��ʮ��������ɵ��ַ�������cardID */
	sprintf((char *)cardID, "%d%d%d%d", cardIDTemp[0], cardIDTemp[1], cardIDTemp[2], cardIDTemp[3]);
	return MI_OK;
}

/**
 * @brief �����û��˵�
 */
void ManageUserMenu(void)
{
	LCD_ShowString(20, 130, 200, 16, 16, "=========================");
	LCD_ShowString(20, 150, 200, 16, 16, "*      <Manage User>    *");
	LCD_ShowString(20, 170, 200, 16, 16, "=========================");
	LCD_ShowString(20, 190, 200, 16, 16, "*    KEY0: Add User     *");
	LCD_ShowString(20, 210, 200, 16, 16, "*    KEY1: Delete User  *");
	LCD_ShowString(20, 230, 200, 16, 16, "*    KEYUP: Exit        *");
	LCD_ShowString(20, 250, 200, 16, 16, "=========================");
}

/**
 * @brief �����û�
 */
void ManageUser(void)
{
	u8 status;

	keyFun = KEY_NULL;

	while (1)
	{
		ManageUserMenu();
		/* KEY_UP ���� ���˳� */
		if (KEYUP_VALUE == keyFun)
		{
			// PcdHalt();
			LCD_Clear(BLUE);
			return;
		}
		/* ����û� -> �û�ע�� */
		if (keyFun == KEY0_VALUE)
		{
			UserSignup();
			keyFun = KEY_NULL;
		}
		/* ɾ���û� */
		if (keyFun == KEY1_VALUE)
		{
			LCD_Clear(BLUE);
			LCD_ShowString(20, 110, 200, 16, 16, "*      <Admin Login>    *");
			ReadCardTips();
			status = ReadCard(); // ����
			if (status == MI_OK)
			{
				/* ��֤����Ա */
				if (IsEquals(cardID, adminID) == TRUE)
				{
					OK_BEEP();
					DeleteUser();
					keyFun = KEY_NULL;
				}
				else
				{
					LCD_Clear(BLUE);
					LCD_ShowString(30, 110, 220, 16, 16, "Error:Non-Administrator!");
					ERR_BEEP();
					delay_ms(1000);
					LCD_Clear(BLUE);
					keyFun = KEY_NULL;
				}
			}
			else
			{
				keyFun = KEY_NULL;
			}
		}
	}
}

/**
 * @brief ��ʾ����û��˵�
 */
void AddUserMenu(void)
{
	LCD_ShowString(20, 130, 200, 16, 16, "=========================");
	LCD_ShowString(20, 150, 200, 16, 16, "*       <Add User>      *");
	LCD_ShowString(20, 170, 200, 16, 16, "=========================");
	LCD_ShowString(20, 190, 200, 16, 16, "*    KEY0: Zhou Mingfa  *");
	LCD_ShowString(20, 210, 200, 16, 16, "*    KEY1: Xu Mengyan   *");
	LCD_ShowString(20, 230, 200, 16, 16, "*    KEY2: Fu Yiqing    *");
	LCD_ShowString(20, 250, 200, 16, 16, "*    KEYUP: Exit        *");
	LCD_ShowString(20, 270, 200, 16, 16, "=========================");
}

/**
 * @brief ע��
 */
void UserSignup(void)
{
	char status;
	u8 name[10] = "ZMFXMYFYQ";
	keyFun = KEY_NULL; // �ȳ�ʼ��û���κΰ������µ�״̬
	while (1)
	{
		AddUserMenu();
		ShowTime(DHM); // ��ʾʱ��

		/* KEY_UP ���� ���˳� */
		if (KEYUP_VALUE == keyFun)
		{
			// PcdHalt();
			LCD_Clear(BLUE);
			return;
		}

		/* ���ڰ������� �� ���µİ�������KEYUP */
		if ((KEY_NULL != keyFun) && (keyFun != KEYUP_VALUE))
		{
			LCD_Clear(BLUE);
			ReadCardTips();
			status = ReadCard(); // ����
			if (status == MI_OK)
			{
				/* �����û��� �� ע��Ĭ�ϳ�ֵ10RMB */
				snprintf((char *)userName, 4, "%s", &name[keyFun * 3]);
				balance = 10;
				AddUser(cardID);
				keyFun = KEY_NULL; // ִ��һ��ע�Ṧ�� �������ÿ�
			}
			else
			{
				keyFun = KEY_NULL;
			}
		}
	}
}

/**
 * @brief ͨ��������û���ID����û�
 * @param cardID
 */
void AddUser(u8 *cardID)
{
	/* ��ӳɹ� */
	if (AddData(cardID) == TRUE)
	{
		LCD_Clear(BLUE);
		LCD_ShowString(20, 110, 200, 16, 16, "OK:Signup succeed!");
		LCD_ShowString(20, 140, 200, 16, 16, "Name:");
		LCD_ShowString(80, 135, 200, 16, 24, userName); // ��ʾ�û���
		LCD_ShowString(20, 170, 200, 16, 16, "Balance:");
		LCD_ShowNum(80, 165, balance, 3, 24); // ��ʾ���
		LCD_ShowString(120, 165, 200, 16, 24, "RMB");

		OK_BEEP();
		delay_ms(3000);
		LCD_Clear(BLUE);
	}
	else /* ���ʧ�� */
	{
		LCD_Clear(BLUE);
		LCD_ShowString(20, 110, 200, 16, 16, "Error:Signup fail!");
		ERR_BEEP();
		delay_ms(1000);
		LCD_Clear(BLUE);
	}
}

/**
 * @brief ����û�����
 * @param cardID[u8*]
 * @return Boolean
 */
Boolean AddData(u8 *cardID)
{
	u8 sendBuf[DATA_SIZE], dataBuf[DATA_SIZE];
	sprintf((char *)dataBuf, "%s,%s,%d", cardID, userName, balance);
	sprintf((char *)sendBuf, "[(%s]", dataBuf);
	printf(sendBuf); // ���ʹ������� [(ID,�û���,���]
	delay_ms(1000);	 // ��ʱ��Ϊ���ȶ�ͨ��
	while (USART_RX_STA & 0x8000 == 0)
	{
		/* �ȴ����ݽ������ */
	}
	USART_RX_STA = 0;
	if (USART_RX_BUF[1] == '(')
	{
		return TRUE;
	}
	return FALSE;
}

/**
 * @brief ��ʾɾ���û��˵�
 *
 */
void DelUserMenu(void)
{
	LCD_ShowString(20, 130, 200, 16, 16, "=========================");
	LCD_ShowString(20, 150, 200, 16, 16, "*    <Please Select>    *");
	LCD_ShowString(20, 170, 200, 16, 16, "=========================");
	LCD_ShowString(20, 190, 200, 16, 16, "* KEY1: Delete a user   *");
	LCD_ShowString(20, 210, 200, 16, 16, "* KEYUP: Exit           *");
	LCD_ShowString(20, 230, 200, 16, 16, "=========================");
}

/**
 * @brief ɾ���û�
 */
void DeleteUser(void)
{
	char status;
	keyFun = KEY_NULL;
	LCD_Clear(BLUE);
	while (1)
	{
		DelUserMenu();
		if (KEYUP_VALUE == keyFun)
		{
			LCD_Clear(BLUE);
			return;
		}

		/* ��KEY1ɾ��ָ���û���ID ��ˢָ���Ŀ�����ɾ������ */
		while (keyFun == KEY1_VALUE)
		{
			LCD_Clear(BLUE);
			ReadCardTips();
			status = ReadCard(); // ˢ��
			if (status == MI_OK)
			{
				RemoveUser(cardID);
				keyFun = KEY_NULL;
			}
			else /* ����ʧ�� */
			{
				keyFun = KEY_NULL; // ǿ���˳�
			}
		}
	}
}

/**
 * @brief ͨ��������û���IDɾ��ָ���û�
 * @param cardID
 */
void RemoveUser(u8 *cardID)
{
	/* ɾ���ɹ� */
	if (RemoveData(cardID) == TRUE)
	{
		LCD_Clear(BLUE);
		LCD_ShowString(10, 110, 220, 16, 16, "OK:Delete user succeed!");
		OK_BEEP();
		delay_ms(1000);
		LCD_Clear(BLUE);
	}
	else /* �޴��û� ɾ��ʧ�� */
	{
		LCD_Clear(BLUE);
		LCD_ShowString(10, 110, 220, 16, 16, "Error:Not fuond user! ");
		ERR_BEEP();
		delay_ms(1000);
		LCD_Clear(BLUE);
	}
}

/**
 * @brief ɾ��ָ���û�����
 * @param cardID[u8*]
 * @return Boolean
 */
Boolean RemoveData(u8 *cardID)
{
	u8 sendBuf[DATA_SIZE];
	sprintf((char *)sendBuf, "[)%s]", cardID);
	printf(sendBuf); // ���ʹ������� [)ID]
	delay_ms(1000);	 // ��ʱ��Ϊ���ȶ�ͨ��
	while (USART_RX_STA & 0x8000 == 0)
	{
		/* �ȴ����ݽ������ */
	}
	USART_RX_STA = 0;
	if (USART_RX_BUF[1] == ')')
	{
		return TRUE;
	}
	return FALSE;
}

/**
 * @brief ���ظ���ģʽ��ʱ�� �� ��ʾʱ��
 * @param timeMode[u8] ѡ���ģʽ
 * @return u8 D������ H����ʱ��M���ط�  DHM��ʾʱ�䣬�޷���
 */
u8 ShowTime(u8 timeMode)
{
	RTC_TimeTypeDef TheTime; //ʱ��
	RTC_DateTypeDef TheData; //����
	u8 *TimeBuff;			 //ʱ��洢

	RTC_GetTime(RTC_Format_BIN, &TheTime);
	RTC_GetDate(RTC_Format_BIN, &TheData);
	switch (timeMode)
	{
	case D:
		return TheData.RTC_Date;
	case H:
		return TheTime.RTC_Hours;
	case M:
		return TheTime.RTC_Minutes;
	case DHM:
		TimeBuff = malloc(40);
		sprintf((char *)TimeBuff, "%02d%s:%02d%s:%02d%s", TheData.RTC_Date, "D", TheTime.RTC_Hours, "H", TheTime.RTC_Minutes, "M");
		LCD_ShowString(130, 300, 200, 16, 16, TimeBuff);
		free(TimeBuff);
		break;
	}
	return 0;
}

/**
 * @brief BusChargeSystem(BCS)��ʼ��
 */
void BCSInit(void)
{
	LCD_Clear(BLUE);   // ����Ϊ��ɫ ��������ɫ
	BACK_COLOR = BLUE; // ���ñ�����ɫΪ��ɫ
	POINT_COLOR = RED; // ��������Ϊ��ɫ
	keyFun = KEY_NULL; // �ȳ�ʼ��û���κΰ������µ�״̬
}
