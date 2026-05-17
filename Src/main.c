/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define MAGIC_KEY_DEFINE 0x12345678
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */
uint16_t periods[3] = {511, 511, 1023};
uint16_t pulses[3] = {255, 255, 511};
uint16_t phases[3] = {0, 0, 0};
uint16_t prescalers[3] = {0, 0, 0};
uint16_t btnCounts[6] = {0, 0, 0, 0, 0, 0};
uint16_t waiting = 0, saveCount = 0;
uint32_t waitingToSave = 0;
uint8_t freqStep = 1, dutyStep = 1, phaseStep = 1;
uint8_t channels[3] = {0, 0, 0};
uint8_t encCounter = 0, encChange = 0, currentChannel = 1, currentOption = 0, PWMCounter = 0, brightness = 10;
float dutys[3] = {0.5, 0.5, 0.5};

const uint16_t WAIT_TICKS = 4000;
const uint32_t WAIT_SAVE_TICKS = 3000000;

typedef struct {
	uint16_t periods[3];
	uint16_t pulses[3];
	uint16_t phases[3];
	uint16_t prescalers[3];
	
	uint8_t freqStep, dutyStep, phaseStep;
	
	uint8_t channels[3];
	
	uint8_t currentChannel;
	uint8_t currentOption;
	uint32_t magicNum;
} Config_t;

struct FLASH_Sector {
	uint8_t data[1024-8];
	uint32_t nWrite;
	uint32_t checkSum;
};

union NVRAM {
	Config_t config;
	struct FLASH_Sector sector;
	uint32_t data32[256];
} DevNVRAM;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void procConfig(void const * argument) {
	#define FLASH_CONFIG_START_ADDR    ((uint32_t)0x08008000)
	#define FLASH_CONFIG_END_ADDR      FLASH_CONFIG_START_ADDR + FLASH_PAGE_SIZE
	
	uint32_t l_Address;
	uint32_t l_Index;
	
	l_Address = FLASH_CONFIG_START_ADDR;
	l_Index = 0x00;
	while (l_Address < FLASH_CONFIG_END_ADDR) {
		DevNVRAM.data32[l_Index] = *(__IO uint32_t *)l_Address;
		l_Index++;
		l_Address += 4;
	}
	
	if (DevNVRAM.config.magicNum != MAGIC_KEY_DEFINE) {
		DevNVRAM.config.periods[0] = 511;
		DevNVRAM.config.periods[1] = 511;
		DevNVRAM.config.periods[2] = 1023;
		DevNVRAM.config.pulses[0] = 255;
		DevNVRAM.config.pulses[1] = 255;
		DevNVRAM.config.pulses[2] = 511;
		DevNVRAM.config.phases[0] = 0;
		DevNVRAM.config.phases[1] = 0;
		DevNVRAM.config.phases[2] = 0;
		DevNVRAM.config.prescalers[0] = 0;
		DevNVRAM.config.prescalers[1] = 0;
		DevNVRAM.config.prescalers[2] = 0;
		DevNVRAM.config.freqStep = 1;
		DevNVRAM.config.dutyStep = 1;
		DevNVRAM.config.phaseStep = 1;
		DevNVRAM.config.channels[0] = 0;
		DevNVRAM.config.channels[1] = 0;
		DevNVRAM.config.channels[2] = 0;
		DevNVRAM.config.currentChannel = 0;
		DevNVRAM.config.currentOption = 0;
		
		DevNVRAM.config.magicNum = MAGIC_KEY_DEFINE;
		DevNVRAM.sector.nWrite = 0;
	}
}
void procConfigUpdate(void) {
	#define FLASH_CONFIG_START_ADDR    ((uint32_t)0x08008000)
	#define FLASH_CONFIG_END_ADDR      FLASH_CONFIG_START_ADDR + FLASH_PAGE_SIZE
	
	static FLASH_EraseInitTypeDef EraseInitStruct;
	
	uint32_t l_Address;
	uint32_t l_Error;
	uint32_t l_Index;
	
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = FLASH_CONFIG_START_ADDR;
	EraseInitStruct.NbPages = 0x01;
	
	l_Address = FLASH_CONFIG_START_ADDR;
	l_Error = 0x00;
	l_Index = 0x00;
	while (l_Address < FLASH_CONFIG_END_ADDR) {
		if (DevNVRAM.data32[l_Index] != *(__IO uint32_t*)l_Address) {
			l_Error++;
		}
		l_Index++;
		l_Address += 4;
	}
	
	if (l_Error > 0) {
		HAL_FLASH_Unlock();
		HAL_FLASHEx_Erase(&EraseInitStruct, &l_Error);
		l_Address = FLASH_CONFIG_START_ADDR;
		l_Error = 0x00;
		l_Index = 0x00;
		DevNVRAM.sector.nWrite++;
		while (l_Address < FLASH_CONFIG_END_ADDR) {
			HAL_Delay(5);
			if ( HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, l_Address, DevNVRAM.data32[l_Index]) == HAL_OK) {
				l_Index++;
				l_Address += 4;
			}
			HAL_Delay(10);
		}
		HAL_FLASH_Lock();
	}
}

