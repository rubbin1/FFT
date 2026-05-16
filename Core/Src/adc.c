/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "adc.h"

#include "tim.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/* ADC1 init function */
void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* ADC1 clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PC0     ------> ADC1_IN10
    */
    GPIO_InitStruct.Pin = ADC_IN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(ADC_IN_GPIO_Port, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc1);

  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC1_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PC0     ------> ADC1_IN10
    */
    HAL_GPIO_DeInit(ADC_IN_GPIO_Port, ADC_IN_Pin);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);
  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
#include "private_typedef.h"

#define VREFINT_TYPICAL 1.20f   // 典型值 1.2V

float ADC_GetVDDA(void)
{
  // 保存当前 ADC 配置（如果需要）
  // 临时将 ADC 触发方式改为软件触发，停止 DMA
  HAL_ADC_Stop_DMA(&hadc1);
  HAL_TIM_Base_Stop(&htim3);   // 停止定时器触发

  // 配置 ADC 通道为内部参考电压（通道 17）
  ADC_ChannelConfTypeDef sConfig = {0};
  sConfig.Channel = ADC_CHANNEL_VREFINT;   // 通常定义为 17
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5; // 长采样时间保证稳定
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
    // 恢复原通道并返回默认值
    sConfig.Channel = ADC_CHANNEL_10;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcbuf_flag.raw, system_config.adc_buffer_size);
    HAL_TIM_Base_Start(&htim3);
    return 3.3f;
  }

  // 软件触发转换
  HAL_ADC_Start(&hadc1);
  if (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK) {
    // 超时处理
    HAL_ADC_Stop(&hadc1);
    sConfig.Channel = ADC_CHANNEL_10;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcbuf_flag.raw, system_config.adc_buffer_size);
    HAL_TIM_Base_Start(&htim3);
    return 3.3f;
  }
  uint16_t raw_vrefint = HAL_ADC_GetValue(&hadc1);
  HAL_ADC_Stop(&hadc1);

  // 恢复原 ADC 通道（PC0）
  sConfig.Channel = ADC_CHANNEL_10;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

  // 恢复 DMA 和定时器
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcbuf_flag.raw, system_config.adc_buffer_size);
  HAL_TIM_Base_Start(&htim3);

  // 计算实际 VDDA
  if (raw_vrefint == 0) return 3.3f;
  float vdda = VREFINT_TYPICAL * 4095.0f / raw_vrefint;
  return vdda;
}
/* USER CODE END 1 */
