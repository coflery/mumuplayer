////////////////////////////////////////////////////////////////////////////////
//  文 件 名   : sys.c
//  版 本 号   : V1.0
//  作    者   : TinderSmith
//  生成日期   : 2014-01-01
//  最近修改   : 2015-06-27
//  功能描述   : <MP3><stm32 vs1053 oled tfcard(spi) fat0.11>
//  版权所有   : 任复衡
//	说    明   :
//              ----------------------------------------------------------------
//              GND  接电源地
//              VCC  接5V或3.3v电源
//              CS   接PE7
//              RES  接PE8
//              DC   接PE9
//              D0   接PE10(SCL)
//              D1   接PE11(SDA)
//              ----------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

#include "sys.h"

/*********************************************************************************
* 名    称: NVIC_Configuration
* 功    能: 配置系统嵌套向量中断控制器
* 入口参数: 
* 出口参数: 
* 说    明: 
*********************************************************************************/
void NVIC_Configuration(void)
{
	  NVIC_InitTypeDef NVIC_InitStructure;
		/* 设置向量表的位置和偏移 */
#ifdef  VECT_TAB_RAM  
		/* 设置向量表基准位置在 0x20000000 */ 
		NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else  /* VECT_TAB_FLASH  */
		/* 设置向量表基准位置在 0x08000000 */ 
		NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
#endif
		/* 设置NVIC优先级分组：先占优先级0位,从优先级4位 */  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);	
		/* 打开 TIM2 中断 */
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
* 名    称: RCC_Configuration
* 功    能: 配置不同的系统时钟
* 入口参数: 
* 出口参数: 
* 说		明: 系统的时钟由RCC_CFGR寄存器中的SW来选择3个来源中的一个
* 						1.内部时钟HSI			2.外部时钟HSE			 3.PLL锁相环
* 					配置的时钟如下
* 						SYSCLK(系统时钟)	 = 72MHz		
* 						AHB总线时钟			 = 72MHz
* 						APB1总线时钟 		 = 36MHz
* 						APB2总线时钟	   	 = 72MHz
* 						PLL时钟   			 = 72MHz
* 						PLL2时钟				 = 40MHz
*******************************************************************************/
void RCC_Configuration(void)
{
	ErrorStatus HSEStartUpStatus;													//定义枚举类型变量 HSEStartUpStatus 
  RCC_DeInit();																          //复位系统时钟设置(为了Debug)
  RCC_HSEConfig(RCC_HSE_ON);									          //开启HSE
  HSEStartUpStatus = RCC_WaitForHSEStartUp();           //等待外部高速晶振HSE起振并稳定

  if(HSEStartUpStatus == SUCCESS)												//判断HSE是否起振成功,是则进入if内
	 {
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);//使能FLASH预取缓存
    FLASH_SetLatency(FLASH_Latency_2);									 //设置FLASH延时周期数为2
    RCC_HCLKConfig(RCC_SYSCLK_Div1);										 //选择HCLK(AHB)时钟源为SYSCLK 1分频
		RCC_PCLK1Config(RCC_HCLK_Div2);											 //设置APB1时钟 PCLK1 	= HCLK(AHB)/2
    RCC_PCLK2Config(RCC_HCLK_Div1);											 //设置APB2时钟 PCLK2 	= HCLK(AHB)
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); //选择锁相环（PLL）时钟源为HSE 1分频，并配置PLLCLK 	= 8MHz * 9 = 72 MHz
    RCC_PLLCmd(ENABLE);																	 //PLL使能
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);	 //等待锁相环时钟(PLL)输出稳定
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);					 //选择SYSCLK时钟源为PLL
    while(RCC_GetSYSCLKSource() != 0x08);								 //等待PLL成为SYSCLK时钟源
   }
	/* 打开SPI1,GPIOA.B.C.D 和 USART1 的时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1  | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
												 RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
												 RCC_APB2Periph_GPIOG | RCC_APB2Periph_USART1, ENABLE);
	/* 打开TIM2,TIM3,SPI2	的时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2  | RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE);
}

/*******************************************************************************
* 名    称: TIM_Configuration
* 功    能: 配置定时中断器
* 入口参数: 
* 出口参数: 
* 说		明: 
*******************************************************************************/
void TIM_Configuration(void)
{ 
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStruct;
  /* 设置计数溢出大小，每计96个数就产生一个更新事件 */
  TIM_TimeBaseStructure.TIM_Period = 0x0060;
	/* 预分频系数为60000-1，这样计数器时钟为72MHz/60000 = 1.2kHz */
  TIM_TimeBaseStructure.TIM_Prescaler = 0xEA5F;
	/* 设置时钟分割 */
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	/* 设置计数器模式为向上计数模式 */
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	/* 将配置应用到TIM2中 */
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  /* 输出比较计时器模式配置:OC1 */
  TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_Timing;
  TIM_OCInitStruct.TIM_Pulse = 0x0;
	/* 重新将Timer设置为缺省值 */
  TIM_OC1Init(TIM2, &TIM_OCInitStruct);//TIM2在通道OC1使能,ST的3.5库变化后的写法
  /* 使能TIM2计数器 */
  TIM_Cmd(TIM2, ENABLE);
  /* 立即加载TIM2 预分频值 */
  TIM_PrescalerConfig(TIM2, 0xEA5F, TIM_PSCReloadMode_Immediate);
  /* 清除溢出中断标志 */
  TIM_ClearFlag(TIM2, TIM_FLAG_Update);
  /* 开启TIM2的中断 */
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	
	
	/* 设置计数溢出大小，每计96个数就产生一个更新事件 */
  TIM_TimeBaseStructure.TIM_Period = 0x0258;
	/* 预分频系数为60000-1，这样计数器时钟为72MHz/60000 = 1.2kHz */
  TIM_TimeBaseStructure.TIM_Prescaler = 0xEA5F;
	/* 设置时钟分割 */
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	/* 设置计数器模式为向上计数模式 */
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	/* 将配置应用到TIM2中 */
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  /* 输出比较计时器模式配置:OC1 */
  TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_Timing;
  TIM_OCInitStruct.TIM_Pulse = 0x0;
	/* 重新将Timer设置为缺省值 */
  TIM_OC2Init(TIM3, &TIM_OCInitStruct);//TIM3在通道OC2使能,ST的3.5库变化后的写法
  /* 使能TIM2计数器 */
  TIM_Cmd(TIM3, ENABLE);
  /* 立即加载TIM2 预分频值 */
  TIM_PrescalerConfig(TIM3, 0xEA5F, TIM_PSCReloadMode_Immediate);
  /* 清除溢出中断标志 */
  TIM_ClearFlag(TIM3, TIM_FLAG_Update);
  /* 开启TIM2的中断 */
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
}

