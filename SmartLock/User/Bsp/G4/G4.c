#include "G4.h"
#include <stdio.h>  
#include <string.h>  
#include <stdbool.h>
#include "board.h"
#include "rtthread.h"
#include "delay.h"


rt_mailbox_t G4_mail = RT_NULL;


struct G4_UART_BUFF G4_GET[3];//��Ž��յ�����
uint8_t G4_GET_POINT=0;//ָ��ǰ��ŵ�����

struct  G4_USARTx_Fram  G4_Fram_Record = { 0 };

extern uint8_t unVanish;


uint32_t StringToInt(char *str)
{
	uint32_t temp=0;
	uint8_t k=0;
	while(*(str+k)!='\0')
	{
		temp=temp*10+*(str+k)-'0';
		k++;
	}
	return temp;
}

char* IntToString(uint32_t integer)
{
	uint32_t temp=1,temp1=integer;
	uint8_t k=1,j=0;
	char *str;
	while((temp*10)<integer)
	{
		temp=temp*10;
		k++;
	}
	str=rt_malloc(sizeof(char)*k);
	for(j=0;j<k;j++)
	{
		*(str+j)=temp1/temp+'0';
		temp1=temp1%temp;
		temp=temp/10;
	}
	*(str+j)='\0';
	return str;
}

void G4_StatePinConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// ��RST��GPIO����Ϊ���츴��ģʽ
	GPIO_InitStructure.GPIO_Pin = G4_RST_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(G4_RST_PORT, &GPIO_InitStructure);
	
	GPIO_SetBits(G4_RST_PORT, G4_RST_PIN);

  // ��LIKA��GPIO����Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = G4_LIKA_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(G4_LIKA_PORT, &GPIO_InitStructure);
	
	// ��STAT��GPIO����Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = G4_STAT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(G4_STAT_PORT, &GPIO_InitStructure);
}

/*�������״̬*/
bool G4_CheckSTAT(void)
{
	uint8_t temp;
	temp=GPIO_ReadInputDataBit(G4_STAT_PORT, G4_STAT_PIN);
	if(temp==0)
	{
		return false;
	}
	return true;
}

/*������������״̬*/
bool G4_CheckLIKA(void)
{
	uint8_t temp;
	temp=GPIO_ReadInputDataBit(G4_LIKA_PORT, G4_LIKA_PIN);
	if(temp==0)
	{ 
		return false;
	}	
	return true;
}

/*��λģ��*/
void G4_RST(void)
{
	uint16_t temp;
	GPIO_ResetBits(G4_RST_PORT, G4_RST_PIN);
	for(temp=0;temp<1000;temp++);
	GPIO_SetBits(G4_RST_PORT, G4_RST_PIN);

}

 /**
  * @brief  ����Ƕ�������жϿ�����NVIC
  * @param  ��
  * @retval ��
  */
static void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Ƕ�������жϿ�������ѡ�� */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* ����USARTΪ�ж�Դ */
  NVIC_InitStructure.NVIC_IRQChannel = G4_USART_IRQ;
  /* �������ȼ�*/
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  /* �����ȼ� */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  /* ʹ���ж� */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  /* ��ʼ������NVIC */
  NVIC_Init(&NVIC_InitStructure);
}

 /**
  * @brief  USART GPIO ����,������������
  * @param  ��
  * @retval ��
  */
