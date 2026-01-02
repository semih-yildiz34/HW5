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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lib_image.h"
#include "lib_serialimage.h"
#include "lib_nn.h"
#include "model_q1_int.h"  // For Q1: hu_mean_q and hu_std_q
#include "model_q2_int.h"  // For Q2: hu_mean_q2 and hu_std_q2
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
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// ============================================================================
// IMAGE BUFFERS FOR DIGIT RECOGNITION
// ============================================================================
volatile uint8_t pImageGray[28*28*1];       // Input grayscale digit image (MNIST: 28x28)

// ============================================================================
// IMAGE STRUCTURE HANDLES
// ============================================================================
IMAGE_HandleTypeDef imgGray;     // Input grayscale digit image
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
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  // Initialize image structure for digit recognition (MNIST: 28x28)
  LIB_IMAGE_InitStruct(&imgGray, (uint8_t*)pImageGray, 28, 28, IMAGE_FORMAT_GRAYSCALE);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // ============================================================
    // DIGIT RECOGNITION: Q1 (Single Neuron) or Q2 (MLP)
    // ============================================================
    // CYCLE: Receive grayscale digit image → Calculate Hu moments → 
    //        Run neural network → Send predicted digit
    
    if (LIB_SERIAL_IMG_Receive(&imgGray) == SERIAL_OK)
    {
        // Calculate Hu moments (7 features)
        float huMoments[7];
        if (LIB_IMAGE_CalculateHuMoments(&imgGray, huMoments) == IMAGE_OK)
        {
            // Normalize Hu moments using stored mean and std (convert Q15 to float first)
            float mean[7], std[7];
            for (int i = 0; i < 7; i++)
            {
                mean[i] = (float)hu_mean_q[i] / 32768.0f;
                std[i] = (float)hu_std_q[i] / 32768.0f;
            }
            
            // Normalize: (hu - mean) / std
            float normalized[7];
            for (int i = 0; i < 7; i++)
            {
                if (std[i] != 0.0f)
                {
                    normalized[i] = (huMoments[i] - mean[i]) / std[i];
                }
                else
                {
                    normalized[i] = 0.0f;
                }
            }
            
            // Convert normalized features to Q15 format
            // Use int32_t to avoid overflow (normalized values can be large)
            int32_t huQ15[7];
            for (int i = 0; i < 7; i++)
            {
                // Convert float to Q15: multiply by 32768
                // Don't clamp - keep as int32_t to preserve precision
                huQ15[i] = (int32_t)(normalized[i] * 32768.0f);
            }
            
            // Run Q1: Single neuron classifier (0 vs not-0)
            // Use Q1 normalization stats (already applied above)
            uint8_t q1_result = LIB_NN_PredictQ1((int16_t*)huQ15);
            
            // Run Q2: MLP classifier (0-9)
            // Q2 uses different normalization stats - re-normalize
            float mean_q2[7], std_q2[7];
            for (int i = 0; i < 7; i++)
            {
                mean_q2[i] = (float)hu_mean_q2[i] / 32768.0f;
                std_q2[i] = (float)hu_std_q2[i] / 32768.0f;
            }
            
            float normalized_q2[7];
            for (int i = 0; i < 7; i++)
            {
                if (std_q2[i] != 0.0f)
                {
                    normalized_q2[i] = (huMoments[i] - mean_q2[i]) / std_q2[i];
                }
                else
                {
                    normalized_q2[i] = 0.0f;
                }
            }
            
            int32_t huQ15_q2[7];
            for (int i = 0; i < 7; i++)
            {
                huQ15_q2[i] = (int32_t)(normalized_q2[i] * 32768.0f);
            }
            
            uint8_t q2_result = LIB_NN_PredictQ2((int16_t*)huQ15_q2);
            
            // Send Q1 result (binary: 0 or 1)
            LIB_SERIAL_SendDigit(q1_result);
            
            // Delay to ensure byte is fully transmitted and Python can read it
            HAL_Delay(200);
            
            // Send Q2 result (digit: 0-9)
            LIB_SERIAL_SendDigit(q2_result);
            
            // Delay before requesting next image to allow Python to read results
            HAL_Delay(200);
        }
    }
    
    // ============================================================
    // CYCLE COMPLETE - Wait for next image
    // ============================================================
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 2000000;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
#ifdef USE_FULL_ASSERT
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
