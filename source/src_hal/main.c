/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    OpenAMP/OpenAMP_TTY_echo/Inc/main.c
  * @author  MCD Application Team
  * @brief   Main program body.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics. 
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
#include "stm32mp15xx_disco.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_BUFFER_SIZE RPMSG_BUFFER_SIZE
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
IPCC_HandleTypeDef hipcc;

enum { PREAMBLE=0xABCD, };
#pragma pack(1)
struct packet {
  uint16_t preamble;
  uint16_t length;
  uint16_t crc16;
};

typedef void (*VIRT_UART_RxCpltCallback)(VIRT_UART_HandleTypeDef *huart);

void VIRT_UART0_RxCpltCallback(VIRT_UART_HandleTypeDef *huart);
void VIRT_UART1_RxCpltCallback(VIRT_UART_HandleTypeDef *huart);


// uint8_t VirtUart0ChannelBuffRx[MAX_BUFFER_SIZE];
uint8_t VirtUart0ChannelBuffTx[MAX_BUFFER_SIZE];
// uint8_t VirtUart1ChannelBuffRx[MAX_BUFFER_SIZE];
uint8_t VirtUart1ChannelBuffTx[MAX_BUFFER_SIZE];

struct virt_uart {
  VIRT_UART_HandleTypeDef huart;
  __IO FlagStatus rx_status;
  __IO uint8_t *rx_buffer;
  __IO uint16_t rx_size;
  __IO FlagStatus tx_status;
  __IO uint8_t *tx_buffer;
  __IO uint16_t tx_size;
  VIRT_UART_RxCpltCallback cbk;
};

struct virt_uart virt_uart0 = {
  .rx_status = RESET,
  .rx_buffer = NULL,
  .rx_size = 0,
  .tx_status = RESET,
  .tx_buffer = VirtUart0ChannelBuffTx,
  .tx_size = 0,
  .cbk = VIRT_UART0_RxCpltCallback,
};
__IO uint16_t virt_uart0_expected_nbytes = 0;

struct virt_uart virt_uart1 = {
  .rx_status = RESET,
  .rx_buffer = NULL,
  .rx_size = 0,
  .tx_status = RESET,
  .tx_buffer = VirtUart1ChannelBuffTx,
  .tx_size = 0,
  .cbk = VIRT_UART1_RxCpltCallback,
};
__IO uint16_t virt_uart1_expected_nbytes = 0;