void G4_USART_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// �򿪴���GPIO��ʱ��
	G4_USART_GPIO_APBxClkCmd(G4_USART_GPIO_CLK, ENABLE);
	
	// �򿪴��������ʱ��
	G4_USART_APBxClkCmd(G4_USART_CLK, ENABLE);

	// ��USART Tx��GPIO����Ϊ���츴��ģʽ
	GPIO_InitStructure.GPIO_Pin = G4_USART_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(G4_USART_TX_GPIO_PORT, &GPIO_InitStructure);

  // ��USART Rx��GPIO����Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = G4_USART_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(G4_USART_RX_GPIO_PORT, &GPIO_InitStructure);
	
	// ���ô��ڵĹ�������
	// ���ò�����
	USART_InitStructure.USART_BaudRate = G4_USART_BAUDRATE;
	// ���� �������ֳ�
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// ����ֹͣλ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// ����У��λ
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	// ����Ӳ��������
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	// ���ù���ģʽ���շ�һ��
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// ��ɴ��ڵĳ�ʼ������
	USART_Init(G4_USARTx, &USART_InitStructure);
	
	// �����ж����ȼ�����
	NVIC_Configuration();
	
	G4_StatePinConfig();
	
	// ʹ�ܴ��ڽ����ж�
	USART_ITConfig(G4_USARTx, USART_IT_RXNE, ENABLE);	
	//ʹ�ܿ���
	USART_ITConfig(G4_USARTx, USART_IT_IDLE, ENABLE);	
	
	// ʹ�ܴ���
	USART_Cmd(G4_USARTx, ENABLE);	    
}

/*
	����һ���ֽ�����
	*/
void G4_SendByte(uint8_t data)
{
	/* ����һ���ֽ����ݵ�USART */
	USART_SendData(G4_USARTx,data);
		
	/* �ȴ��������ݼĴ���Ϊ�� */
	while (USART_GetFlagStatus(G4_USARTx, USART_FLAG_TXE) == RESET);	
}

/*
	����һ���ֽ�������
	*/
void G4_SendArray(uint8_t *array, uint16_t num)
{
  uint16_t i;
	
	for(i=0; i<num; i++)
  {
	    /* ����һ���ֽ����ݵ�USART */
	    G4_SendByte(array[i]);	
  
  }
}

/*
	�����ַ���
	*/
void G4_SendString( char *str)
{
	unsigned int k=0;
	//rt_kprintf("\r\nsending start\r\n");
  while(*(str+k)!='\0') 
  {
      G4_SendByte( *(str + k) );
      k++;
  }
	//rt_kprintf("\r\nsending end\r\n");
}
/*
 * ��������G4_Cmd
 * ����  ����4Gģ�鷢��ATָ��
 * ����  ��cmd�������͵�ָ��
 *         reply1��reply2���ڴ�����Ӧ��ΪNULL��������Ӧ������Ϊ���߼���ϵ
 *         waittime���ȴ���Ӧ��ʱ��
 * ����  : 1��ָ��ͳɹ�
 *         0��ָ���ʧ��
 * ����  �����ⲿ����
 */
bool G4_Cmd ( char * cmd, char * reply1, char * reply2, u32 waittime )
{    
	uint32_t delayTime=0;
	G4_Fram_Record.FramLength=0;               //���¿�ʼ�����µ����ݰ�
	G4_Fram_Record.FramFinishFlag=0;
	bool sure;//����ֵ����ʾָ��ִ�����
	//cmdFlag=1;
	G4_SendString (cmd);
	if ( ( reply1 == 0 ) && ( reply2 == 0 ) )                      //����Ҫ��������
	{
		return true;
	}
	
	//Delay_ms ( waittime );                 //��ʱ
	
	do
	{
		delay_ms(100);						 //��ʱ
		delayTime=delayTime+100;
	}while((delayTime<waittime)&&(G4_Fram_Record.FramFinishFlag==0));
	
	if(G4_Fram_Record.FramFinishFlag==0)
	{
		rt_kprintf("\r\nget nothing\r\n");
		return false;
	}
	G4_Fram_Record .Data_RX_BUF [ G4_Fram_Record .FramLength ]  = '\0';

  rt_kprintf( "%s", G4_Fram_Record .Data_RX_BUF );
	
	if ( ( reply1 != 0 ) && ( reply2 != 0 ) )
		sure = ( ( bool ) strstr ( G4_Fram_Record .Data_RX_BUF, reply1 ) || 
						 ( bool ) strstr ( G4_Fram_Record .Data_RX_BUF, reply2 ) ); 
 	
	else if ( reply1 != 0 )
		sure = ( ( bool ) strstr ( G4_Fram_Record .Data_RX_BUF, reply1 ) );
	
	else
		sure = ( ( bool ) strstr ( G4_Fram_Record .Data_RX_BUF, reply2 ) );
	return sure;
	
}

