////////////////////////////////////////////////////////////////////////////////
//  �� �� ��   : sys.c
//  �� �� ��   : V1.0
//  ��    ��   : TinderSmith
//  ��������   : 2014-01-01
//  ����޸�   : 2015-06-27
//  ��������   : <MP3><stm32 vs1053 oled tfcard(spi) fat0.11>
//  ��Ȩ����   : �θ���
//	˵    ��   :
//              ----------------------------------------------------------------
//              GND  �ӵ�Դ��
//              VCC  ��5V��3.3v��Դ
//              CS   ��PE7
//              RES  ��PE8
//              DC   ��PE9
//              D0   ��PE10(SCL)
//              D1   ��PE11(SDA)
//              ----------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

#include "sys.h"

/*********************************************************************************
* ��    ��: NVIC_Configuration
* ��    ��: ����ϵͳǶ�������жϿ�����
* ��ڲ���: 
* ���ڲ���: 
* ˵    ��: 
*********************************************************************************/
void NVIC_Configuration(void)
{
	  NVIC_InitTypeDef NVIC_InitStructure;
		/* �����������λ�ú�ƫ�� */
#ifdef  VECT_TAB_RAM  
		/* �����������׼λ���� 0x20000000 */ 
		NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else  /* VECT_TAB_FLASH  */
		/* �����������׼λ���� 0x08000000 */ 
		NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
#endif
		/* ����NVIC���ȼ����飺��ռ���ȼ�0λ,�����ȼ�4λ */  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);	
		/* �� TIM2 �ж� */
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* ��    ��: RCC_Configuration
* ��    ��: ���ò�ͬ��ϵͳʱ��
* ��ڲ���: 
* ���ڲ���: 
* ˵		��: ϵͳ��ʱ����RCC_CFGR�Ĵ����е�SW��ѡ��3����Դ�е�һ��
* 						1.�ڲ�ʱ��HSI			2.�ⲿʱ��HSE			 3.PLL���໷
* 					���õ�ʱ������
* 						SYSCLK(ϵͳʱ��)	 = 72MHz		
* 						AHB����ʱ��			 = 72MHz
* 						APB1����ʱ�� 		 = 36MHz
* 						APB2����ʱ��	   	 = 72MHz
* 						PLLʱ��   			 = 72MHz
* 						PLL2ʱ��				 = 40MHz
*******************************************************************************/
void RCC_Configuration(void)
{
	ErrorStatus HSEStartUpStatus;													//����ö�����ͱ��� HSEStartUpStatus 
  RCC_DeInit();																          //��λϵͳʱ������(Ϊ��Debug)
  RCC_HSEConfig(RCC_HSE_ON);									          //����HSE
  HSEStartUpStatus = RCC_WaitForHSEStartUp();           //�ȴ��ⲿ���پ���HSE�����ȶ�

  if(HSEStartUpStatus == SUCCESS)												//�ж�HSE�Ƿ�����ɹ�,�������if��
	 {
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);//ʹ��FLASHԤȡ����
    FLASH_SetLatency(FLASH_Latency_2);									 //����FLASH��ʱ������Ϊ2
    RCC_HCLKConfig(RCC_SYSCLK_Div1);										 //ѡ��HCLK(AHB)ʱ��ԴΪSYSCLK 1��Ƶ
		RCC_PCLK1Config(RCC_HCLK_Div2);											 //����APB1ʱ�� PCLK1 	= HCLK(AHB)/2
    RCC_PCLK2Config(RCC_HCLK_Div1);											 //����APB2ʱ�� PCLK2 	= HCLK(AHB)
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); //ѡ�����໷��PLL��ʱ��ԴΪHSE 1��Ƶ��������PLLCLK 	= 8MHz * 9 = 72 MHz
    RCC_PLLCmd(ENABLE);																	 //PLLʹ��
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);	 //�ȴ����໷ʱ��(PLL)����ȶ�
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);					 //ѡ��SYSCLKʱ��ԴΪPLL
    while(RCC_GetSYSCLKSource() != 0x08);								 //�ȴ�PLL��ΪSYSCLKʱ��Դ
   }
	/* ��SPI1,GPIOA.B.C.D �� USART1 ��ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1  | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
												 RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
												 RCC_APB2Periph_GPIOG | RCC_APB2Periph_USART1, ENABLE);
	/* ��TIM2,TIM3,SPI2	��ʱ�� */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2  | RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE);
}

