/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * 自233-尹鸿宇-U202342240
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
//输入模式
typedef enum
{
  SINGLE_WAVE_Input,
  MULTI_WAVE_Input,
}Input_Mode;

//是否进行图像显示
typedef enum
{
  IMAGE_MODE_ON,
  IMAGE_MODE_OFF,
}IMAGE_MOD;
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
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_Delay(20);
  OLED_Init();

  //非正弦输入的频率和幅值和正弦输入的频率和幅值
  float exact_freq = 0;
  float exact_ampl = 0;

  //定义Input_Mode为单信号输入，图像模式同理
  Input_Mode current_mode = SINGLE_WAVE_Input;
  IMAGE_MOD current_imaging_mode = IMAGE_MODE_OFF;

  //定义一个用于控制非正弦输入时，OLED屏幕页数的变量
  int pages = 0;
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
    switch (current_mode)
    {
    case SINGLE_WAVE_Input:
      generate_sin_wave();
      float probably_freq = zero_crossing_raw(Data_buffer, 1024, 10000.f);
      if (probably_freq > 0)
      {
        precise_measure(probably_freq, &exact_freq, &exact_ampl);
        OLED_Show_sin_input(exact_freq , exact_ampl);
      }
      break;
    case MULTI_WAVE_Input:
      generate_square_wave();
      float freqs[5], ampls[5];
      fft_process_harmonics(freqs, ampls);
      if (current_imaging_mode == IMAGE_MODE_ON)
        OLED_Show_Image(freqs[0]);
      else
        OLED_Show_mul_input(freqs, ampls, pages);
      break;
    }
    /* USER CODE END 3 */
  }
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