/*
	����ATģʽ
	*/
bool G4_EnterATMode(void)
{
	bool result=false;
	G4_Cmd ( "+++", NULL, NULL, 0 );
	delay_ms(50);
	G4_Cmd ( "\r\n", NULL, NULL, 0 );
	delay_ms(500);
	result=G4_Cmd ( "AT+VER\r\n", "+OK=", NULL, 5000 );
  rt_kprintf("result=%d\n\n",result);
	return result;
}
/*
	�˳�ATģʽ
	*/
bool G4_ExitATMode(void)
{
	bool result=false;
	result=G4_Cmd ( "AT+EXAT\r\n", "+OK", NULL, 5000 );
	return result;
}
/*
	�ָ�����ģʽ
	*/
bool G4_AT_RESTORE(void)
{
	bool result=false;
	result=G4_Cmd ( "AT+RESTORE\r\n", "+", NULL, 500 );
	return result;
}

/*
	����ģ��
	*/
bool G4_AT_REBT(void)
{
	bool result=false;
	result=G4_Cmd ( "AT+REBT\r", "+OK", NULL, 500 );
	return result;
}

/*
	��ѯ���ڲ���
	*/
bool G4_AT_UART_Query(uint32_t *baudrate,uint8_t *parity)
{
	bool result=false;
	char *point,chTemp[10];
	uint8_t k=0;
	result=G4_Cmd ( "AT+UART\r", "+OK=", NULL, 500 );
	if(!result)
	{
		return false;
	}
	point=G4_Fram_Record.Data_RX_BUF+6;
	while(*(point+k)!=',')
	{
		chTemp[k]=*(point+k);
		k++;
	}
	chTemp[k]='\0';
	*baudrate=StringToInt(chTemp);
	if(strstr(G4_Fram_Record.Data_RX_BUF,"NONE"))
	{
		*parity=0;
	}else if(strstr(G4_Fram_Record.Data_RX_BUF,"EVEN"))
	{
		*parity=2;
	}else
	{
		*parity=1;
	}
	return result;
}
/*
	���ô��ڲ���
	*/
bool G4_AT_UART_Set(uint32_t baudrate,char *parity)
{
	bool result=false;
	char chTemp[50],*point;
	strcpy(chTemp,"AT+UART=");
	point=IntToString(baudrate);
	strcat(chTemp,point);
	strcat(chTemp,",");
	strcat(chTemp,parity);
	strcat(chTemp,"\r\n");
	result=G4_Cmd ( chTemp, "+OK", NULL, 500 );
	rt_free(point);
	if(!result)
	{
		return false;
	}
	return result;
}
/*
	��ѯ����״̬
	*/
bool G4_AT_LINKSTA(uint8_t *sta)
{
	bool result=false;
	result=G4_Cmd ( "AT+LINKSTA\r", "+OK=", NULL, 500 );
	if(!result)
	{
		*sta=0;
		return false;
	}
	if(strstr(G4_Fram_Record.Data_RX_BUF,"Connect"))
	{
		*sta=1;
	}else
	{
		*sta=0;
	}
	return result;
}

/*
	�������ӵ�ַ
	*/
bool G4_AT_SOCK(char *protocol,char *ip,char *port)
{
	bool result=false;
	char ch[50];
	strcpy(ch,"AT+SOCK=");
	strcat(ch,protocol);
	strcat(ch,",");
	strcat(ch,ip);
	strcat(ch,",");
	strcat(ch,port);
	strcat(ch,"\r");
	result=G4_Cmd ( ch, "+OK", NULL, 500 );
	return result;
}

/*
	����������ģʽ
	*/