/* USER CODE BEGIN PV */


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_IPCC_Init(void);
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

  /* Reset of all peripherals, Initialize the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
    if(IS_ENGINEERING_BOOT_MODE())
  {
    /* Configure the system clock */
    SystemClock_Config();
  }
  /* USER CODE END Init */

  /*HW semaphore Clock enable*/
  __HAL_RCC_HSEM_CLK_ENABLE();
  /* IPCC initialisation */
   MX_IPCC_Init();
  /* OpenAmp initialisation ---------------------------------*/
  MX_OPENAMP_Init(RPMSG_REMOTE, NULL);

  COM_InitTypeDef uart_init;
  uart_init.BaudRate = 115200;
  uart_init.Parity = UART_PARITY_NONE;
  uart_init.StopBits = UART_STOPBITS_1;
  uart_init.WordLength = UART_WORDLENGTH_8B;
  uart_init.HwFlowCtl = UART_HWCONTROL_NONE;
  BSP_COM_Init(COM2, &uart_init);
  BSP_COM_SelectLogPort(COM2);

  log_info("Cortex-M4 boot successful with STM32Cube FW version: v%ld.%ld.%ld \r\n",
                                            ((HAL_GetHalVersion() >> 24) & 0x000000FF),
                                            ((HAL_GetHalVersion() >> 16) & 0x000000FF),
                                            ((HAL_GetHalVersion() >> 8) & 0x000000FF));

  log_info("MAX_BUFFER_SIZE: %d\n", MAX_BUFFER_SIZE);
  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */

  /* USER CODE BEGIN 2 */
  /*
   * Create Virtual UART device
   * defined by a rpmsg channel attached to the remote device
   */
  log_info("Virtual UART0 OpenAMP-rpmsg channel creation\r\n");
  if (VIRT_UART_Init(&virt_uart0.huart) != VIRT_UART_OK) {
    log_err("VIRT_UART_Init UART0 failed.\r\n");
    Error_Handler();
  }

  log_info("Virtual UART1 OpenAMP-rpmsg channel creation\r\n");
  if (VIRT_UART_Init(&virt_uart1.huart) != VIRT_UART_OK) {
    log_err("VIRT_UART_Init UART1 failed.\r\n");
    Error_Handler();
  }

  /*Need to register callback for message reception by channels*/
  if(VIRT_UART_RegisterCallback(&virt_uart0.huart, VIRT_UART_RXCPLT_CB_ID, virt_uart0.cbk) != VIRT_UART_OK)
  {
   Error_Handler();
  }
  if(VIRT_UART_RegisterCallback(&virt_uart1.huart, VIRT_UART_RXCPLT_CB_ID, virt_uart1.cbk) != VIRT_UART_OK)
  {
    Error_Handler();
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    OPENAMP_check_for_message();

    /* USER CODE END WHILE */
    if (virt_uart0.tx_status) {
      virt_uart0.tx_status = RESET;
      VIRT_UART_Transmit(&virt_uart0.huart, (uint8_t*) virt_uart0.tx_buffer, virt_uart0.tx_size);
    }

    if (virt_uart1.tx_status) {
      virt_uart1.tx_status = RESET;
      VIRT_UART_Transmit(&virt_uart1.huart, (uint8_t*) virt_uart1.tx_buffer, virt_uart1.tx_size);
    }
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

  /**Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMHIGH);

  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE
                |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS_DIG;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.HSIDivValue = RCC_HSI_DIV1;

  /**PLL1 Config
  */
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLL12SOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 3;
  RCC_OscInitStruct.PLL.PLLN = 81;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 1;
  RCC_OscInitStruct.PLL.PLLR = 1;
  RCC_OscInitStruct.PLL.PLLFRACV = 0x800;
  RCC_OscInitStruct.PLL.PLLMODE = RCC_PLL_FRACTIONAL;
  RCC_OscInitStruct.PLL.RPDFN_DIS = RCC_RPDFN_DIS_DISABLED;
  RCC_OscInitStruct.PLL.TPDFN_DIS = RCC_TPDFN_DIS_DISABLED;

    /**PLL2 Config
    */
  RCC_OscInitStruct.PLL2.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL2.PLLSource = RCC_PLL12SOURCE_HSE;
  RCC_OscInitStruct.PLL2.PLLM = 3;
  RCC_OscInitStruct.PLL2.PLLN = 66;
  RCC_OscInitStruct.PLL2.PLLP = 2;
  RCC_OscInitStruct.PLL2.PLLQ = 1;
  RCC_OscInitStruct.PLL2.PLLR = 1;
  RCC_OscInitStruct.PLL2.PLLFRACV = 0x1400;
  RCC_OscInitStruct.PLL2.PLLMODE = RCC_PLL_FRACTIONAL;
  RCC_OscInitStruct.PLL2.RPDFN_DIS = RCC_RPDFN_DIS_DISABLED;
  RCC_OscInitStruct.PLL2.TPDFN_DIS = RCC_TPDFN_DIS_DISABLED;

    /**PLL3 Config
    */
  RCC_OscInitStruct.PLL3.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL3.PLLSource = RCC_PLL3SOURCE_HSE;
  RCC_OscInitStruct.PLL3.PLLM = 2;
  RCC_OscInitStruct.PLL3.PLLN = 34;
  RCC_OscInitStruct.PLL3.PLLP = 2;
  RCC_OscInitStruct.PLL3.PLLQ = 17;
  RCC_OscInitStruct.PLL3.PLLR = 37;
  RCC_OscInitStruct.PLL3.PLLRGE = RCC_PLL3IFRANGE_1;
  RCC_OscInitStruct.PLL3.PLLFRACV = 0x1A04;
  RCC_OscInitStruct.PLL3.PLLMODE = RCC_PLL_FRACTIONAL;
  RCC_OscInitStruct.PLL3.RPDFN_DIS = RCC_RPDFN_DIS_DISABLED;
  RCC_OscInitStruct.PLL3.TPDFN_DIS = RCC_TPDFN_DIS_DISABLED;

    /**PLL4 Config
    */
  RCC_OscInitStruct.PLL4.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL4.PLLSource = RCC_PLL4SOURCE_HSE;
  RCC_OscInitStruct.PLL4.PLLM = 4;
  RCC_OscInitStruct.PLL4.PLLN = 99;
  RCC_OscInitStruct.PLL4.PLLP = 6;
  RCC_OscInitStruct.PLL4.PLLQ = 8;
  RCC_OscInitStruct.PLL4.PLLR = 8;
  RCC_OscInitStruct.PLL4.PLLRGE = RCC_PLL4IFRANGE_0;
  RCC_OscInitStruct.PLL4.PLLFRACV = 0;
  RCC_OscInitStruct.PLL4.PLLMODE = RCC_PLL_INTEGER;
  RCC_OscInitStruct.PLL4.RPDFN_DIS = RCC_RPDFN_DIS_DISABLED;
  RCC_OscInitStruct.PLL4.TPDFN_DIS = RCC_TPDFN_DIS_DISABLED;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
  Error_Handler();
  }
  /**RCC Clock Config
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_ACLK
                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                |RCC_CLOCKTYPE_PCLK3|RCC_CLOCKTYPE_PCLK4
                |RCC_CLOCKTYPE_PCLK5|RCC_CLOCKTYPE_MPU;
  RCC_ClkInitStruct.MPUInit.MPU_Clock = RCC_MPUSOURCE_PLL1;
  RCC_ClkInitStruct.MPUInit.MPU_Div = RCC_MPU_DIV2;
  RCC_ClkInitStruct.AXISSInit.AXI_Clock = RCC_AXISSOURCE_PLL2;
  RCC_ClkInitStruct.AXISSInit.AXI_Div = RCC_AXI_DIV1;
  RCC_ClkInitStruct.MCUInit.MCU_Clock = RCC_MCUSSOURCE_PLL3;
  RCC_ClkInitStruct.MCUInit.MCU_Div = RCC_MCU_DIV1;
  RCC_ClkInitStruct.APB4_Div = RCC_APB4_DIV2;
  RCC_ClkInitStruct.APB5_Div = RCC_APB5_DIV4;
  RCC_ClkInitStruct.APB1_Div = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2_Div = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB3_Div = RCC_APB3_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct) != HAL_OK)
  {
  Error_Handler();
  }

  /**Set the HSE division factor for RTC clock
  */
  __HAL_RCC_RTC_HSEDIV(24);
}