/*******************************************************************************
* ��    ��: TIM_Configuration
* ��    ��: ���ö�ʱ�ж���
* ��ڲ���: 
* ���ڲ���: 
* ˵		��: 
*******************************************************************************/
void TIM_Configuration(void)
{ 
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStruct;
  /* ���ü��������С��ÿ��96�����Ͳ���һ�������¼� */
  TIM_TimeBaseStructure.TIM_Period = 0x0060;
	/* Ԥ��Ƶϵ��Ϊ60000-1������������ʱ��Ϊ72MHz/60000 = 1.2kHz */
  TIM_TimeBaseStructure.TIM_Prescaler = 0xEA5F;
	/* ����ʱ�ӷָ� */
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	/* ���ü�����ģʽΪ���ϼ���ģʽ */
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	/* ������Ӧ�õ�TIM2�� */
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  /* ����Ƚϼ�ʱ��ģʽ����:OC1 */
  TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_Timing;
  TIM_OCInitStruct.TIM_Pulse = 0x0;
	/* ���½�Timer����Ϊȱʡֵ */
  TIM_OC1Init(TIM2, &TIM_OCInitStruct);//TIM2��ͨ��OC1ʹ��,ST��3.5��仯���д��
  /* ʹ��TIM2������ */
  TIM_Cmd(TIM2, ENABLE);
  /* ��������TIM2 Ԥ��Ƶֵ */
  TIM_PrescalerConfig(TIM2, 0xEA5F, TIM_PSCReloadMode_Immediate);
  /* �������жϱ�־ */
  TIM_ClearFlag(TIM2, TIM_FLAG_Update);
  /* ����TIM2���ж� */
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	
	
	/* ���ü��������С��ÿ��96�����Ͳ���һ�������¼� */
  TIM_TimeBaseStructure.TIM_Period = 0x0258;
	/* Ԥ��Ƶϵ��Ϊ60000-1������������ʱ��Ϊ72MHz/60000 = 1.2kHz */
  TIM_TimeBaseStructure.TIM_Prescaler = 0xEA5F;
	/* ����ʱ�ӷָ� */
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	/* ���ü�����ģʽΪ���ϼ���ģʽ */
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	/* ������Ӧ�õ�TIM2�� */
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  /* ����Ƚϼ�ʱ��ģʽ����:OC1 */
  TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_Timing;
  TIM_OCInitStruct.TIM_Pulse = 0x0;
	/* ���½�Timer����Ϊȱʡֵ */
  TIM_OC2Init(TIM3, &TIM_OCInitStruct);//TIM3��ͨ��OC2ʹ��,ST��3.5��仯���д��
  /* ʹ��TIM2������ */
  TIM_Cmd(TIM3, ENABLE);
  /* ��������TIM2 Ԥ��Ƶֵ */
  TIM_PrescalerConfig(TIM3, 0xEA5F, TIM_PSCReloadMode_Immediate);
  /* �������жϱ�־ */
  TIM_ClearFlag(TIM3, TIM_FLAG_Update);
  /* ����TIM2���ж� */
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
}

/*
void MUC_SLEEP(void)
{
GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);  //��ȫʧ��JTAG/SWD ���ص���Ҳ����Ӱ��
GPIO_PinRemapConfig( GPIO_Remap_PD01 , ENABLE );//����ΪGPIO�����ص���Ҳ����Ӱ��
RCC_LSEConfig(RCC_LSE_OFF);//�ر�RTC�� ���ص���Ҳ����Ҳ��Ӱ��
RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
ADC_CONFIG(DISABLE);
USART_DeInit(USART1);
USART_DeInit(USART2);
USART_DeInit(USART3);
USART_DeInit(UART5);
EXTI_INITIAL(ENABLE);//���¶˿ڵ���������Ӱ��
GPIO_PIN_INITIAL(GPIOA,GPIO_Pin_All,GPIO_Mode_AIN,0); //�˴�û���ⲿ���ѡ������ڲ��ԡ�EXTI��������GPIO��Ӱ�����
GPIO_PIN_INITIAL(GPIOB,GPIO_Pin_All,GPIO_Mode_AIN,0); 
GPIO_PIN_INITIAL(GPIOC,GPIO_Pin_All,GPIO_Mode_AIN,0); 
GPIO_PIN_INITIAL(GPIOD,GPIO_Pin_All,GPIO_Mode_AIN,0);
//PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);//STOPģʽ
PWR_EnterSTANDBYMode();//standbyģʽ 
}
*/
