/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <math.h>
#define ARM_MATH_CM4
#include "arm_math.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim){
	HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);

}*/
#define BUFFER_NUM 100
#define LED_NUM 60
#define LED_SLOT 40
uint16_t ADC_BUFFER[BUFFER_NUM];
uint16_t pwmData[LED_NUM*24+LED_SLOT];
volatile int SPEED = 20;
void SET_LED(int numLED, int Green, int Red, int Blue){
	uint32_t data = (Green<<16) | (Red<<8) | Blue;
	for (int i=0; i<24;i++){
			if (data&(1<<(23-i))) pwmData[numLED*24+i] = 60;
			else pwmData[numLED*24+i]=30;
		}
}

void send (){
	for (int i = (LED_NUM)*24; i < (LED_NUM)*24 + LED_SLOT; i++) {
	        pwmData[i] = 0;
	    }
	HAL_TIM_PWM_Start_DMA(&htim1,TIM_CHANNEL_1, (uint32_t *) pwmData, LED_NUM*24+LED_SLOT);
}

volatile int effect_led=0, effect_sounds=0, Mode=0;
int last_press_1 =0, last_press_2=0, last_press_3=0;

  int effStep=0;
  void Reset(){
	  effStep=0;
  }
  uint8_t Rainboweffect() {
    float factor1, factor2;
    uint16_t ind;
    for(uint16_t j=0;j<60;j++) {
      ind = effStep + j * 1;
      switch((int)((ind % 60) / 20)) {
        case 0: factor1 = 1.0 - ((float)(ind % 60 - 0 * 20) / 20);
                factor2 = (float)((int)(ind - 0) % 60) / 20;
                SET_LED(j, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2);
                break;
        case 1: factor1 = 1.0 - ((float)(ind % 60 - 1 * 20) / 20);
                factor2 = (float)((int)(ind - 20) % 60) / 20;
                SET_LED(j, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2);
                break;
        case 2: factor1 = 1.0 - ((float)(ind % 60 - 2 * 20) / 20);
                factor2 = (float)((int)(ind - 40) % 60) / 20;
                SET_LED(j, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2);
                break;
      }
    }
    send();
    HAL_Delay(SPEED);
    if(effStep >= 60) {Reset(); return 0x03; }
    else effStep++;
    return 0x01;
  }

  int choose=0, done =0;
  int GtoRres=0, RtoBres=0, BtoGres = 0;
  void SingleColorChange(){
	  if (choose == 0){
		  for (int i=0;i<60;i++){
			  SET_LED(i,255-GtoRres,GtoRres,0);
		  }
		  send();
		  GtoRres++;
		  if (GtoRres > 255){
			  done =1;
			  GtoRres=0;
		  }
	  } else
	  if (choose == 1){
		  for (int i=0;i<60;i++){
			  SET_LED(i,0,255-RtoBres,RtoBres);
		  }
		  send();
		  RtoBres++;
		  if (RtoBres > 255){
			  done =1;
			  RtoBres=0;
		  }
	  } else
	  if (choose == 2){
		  for (int i=0;i<60;i++){
			  SET_LED(i,BtoGres,0,255-BtoGres);
		  }
		  send();
		  BtoGres++;
		  if (BtoGres > 255){
			  done =1;
			  BtoGres=0;
		  }
	  }
	  if (done == 1){
		  done =0;
		  choose=(choose+1) % 3;
	  }
	  HAL_Delay(SPEED);
  };
  void Strobe(){
	  if (choose == 0){
	  		  for (int i=0;i<60;i++){
	  			  SET_LED(i,255-GtoRres,GtoRres,0);
	  		  }
	  		  send();
	  		  GtoRres++;
	  		  if (GtoRres > 255){
	  			  done =1;
	  			  GtoRres=0;
	  		  }
	  	  } else
	  	  if (choose == 1){
	  		  for (int i=0;i<60;i++){
	  			  SET_LED(i,0,255-RtoBres,RtoBres);
	  		  }
	  		  send();
	  		  RtoBres++;
	  		  if (RtoBres > 255){
	  			  done =1;
	  			  RtoBres=0;
	  		  }
	  	  } else
	  	  if (choose == 2){
	  		  for (int i=0;i<60;i++){
	  			  SET_LED(i,BtoGres,0,255-BtoGres);
	  		  }
	  		  send();
	  		  BtoGres++;
	  		  if (BtoGres > 255){
	  			  done =1;
	  			  BtoGres=0;
	  		  }
	  	  }
	  	  if (done == 1){
	  		  done =0;
	  		  choose=(choose+1) % 3;
	  	  }
	  	  HAL_Delay(SPEED);
	  	  for (int i=0;i <60;i++){
	  		  SET_LED(i,0, 0, 0);
	  	  }
	  	  send();
	  	  HAL_Delay(SPEED);
  }
  void PixelRun(){
	  for (int i=0; i<60;i++){
		  for (int j=0;j<60;j++){
			  if (j==i){
				  SET_LED(j, 0, 0, 255);
			  } else SET_LED(j, 0, 0, 0);
		  }
		  send();
		  HAL_Delay(SPEED +20);
	  }
  }

  uint16_t sounds_effect = 0;
  void Reset_Sounds_Effect(){
	  sounds_effect = 0;
  }
  void SET_LED_NUM_BY_SOUNDS(){
	  for (int i=0; i < BUFFER_NUM; i++){
		  HAL_ADC_Start(&hadc1);
		  HAL_ADC_PollForConversion(&hadc1, 10);
		  ADC_BUFFER[i] = HAL_ADC_GetValue(&hadc1);
		  HAL_ADC_Stop(&hadc1);
	  }
	  uint32_t sum=0;
	  uint32_t center=0;
	  int Led_On;
	  float volume;
	  for (int i=0; i < BUFFER_NUM; i++){
		  center = (uint32_t)ADC_BUFFER[i];
		  sum = sum + (center * center);
	  }
	  volume =sqrtf (sum / (float)BUFFER_NUM);
	  Led_On = ((int)volume * LED_NUM)/4095;
	  float factor1, factor2;
	  uint16_t ind;
	  for (int i=0; i < LED_NUM;i++){
		  if (i < Led_On){
			  ind = sounds_effect +i * 1;
				switch((int)((ind % 60) / 20)) {
				  case 0: factor1 = 1.0 - ((float)(ind % 60 - 0 * 20) / 20);
						  factor2 = (float)((int)(ind - 0) % 60) / 20;
						  SET_LED(i, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2);
						  break;
				  case 1: factor1 = 1.0 - ((float)(ind % 60 - 1 * 20) / 20);
						  factor2 = (float)((int)(ind - 20) % 60) / 20;
						  SET_LED(i, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2);
						  break;
				  case 2: factor1 = 1.0 - ((float)(ind % 60 - 2 * 20) / 20);
						  factor2 = (float)((int)(ind - 40) % 60) / 20;
						  SET_LED(i, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2);
						  break;
				}
		  } else {
			  SET_LED(i,0,0,0);
		  }
	  }

	  send();
	  HAL_Delay(10);
	  if (sounds_effect >= LED_NUM){
		  Reset_Sounds_Effect();
	  } else
		  sounds_effect ++;
  }

  void SET_LED_COLOR_BY_SOUNDS(){
	  for (int i=0; i < BUFFER_NUM; i++){
		  HAL_ADC_Start(&hadc1);
		  HAL_ADC_PollForConversion(&hadc1, 10);
		  ADC_BUFFER[i] = HAL_ADC_GetValue(&hadc1);
		  HAL_ADC_Stop(&hadc1);
	  }
	  uint32_t sum=0;
	  uint32_t center=0;
	  float volume;
	  for (int i=0; i < BUFFER_NUM; i++){
		  center = (uint32_t)ADC_BUFFER[i];
		  sum = sum + (center * center);
	  }
	  volume =sqrtf (sum / (float)BUFFER_NUM); //volume co gia tri [200,4095]

	  // Scale volume về 0.0–1.0
	  float ratio = (volume + 500) / 3895.0f;
	  int G, R, B;

	  // Ánh xạ theo dải nhiệt độ: Xanh → Lá → Vàng → Đỏ
	  if (ratio < 0.33f) {
		  // Xanh dương → Xanh lá
		  float t = ratio / 0.33f;
		  R = 0;
		  G = (uint8_t)(t * 255);
		  B = (uint8_t)((1 - t) * 255);
	  }
	  else if (ratio < 0.66f) {
		  // Xanh lá → Vàng
		  float t = (ratio - 0.33f) / 0.33f;
		  R = (uint8_t)(t * 255);
		  G = 255;
		  B = 0;
	  }
	  else {
		  // Vàng → Đỏ
		  float t = (ratio - 0.66f) / 0.34f;
		  R = 255;
		  G = (uint8_t)((1 - t) * 255);
		  B = 0;
	  }

	  for (int i =0; i < LED_NUM; i++){
		  SET_LED(i,G, R, B);
	  }
	  send();
	  HAL_Delay(10);
  }

