/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    IOS_BROKEN   = 0x00,
    IOS_READY    = 0x01,
    IOS_UPDATING = 0x02,
    IOS_COPYING  = 0x03
} IOS_STATE;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define APP1_VERSION_ADDR       0x08001000
#define APP1_IOS_STATE          0x08001004
#define APP1_IOS_SIZE           0x08001008

#define APP1_PROGRAM_ADDR       0x08001400

#define APP2_VERSION_ADDR       0x0800100C
#define APP2_IOS_STATE          0x08001010
#define APP2_IOS_SIZE           0x08001014

#define APP2_PROGRAM_ADDR       0x08008C00
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t write_data = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
uint8_t min(uint8_t data1, uint8_t data2);
void jumpToProgram(uint32_t program_addr);
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

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
    uint32_t app1_version = *(volatile uint32_t *)APP1_VERSION_ADDR;
    uint32_t app2_version = *(volatile uint32_t *)APP2_VERSION_ADDR;
    uint32_t app1_state   = *(volatile uint32_t *)APP1_IOS_STATE;
    uint32_t app2_state   = *(volatile uint32_t *)APP2_IOS_STATE;
    uint32_t app1_size    = *(volatile uint32_t *)APP1_IOS_SIZE;
    uint32_t app2_size    = *(volatile uint32_t *)APP2_IOS_SIZE;
    
    uint8_t need_copy = 0U;

    if (app2_state == IOS_COPYING) {
        need_copy = 1U;
    }
    else if (app2_state == IOS_READY) {
        if ((app1_state != IOS_READY) || (app1_version != app2_version)) {
            need_copy = 1U;
        }
    }
    
    /* 如果检测到新固件，则复制更新 */
    if (need_copy) {
        /* 更新固件信息，并设置为copy状态 */
        HAL_FLASH_Unlock();
        FLASH_EraseInitTypeDef erase_init;
        erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
        erase_init.NbPages = 1;
        erase_init.Banks = FLASH_BANK_1;
        erase_init.PageAddress = APP1_VERSION_ADDR;
        uint32_t page_error = 0;
        HAL_FLASHEx_Erase(&erase_init, &page_error);
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP1_VERSION_ADDR, app2_version);
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP1_IOS_STATE, IOS_COPYING);
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP1_IOS_SIZE, app2_size);
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP2_VERSION_ADDR, app2_version);
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP2_IOS_STATE, IOS_COPYING);
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP2_IOS_SIZE, app2_size);
        
        /* 擦除原有固件 */
        for (uint8_t i = 0; i < min((app2_size / 1024 + 1), 29); i ++) {
            for (uint16_t j = 0; j < 1024; j ++) {
                /* 如果原本有数据则擦除 */
                if (*(volatile uint8_t *)(APP1_PROGRAM_ADDR + i * 1024 + j) != 0xFF) {
                
                    erase_init.PageAddress = APP1_PROGRAM_ADDR + i * 1024;
                    HAL_FLASHEx_Erase(&erase_init, &page_error);
                
                    break;
                }
            }
        }
        
        /* 复制固件信息 */
        for (uint16_t i = 0; i < app2_size / 2 + app2_size % 2; i++) {
            write_data = *(volatile uint16_t *)(APP2_PROGRAM_ADDR + i * 2);
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (APP1_PROGRAM_ADDR + i * 2), write_data);
        }
        
        /* 检查复制是否成功 */
        uint8_t error_flag = 0;
        
        for (uint16_t i = 0; i < app2_size / 2 + app2_size % 2; i++) {
            if (*(volatile uint16_t *)(APP1_PROGRAM_ADDR + i * 2) != *(volatile uint16_t *)(APP2_PROGRAM_ADDR + i * 2)) {
                error_flag = 1;
            }
        }
        
        /* 更新固件信息 */
        erase_init.PageAddress = APP1_VERSION_ADDR;
        HAL_FLASHEx_Erase(&erase_init, &page_error);
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP1_VERSION_ADDR, app2_version);
        if (error_flag == 0) {
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP1_IOS_STATE, IOS_READY);
        }
        else {
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP1_IOS_STATE, IOS_BROKEN);
        }
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP1_IOS_SIZE, app2_size);
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP2_VERSION_ADDR, app2_version);
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP2_IOS_STATE, IOS_READY);
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP2_IOS_SIZE, app2_size);
        
        HAL_FLASH_Lock();
        
        if (error_flag == 0) {
            jumpToProgram(APP1_PROGRAM_ADDR);
        }
    }
    /* 没有则直接跳转app1运行 */
    else {
        if (app1_state == IOS_READY) {
            jumpToProgram(APP1_PROGRAM_ADDR);
        }
    }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
uint8_t min(uint8_t data1, uint8_t data2) {
    return data1 <= data2?data1:data2;
}

void jumpToProgram(uint32_t program_addr) {
    typedef void (*pFunc)(void);
    
    uint32_t app_stack;
    uint32_t app_reset;
    pFunc app_entry;

    app_stack = *(volatile uint32_t *)program_addr;
    app_reset = *(volatile uint32_t *)(program_addr + 4U);

    app_entry = (pFunc)app_reset;
    
    /* 禁止全局中断 */
    __disable_irq();

    /* 停止SysTick */
    SysTick->CTRL = 0U;
    SysTick->LOAD = 0U;
    SysTick->VAL  = 0U;
    
    /* 清除NVIC中断使能和Pending状态 */
    for (uint32_t i = 0; i < 8U; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFFU;
        NVIC->ICPR[i] = 0xFFFFFFFFU;
    }
    
    
    HAL_DeInit();
    
    /* 设置中断向量地址 */
    SCB->VTOR = program_addr;

    __DSB();
    __ISB();
    
    /* 设置主堆栈指针 */
    __set_MSP(app_stack);
    
    __DSB();
    __ISB();
    
    __enable_irq();
    
    app_entry();
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