/**
  * @brief IPPC Initialization Function
  * @param None
  * @retval None
  */
static void MX_IPCC_Init(void)
{

  hipcc.Instance = IPCC;
  if (HAL_IPCC_Init(&hipcc) != HAL_OK)
  {
     Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void VIRT_UART0_RxCpltCallback(VIRT_UART_HandleTypeDef *huart)
{

  /* copy received msg in a variable to sent it back to master processor in main infinite loop*/
  uint16_t recv_size = huart->RxXferSize < MAX_BUFFER_SIZE? huart->RxXferSize : MAX_BUFFER_SIZE-1;

  struct packet* in = (struct packet*) &huart->pRxBuffPtr[0];
  if (in->preamble == PREAMBLE) {
    in->preamble = 0;
    virt_uart0_expected_nbytes = in->length;
    log_info("length: %d\n", virt_uart0_expected_nbytes);                        
  }

  virt_uart0.rx_size += recv_size;
  log_info("UART0: %d/%d\n", virt_uart0.rx_size, virt_uart0_expected_nbytes);
  if (virt_uart0.rx_size >= virt_uart0_expected_nbytes) {
    virt_uart0.rx_size = 0;
    virt_uart0.tx_buffer[0] = virt_uart0_expected_nbytes & 0xff;
    virt_uart0.tx_buffer[1] = (virt_uart0_expected_nbytes >> 8) & 0xff;
    log_info("UART0 resp: %d\n", virt_uart0_expected_nbytes);
    virt_uart0_expected_nbytes = 0;
    virt_uart0.tx_size = 2;
    virt_uart0.tx_status = SET;
    // huart->RxXferSize = 0;
  }
}

void VIRT_UART1_RxCpltCallback(VIRT_UART_HandleTypeDef *huart)
{

  /* copy received msg in a variable to sent it back to master processor in main infinite loop*/
  uint16_t recv_size = huart->RxXferSize < MAX_BUFFER_SIZE? huart->RxXferSize : MAX_BUFFER_SIZE-1;

  struct packet* in = (struct packet*) &huart->pRxBuffPtr[0];
  if (in->preamble == PREAMBLE) {
    
  }

  virt_uart1.rx_size += recv_size;
  log_info("UART0: %d/%d\n", recv_size, virt_uart1.rx_size);
  if (virt_uart1.rx_size >= 512) {
    virt_uart1.tx_buffer[1] = (virt_uart1.rx_size >> 8) & 0xff;
    virt_uart1.tx_buffer[0] = virt_uart1.rx_size & 0xff;
    virt_uart1.rx_size = 0;
    virt_uart1.tx_size = 2;
    virt_uart1.tx_status = SET;
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  log_err("Error_Handler");
  while(1)
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
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  log_err("OOOps: file %s, line %d\r\n", __FILE__, __LINE__);
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
