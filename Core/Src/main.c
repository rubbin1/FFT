/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * 自233-尹鸿宇-U202342240
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "key_press.h"
#include "fft.h"
#include "oled.h"
#include "show_data.h"
#include "zero_crossing_and_dft.h"
#include "private_typedef.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ADC_BUFFER_SIZE 1024
#define FFT_LEN 1024
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

//定义Input_Mode为单信号输入，图像模式同理
Input_Mode current_mode = SINGLE_WAVE_Input;
IMAGE_MOD current_imaging_mode = IMAGE_MODE_OFF;

//定义一个用于控制非正弦输入时，OLED屏幕页数的变量
int pages = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  FFT_Init();
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADCEx_Calibration_Start(&hadc1);

  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcbuf_flag.raw, system_config.adc_buffer_size);

  HAL_TIM_Base_Start(&htim3);

  HAL_TIM_Base_Start_IT(&htim2);
  HAL_Delay(20);
  OLED_Init();
  //加入一个开屏动画
  Open_OLED_Show();

  const float vref = system_config.adc_vref;
  const uint16_t res = system_config.adc_resolution;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /*依旧是按键检测
     *key0: 切换输入模式
     */
    if (key1.short_pressed_flag)
    {
      key1.short_pressed_flag = 0;
      if (current_mode == MULTI_WAVE_Input && current_imaging_mode == IMAGE_MODE_OFF)
      {
        pages = (pages + 1) % 5;
      }
    }
    if (key2.short_pressed_flag)
    {
      key2.short_pressed_flag = 0;
      if (current_mode == MULTI_WAVE_Input)
      {
        if (current_imaging_mode == IMAGE_MODE_OFF)   current_imaging_mode = IMAGE_MODE_ON;
        else current_imaging_mode = IMAGE_MODE_OFF;
      }
    }
    if (key0.short_pressed_flag)
    {
      key0.short_pressed_flag = 0;

      if (current_mode == SINGLE_WAVE_Input)
      {
        //进入非正弦输入时，每次都要回到基波界面
        pages = 0;
        current_mode = MULTI_WAVE_Input;
        current_imaging_mode = IMAGE_MODE_OFF;
      }
      else current_mode = SINGLE_WAVE_Input;
    }

    if (adcbuf_flag.data_ready)
    {
      adcbuf_flag.data_ready = 0;

      // 数据处理
      float sum = 0;
      for (int i = 0; i < ADC_BUFFER_SIZE; i++)
      {
        float v = adcbuf_flag.snapshot[i] * vref / res;
        Data_buffer[ADC_BUFFER_SIZE + i] = v;
        sum += v;
      }
      float avg = sum / system_config.adc_buffer_size;

      //去直流
      for (int i = 0; i < system_config.adc_buffer_size; i++)
      {
        Data_buffer[system_config.adc_buffer_size + i] -= avg;
      }

      switch (current_mode)
      {
      case SINGLE_WAVE_Input:
        Data_buffer_sin(Data_buffer);
        float probably_freq = zero_crossing_raw(Data_buffer, system_config.adc_buffer_size);
        if (probably_freq > 0)
        {
          precise_measure(probably_freq);
          OLED_Show_sin_input();
        }
        break;
      case MULTI_WAVE_Input:
        Data_buffer_nosin(Data_buffer);
        fft_process_harmonics();
        if (current_imaging_mode == IMAGE_MODE_ON)
        {
          if (harmonicsResult.fundamental.frequency <= 0)  break;
          OLED_Show_Image(adcbuf_flag.snapshot, harmonicsResult.fundamental.frequency);
        }
        else
        {
          float freqs[5] =
          {
            harmonicsResult.fundamental.frequency,
            harmonicsResult.harmonics[0].frequency,
            harmonicsResult.harmonics[1].frequency,
            harmonicsResult.harmonics[2].frequency,
            harmonicsResult.harmonics[3].frequency
          };
          float ampls[5] =
          {
            harmonicsResult.fundamental.amplitude,
            harmonicsResult.harmonics[0].amplitude,
            harmonicsResult.harmonics[1].amplitude,
            harmonicsResult.harmonics[2].amplitude,
            harmonicsResult.harmonics[3].amplitude
          };
          OLED_Show_mul_input(freqs, ampls, pages);
        }
        break;
      }
    }
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    Key_Press(&key0, KEY0_Pin, KEY0_GPIO_Port);
    Key_Press(&key1, KEY1_Pin, KEY1_GPIO_Port);
    Key_Press(&key2, KEY2_Pin, KEY2_GPIO_Port);
  }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  if (hadc->Instance == ADC1)
  {
    // 快速复制原始ADC数据
    memcpy(adcbuf_flag.snapshot, adcbuf_flag.raw, sizeof(adcbuf_flag.snapshot));
    adcbuf_flag.data_ready = 1;
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