#define FFT_SIZE 1024            // Số điểm FFT (phải là lũy thừa của 2)
#define SAMPLE_RATE 44100.0f        // Tốc độ lấy mẫu (Hz)
uint32_t adc_buffer_hz[FFT_SIZE];  // DMA điền dữ liệu vào đây

float get_dominant_frequency(void) {
      float input_f32[FFT_SIZE];
      float fft_output[FFT_SIZE];         // FFT trả về FFT_SIZE điểm
      float magnitude[FFT_SIZE / 2];      // Biên độ chỉ tính cho nửa phổ tần số
      arm_rfft_fast_instance_f32 fft_instance;

      for (int i = 0; i < FFT_SIZE; i++){
          input_f32[i] = ((float)adc_buffer_hz[i]-2048.0f) / 2048.0f; // Giả sử 12-bit ADC
      }
      // Khởi tạo FFT
      arm_rfft_fast_init_f32(&fft_instance, FFT_SIZE);
      // Thực hiện FFT
      arm_rfft_fast_f32(&fft_instance, input_f32, fft_output, 0);
      arm_cmplx_mag_f32(fft_output,magnitude, FFT_SIZE/2);
      int max_index = 6; // bỏ index 0 vì thường là DC offset
      float max_value = magnitude[6];
      for (int i = 7; i < FFT_SIZE / 2; i++) {
          if (magnitude[i] > max_value) {
              max_value = magnitude[i];
              max_index = i;
          }
      }

      float freq_resolution = (float)(SAMPLE_RATE/2) / (float)FFT_SIZE; // Tính tần số tương ứng
      float dominant_freq = max_index * freq_resolution;
      return dominant_freq;
  }