bool G4_AT_HEARTMOD(char *mode)
{
	bool result=false;
	char ch[50];
	strcpy(ch,"AT+HEARTMOD=");
	strcat(ch,mode);
	strcat(ch,"\r");
	result=G4_Cmd ( ch, "+OK", NULL, 500 );
	return result;
}
/*
	��������������
	*/
bool G4_AT_HEARTINFO(char *data)
{
	bool result=false;
	char ch[50];
	strcpy(ch,"AT+HEARTINFO=");
	strcat(ch,data);
	strcat(ch,"\r");
	result=G4_Cmd ( ch, "+OK", NULL, 500 );
	return result;
}
/*
	����������ʱ��
	*/
bool G4_AT_HEARTM(uint32_t time)
{
	bool result=false;
	char ch[50],*point;
	point=IntToString(time);
	strcpy(ch,"AT+HEARTM=");
	strcat(ch,point);
	strcat(ch,"\r");
	result=G4_Cmd ( ch, "+OK", NULL, 500 );
	rt_free(point);
	return result;
}
/**
   *@Brief �Ƿ������ӷ�����ǰ��������0��1��
   *@RetVal ִ�н���Ƿ�ɹ�
   */
bool G4_AT_UARTCLR(uint8_t flag)
{
	bool result=false;
	char ch[20];
	strcpy(ch,"AT+UARTCLR=");
	if(flag==0)
	{
		strcat(ch,"OFF\r");
	}else
	{
		strcat(ch,"ON\r");
	}
	result=G4_Cmd ( ch, "+OK", NULL, 500 );
	return result;
}


void G4_USART_IRQHandler()
{
	uint8_t chTemp;
	static uint8_t FirstGet=0;
	if ( USART_GetITStatus ( G4_USARTx, USART_IT_RXNE ) != RESET )
	{
		
		chTemp=USART_ReceiveData( G4_USARTx ); 
		
		rt_kprintf("%c",chTemp);
		
		if(G4_Fram_Record.FramLength<( RX_BUF_MAX_LEN - 1 ))
		{
			G4_Fram_Record.Data_RX_BUF[G4_Fram_Record.FramLength]=chTemp;
			G4_Fram_Record.FramLength++;
		}
		
		if(unVanish==1)
		{
			if(FirstGet==0)
			{
				G4_GET[G4_GET_POINT].num=1;
				G4_GET[G4_GET_POINT].data[0]=chTemp;
				FirstGet=1;
				rt_mb_send(G4_mail,(rt_uint32_t)&G4_GET[G4_GET_POINT]);
			}else
			{
				if(G4_GET[G4_GET_POINT].num<1499)
				{
					G4_GET[G4_GET_POINT].data[G4_GET[G4_GET_POINT].num]=chTemp;
					G4_GET[G4_GET_POINT].num++;
				}
				rt_mb_send(G4_mail,(rt_uint32_t)&G4_GET[G4_GET_POINT]);
			}
		}
	}else if ( USART_GetITStatus( G4_USARTx, USART_IT_IDLE ) == SET )
	{
		chTemp=USART_ReceiveData( G4_USARTx ); 
		
		rt_kprintf("%c",chTemp);
		
		if(G4_Fram_Record.FramLength<( RX_BUF_MAX_LEN - 1 ))
		{
			G4_Fram_Record.Data_RX_BUF[G4_Fram_Record.FramLength]=chTemp;
			G4_Fram_Record.FramLength++;
		}
		
		if(unVanish==1)
		{
			if(G4_GET[G4_GET_POINT].num<1499)
			{
				G4_GET[G4_GET_POINT].data[G4_GET[G4_GET_POINT].num]=chTemp;
				G4_GET[G4_GET_POINT].num++;
			}
			G4_GET[G4_GET_POINT].data[G4_GET[G4_GET_POINT].num]='\0';
			rt_mb_send(G4_mail,(rt_uint32_t)&G4_GET[G4_GET_POINT]);
			G4_GET_POINT=(G4_GET_POINT+1)%3;
		}
		FirstGet=0;
	}
}