/*
void MUC_SLEEP(void)
{
GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);  //完全失能JTAG/SWD 不关电流也好像不影响
GPIO_PinRemapConfig( GPIO_Remap_PD01 , ENABLE );//晶振为GPIO　不关电流也好像不影响
RCC_LSEConfig(RCC_LSE_OFF);//关闭RTC　 不关电流也好像也不影响
RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
ADC_CONFIG(DISABLE);
USART_DeInit(USART1);
USART_DeInit(USART2);
USART_DeInit(USART3);
USART_DeInit(UART5);
EXTI_INITIAL(ENABLE);//以下端口的设置最有影响
GPIO_PIN_INITIAL(GPIOA,GPIO_Pin_All,GPIO_Mode_AIN,0); //此处没做外部唤醒　仅用于测试　EXTI合理设置GPIO不影响电流
GPIO_PIN_INITIAL(GPIOB,GPIO_Pin_All,GPIO_Mode_AIN,0); 
GPIO_PIN_INITIAL(GPIOC,GPIO_Pin_All,GPIO_Mode_AIN,0); 
GPIO_PIN_INITIAL(GPIOD,GPIO_Pin_All,GPIO_Mode_AIN,0);
//PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);//STOP模式
PWR_EnterSTANDBYMode();//standby模式 
}
*/