void One_Frequence(){
	  HAL_TIM_Base_Start(&htim2);
	  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer_hz, FFT_SIZE);
	  float freq_hz;
	  int led_on_hz;
	  char array[]= "\n\r";
	  freq_hz=get_dominant_frequency();
	  int out_freq;
	  out_freq=freq_hz;
	  char out[20];
	  sprintf(out, "%d", out_freq);

	  HAL_UART_Transmit(&huart1, (uint8_t*)out, strlen(out), HAL_MAX_DELAY);
	  HAL_UART_Transmit(&huart1, (uint8_t*)array, strlen(array), HAL_MAX_DELAY);
	  HAL_Delay(500);

	  led_on_hz = (freq_hz / (SAMPLE_RATE / 4)) * LED_NUM;
	  for (int i=0; i<LED_NUM;i++){
		  if (i<led_on_hz){
			  SET_LED(i, 255,0,0);
		  } else{
			  SET_LED(i,0,0,0);
		  }
	  }
  send();
}

void Led_Frequence(){
	HAL_TIM_Base_Start(&htim2);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer_hz, FFT_SIZE);
	float input_f32[FFT_SIZE];
	float fft_output[FFT_SIZE];         // FFT trả về FFT_SIZE điểm
	float magnitude[FFT_SIZE / 2];      // Biên độ chỉ tính cho nửa phổ tần số
	arm_rfft_fast_instance_f32 fft_instance;

	// Chuẩn hóa dữ liệu ADC về float [-1.0, 1.0]
	for (int i = 0; i < FFT_SIZE; i++){
	  input_f32[i] = ((float)adc_buffer_hz[i]-2048.0f) / 2048.0f; // Giả sử 12-bit ADC
	}
	// Khởi tạo FFT
	arm_rfft_fast_init_f32(&fft_instance, FFT_SIZE);

	// Thực hiện FFT
	arm_rfft_fast_f32(&fft_instance, input_f32, fft_output, 0);
	arm_cmplx_mag_f32(fft_output,magnitude, FFT_SIZE/2);
	int bass, mid, treb;
	int bass_id, mid_id, treb_id;
	int bass_led, mid_led, treb_led;
	bass = magnitude[1];
	bass_id=1;
	mid = magnitude[15];
	mid_id=15;
	treb = magnitude[182];
	treb_id = magnitude[182];
	for (int i=1; i<15; i++){
		if (bass < magnitude[i]){
			bass = magnitude[i];
			bass_id=i;
		}
	}
	bass_led=bass_id*17;
	for (int i=15; i<182; i++){
		if (mid<magnitude[i]){
			mid=magnitude[i];
			mid_id=i;
		}
	}
	mid_led = (float)(mid_id -15) * 1.5;
	for (int i=182; i<FFT_SIZE/2; i++){
		if (treb <magnitude[i]){
			treb=magnitude[i];
			treb_id=i;
		}
	}
	treb_led = (float)(treb_id-182)*0.7;
	int freq_resolution = 22;
	bass_id = bass_id *freq_resolution;
	mid_id= mid_id *freq_resolution;
	treb_id = treb_id * freq_resolution;
	for (int i=0;i<LED_NUM;i++){
		if (i <20){
			SET_LED(i,0,0,bass_led);
		} else{
			if (i<40){
				SET_LED(i,mid_led,0,0);
			} else{
				SET_LED(i,0,treb_led,0);
			}
		}
	}
	send();
	char space[] = "  ";
	char endline[] = "\n\r";
	 char out1[20];
	 char out2[20];
	 char out3[20];
	 sprintf(out1, "%d", bass_id);
	 sprintf(out2, "%d", mid_id);
	 sprintf(out3, "%u", treb_id);
	HAL_UART_Transmit(&huart1, (uint8_t*)out1, strlen(out1), HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart1, (uint8_t*)space, strlen(space), HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart1, (uint8_t*)out2, strlen(out2), HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart1, (uint8_t*)space, strlen(space), HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart1, (uint8_t*)out3, strlen(out3), HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart1, (uint8_t*)endline, strlen(endline), HAL_MAX_DELAY);
	 HAL_Delay(500);

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

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  if (Mode ==0){
		  if (effect_led == 0){
			  Rainboweffect();
		  } else
		  if (effect_led == 1){
			  SingleColorChange();
		  } else
		  if (effect_led == 2){
			  Strobe();
		  } else
		  if (effect_led == 3){
			  PixelRun();
		  }
	  } else
	  if (Mode == 1){
		  if (effect_sounds == 0){
			  SET_LED_NUM_BY_SOUNDS();
		  } else
		  if (effect_sounds == 1){
			  SET_LED_COLOR_BY_SOUNDS();
		  } else
		  if (effect_sounds == 2){
			  One_Frequence();
		  } else
		  if (effect_sounds == 3){
			  Led_Frequence();
		  }
	  }
	  HAL_Delay(10);
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
  /* EXTI1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
  /* EXTI2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	  if (GPIO_Pin == GPIO_PIN_1){
		  if (Mode == 0){
			   if (HAL_GetTick() - last_press_1 >= 200){
					effect_led = (effect_led + 1) % 4;
					last_press_1 = HAL_GetTick();
					SPEED = 20;
			   }
		  } else
		  if (Mode == 1){
			  if (HAL_GetTick() - last_press_1 >= 200){
					effect_sounds = (effect_sounds + 1) % 4;
					last_press_1 = HAL_GetTick();
			   }
		  }
	  } else
	  if (GPIO_Pin == GPIO_PIN_2){
		  if (HAL_GetTick() - last_press_2 >= 200){
				SPEED= (SPEED+10) %100;
				last_press_2 = HAL_GetTick();
		   }
	  } else
	  if (GPIO_Pin == GPIO_PIN_3){
		  if (HAL_GetTick() - last_press_3 >= 200){
			  	if (Mode == 0){
			  		Mode = 1;
			  	} else
			  		if (Mode == 1){
			  			Mode = 0;
			  			SPEED = 20;
			  		}
		   }
	  }
  }
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