void startTIM(TIM_TypeDef* tim) {
	tim->CCER = 1;
	tim->DMAR = 1;
}
void stopTIM(TIM_TypeDef* tim) {
	tim->CCER = 0;
	tim->DMAR = 0;
}
void startPWM(void) {
	TIM2->CNT = (uint32_t)phases[0] * TIM2->ARR / 0xFFFF;
	TIM3->CNT = (uint32_t)phases[1] * TIM3->ARR / 0xFFFF;
	TIM4->CNT = (uint32_t)phases[2] * TIM4->ARR / 0xFFFF;
	
	uint8_t ch = (channels[2] << 2) | (channels[1] << 1) | channels[0];
	
	switch (ch) {
		case 0x01:
			TIM2->CCER = TIM2->DMAR = 1;
			break;
		case 0x02:
			TIM3->CCER = TIM3->DMAR = 1;
			break;
		case 0x03:
			TIM2->CNT += 0x000A;
			TIM2->CCER = TIM2->DMAR = TIM3->CCER = TIM3->DMAR = 1;
			break;
		case 0x04:
			TIM4->CCER = TIM4->DMAR = 1;
			break;
		case 0x05:
			TIM2->CNT += 0x000A;
			TIM2->CCER = TIM2->DMAR = TIM4->CCER = TIM4->DMAR = 1;
			break;
		case 0x06:
			TIM3->CNT += 0x000A;
			TIM3->CCER = TIM3->DMAR = TIM4->CCER = TIM4->DMAR = 1;
			break;
		case 0x07:
			TIM3->CNT += 0x000A;
			TIM2->CNT += 0x0014;
			TIM2->CCER = TIM2->DMAR = TIM3->CCER = TIM3->DMAR = TIM4->CCER = TIM4->DMAR = 1;
			break;
	}
}
void stopPWM(void) {
	if (channels[0]) stopTIM(TIM2);
	if (channels[1]) stopTIM(TIM3);
	if (channels[2]) stopTIM(TIM4);
}
void setFrequency(int8_t change) {
	TIM_TypeDef* tim;
	switch (currentChannel) {
		case 0x00: tim = TIM2; break;
		case 0x01: tim = TIM3; break;
		case 0x02: tim = TIM4; break;
	}
	int32_t step = freqStep == 1 ? 1 : tim->ARR * freqStep / 512;
	uint16_t newARR = tim->ARR + change * (step | 1);
	stopPWM();
	if (change > 0 && newARR < tim->ARR) {
		if (tim->PSC < 0xFFFF) {
			tim->ARR = 0x8000;
			tim->PSC = ( (tim->PSC + 1) << 1) - 1;
			tim->CCR1 = tim->CCR1 > 0x0001 ? tim->CCR1 >> 1 : 0x0001;
		} else {
			tim->ARR = 0xFFFF;
		}
	} else if (change < 0 && newARR < 0x8000 && tim->PSC > 0x0000) {
		tim->ARR = 0xFFFF;
		tim->PSC >>= 1;
		tim->CCR1 = tim->CCR1 << 1 > tim->ARR ? tim->ARR : tim->CCR1 << 1;
	} else if (change < 0 && (newARR > tim->ARR || newARR < 0x0001) ) {
		tim->ARR = 0x0001;
	} else {
		tim->ARR = newARR;
	}
	
	if (tim == TIM4 || tim == TIM2) {
		tim->CCR1 = tim->ARR * dutys[currentChannel];
	}
		
	if (tim->CCR1 < 1) tim->CCR1 = 1;
	if (tim->CCR1 > tim->ARR) tim->CCR1 = tim->ARR;
	
	if (tim == TIM4) {
		TIM2->ARR = TIM2->ARR > TIM4->ARR ? (TIM2->ARR + 1) - (TIM2->ARR + 1) % (TIM4->ARR + 1) - 1 : TIM4->ARR;
		//TIM3->ARR = 0xFFFF;
		TIM3->ARR = TIM3->ARR > TIM2->ARR ? (TIM3->ARR + 1) - (TIM3->ARR + 1) % (TIM2->ARR + 1) - 1 : TIM2->ARR;
		TIM2->CCR1 = TIM2->ARR * dutys[0];
		TIM3->CCR1 = TIM3->ARR * dutys[1];
		if (TIM2->CCR1 < 1) TIM2->CCR1 = 1;
		if (TIM3->CCR1 < 1) TIM3->CCR1 = 1;
		DevNVRAM.config.periods[0] = TIM2->ARR;
		DevNVRAM.config.periods[1] = TIM3->ARR;
		DevNVRAM.config.pulses[0] = TIM2->CCR1;
		DevNVRAM.config.pulses[1] = TIM3->CCR1;
	}
	
	DevNVRAM.config.periods[currentChannel] = tim->ARR;
	DevNVRAM.config.pulses[currentChannel] = tim->CCR1;
	DevNVRAM.config.prescalers[currentChannel] = tim->PSC;
	
	startPWM();
}
void setDuty(int8_t change) {
	TIM_TypeDef* tim;
	switch (currentChannel) {
		case 0x00: tim = TIM2; break;
		case 0x01: tim = TIM3; break;
		case 0x02: tim = TIM4; break;
	}
	int32_t step = dutyStep == 1 ? 1 : dutyStep * tim->ARR / 512;
	uint16_t newCCR = tim->CCR1 + change * (step | 1);
	if (change > 0 && (newCCR < tim->CCR1 || newCCR > tim->ARR + 1) ) {
		tim->CCR1 = tim->ARR + 1 > 0x0000 ? tim->ARR + 1 : tim->ARR;
	} else if (change < 0 && newCCR > tim->CCR1) {
		tim->CCR1= 0x0000;
	} else {
		tim->CCR1 = newCCR;
	}
	
	switch (currentChannel) {
		case 0x00: DevNVRAM.config.pulses[0] = tim->CCR1; break;
		case 0x01: DevNVRAM.config.pulses[1] = tim->CCR1; break;
		case 0x02: DevNVRAM.config.pulses[2] = tim->CCR1; break;
	}
	dutys[currentChannel] = (float)(tim->CCR1 + 1) / (tim->ARR + 1);
}
void setPhase(int8_t change) {
	TIM_TypeDef* tim;
	switch (currentChannel) {
		case 0x00: tim = TIM2; break;
		case 0x01: tim = TIM3; break;
		case 0x02: tim = TIM4; break;
	}
	int32_t step = phaseStep == 1 ? 0xFFFF / tim->ARR : phaseStep * 128;
	phases[currentChannel] = phases[currentChannel] + change * (step | 1);
	switch (currentChannel) {
		case 0x00: DevNVRAM.config.phases[0] = phases[0]; break;
		case 0x01: DevNVRAM.config.phases[1] = phases[1]; break;
		case 0x02: DevNVRAM.config.phases[2] = phases[2]; break;
	}
	stopPWM();
	startPWM();
}
void setOption(void) {
	int8_t change;
	if (encChange < 0x80) {
		change = 1;
	} else {
		change = -1;
	}
	switch (currentOption) {
		case 0x00:
			setFrequency(change);
			break;
		case 0x01:
			setDuty(change);
			break;
		case 0x02:
			setPhase(change);
			break;
	}
}
void blink(uint8_t number) {
	uint16_t GPIO_Pin;
	switch (number) {
		case 0x00:
			GPIO_Pin = GPIO_PIN_4;
			break;
		case 0x01:
			GPIO_Pin = GPIO_PIN_5;
			break;
		case 0x02:
			GPIO_Pin = GPIO_PIN_3;
			break;
	}
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
	for (uint8_t i = 0x00; i < 0x03; i++) {
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOB, GPIO_Pin, GPIO_PIN_SET);
		HAL_Delay(10);
		HAL_GPIO_WritePin(GPIOB, GPIO_Pin, GPIO_PIN_RESET);
	}
}
void blinkAll() {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
	for (uint8_t i = 0x00; i < 0x03; i++) {
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
		HAL_Delay(10);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
	}
}
void incCounter(uint8_t number, GPIO_PinState PinState) {
	if (PinState == GPIO_PIN_RESET) {
		btnCounts[number]++;
	} else {
		btnCounts[number] = 0x0000;
	}
}
void handleChannelBtn(uint8_t number) {
	uint16_t GPIO_Pin;
	TIM_TypeDef* tim;
	switch (number) {
		case 0x00:
			GPIO_Pin = GPIO_PIN_11;
			tim = TIM2;
			break;
		case 0x01:
			GPIO_Pin = GPIO_PIN_12;
			tim = TIM3;
			break;
		case 0x02:
			GPIO_Pin = GPIO_PIN_10;
			tim = TIM4;
			break;
	}
	incCounter(number, HAL_GPIO_ReadPin(GPIOA, GPIO_Pin) );
	
	if (btnCounts[number] == WAIT_TICKS) {
		channels[number] = 0x01 - channels[number];
		if (channels[number]) {
			stopPWM();
			startPWM();
		} else {
			stopTIM(tim);
		}
		switch (number) {
			case 0x00:
				DevNVRAM.config.channels[0] = channels[0];
				break;
			case 0x01:
				DevNVRAM.config.channels[1] = channels[1];
				break;
			case 0x02:
				DevNVRAM.config.channels[2] = channels[2];
				break;
		}
	}
}
void handleChannelSelectBtn() {
	incCounter(0x03, HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) );
	
	if (btnCounts[3] == WAIT_TICKS) {
		currentChannel = (currentChannel + 0x02) % 0x03;
		DevNVRAM.config.currentChannel = currentChannel;
		blink(currentChannel);
	}
}
void handleOptionSelectBtn() {
	incCounter(0x04, HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) );
	
	if (btnCounts[4] == WAIT_TICKS) {
		currentOption = (currentOption + 0x01) % 0x03;
		DevNVRAM.config.currentOption = currentOption;
	}
}
void handleStepSelectBtn() {
	incCounter(0x05, HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) );
	
	if (btnCounts[5] == WAIT_TICKS) {
		switch (currentOption) {
			case 0x00:
				freqStep = freqStep << 2 | freqStep >> 6;
				DevNVRAM.config.freqStep = freqStep;
				break;
			case 0x01:
				dutyStep = dutyStep << 2 | dutyStep >> 6;
				DevNVRAM.config.dutyStep = dutyStep;
				break;
			case 0x02:
				phaseStep = phaseStep << 2 | phaseStep >> 6;
				DevNVRAM.config.phaseStep = phaseStep;
				break;
		}
	}
}
void load(void) {
	periods[0] = DevNVRAM.config.periods[0] ? DevNVRAM.config.periods[0] : 511;
	periods[1] = DevNVRAM.config.periods[1] ? DevNVRAM.config.periods[1] : 511;
	periods[2] = DevNVRAM.config.periods[2] ? DevNVRAM.config.periods[2] : 1023;
	pulses[0] = DevNVRAM.config.pulses[0] ? DevNVRAM.config.pulses[0] : 255;
	pulses[1] = DevNVRAM.config.pulses[1] ? DevNVRAM.config.pulses[1] : 255;
	pulses[2] = DevNVRAM.config.pulses[2] ? DevNVRAM.config.pulses[2] : 511;
	phases[0] = DevNVRAM.config.phases[0];
	phases[1] = DevNVRAM.config.phases[1];
	phases[2] = DevNVRAM.config.phases[2];
	prescalers[0] = DevNVRAM.config.prescalers[0];
	prescalers[1] = DevNVRAM.config.prescalers[1];
	prescalers[2] = DevNVRAM.config.prescalers[2];
	dutys[0] = (float)(pulses[0] + 1) / (periods[0] + 1);
	dutys[1] = (float)(pulses[1] + 1) / (periods[1] + 1);
	dutys[2] = (float)(pulses[2] + 1) / (periods[2] + 1);
	
	freqStep = DevNVRAM.config.freqStep ? DevNVRAM.config.freqStep : 1;
	dutyStep = DevNVRAM.config.dutyStep ? DevNVRAM.config.dutyStep : 1;
	phaseStep = DevNVRAM.config.phaseStep ? DevNVRAM.config.phaseStep : 1;
	
	channels[0] = DevNVRAM.config.channels[0];
	channels[1] = DevNVRAM.config.channels[1];
	channels[2] = DevNVRAM.config.channels[2];
	
	currentChannel = DevNVRAM.config.currentChannel;
	currentOption = DevNVRAM.config.currentOption;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
	RCC->APB1ENR |= 0x18000000;
	PWR->CR |= 0x100;
	procConfig(0);
	load();
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
	
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_1);
	
	TIM2->ARR = periods[0];
	TIM3->ARR = periods[1];
	TIM4->ARR = periods[2];
	TIM2->CCR1 = pulses[0];
	TIM3->CCR1 = pulses[1];
	TIM4->CCR1 = pulses[2];
	TIM2->PSC = prescalers[0];
	TIM3->PSC = prescalers[1];
	TIM4->PSC = prescalers[2];
	
	startPWM();
	
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
	
	blinkAll();
	stopPWM();
	startPWM();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		if (!waitingToSave) {
			procConfigUpdate();
			waitingToSave = WAIT_SAVE_TICKS;
			saveCount++;
		}
		waitingToSave--;
		
		encChange = TIM1->CNT - encCounter;
		if (waiting > 0) waiting--;
		if (!waiting && encChange != 0x00) {
			waiting = WAIT_TICKS;
			setOption();
		}
		encCounter = TIM1->CNT;
		
		handleChannelBtn(0x00);
		handleChannelBtn(0x01);
		handleChannelBtn(0x02);
		handleChannelSelectBtn();
		handleOptionSelectBtn();
		handleStepSelectBtn();
		
		PWMCounter++;
		if (PWMCounter < brightness) {
			if (channels[0]) {
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
			}
			if (channels[1]) {
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
			}
			if (channels[2]) {
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
			}
		} else {
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
		}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 255;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI1;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 15;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 15;
  if (HAL_TIM_Encoder_Init(&htim1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 8192;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 8192;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 8192;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PA10 PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB3 PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
