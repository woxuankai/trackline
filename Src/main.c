/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "dcmi.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "ov7725.h"
#include "processImg.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */








struct IMG_frame{
#if (int!=int32_t)
	#warning "请根据系统位数手动对齐ImgData" 
#endif	
	uint8_t dummy1;
	uint8_t dummy2;//为了对齐用

	volatile uint8_t FrameState;
	uint8_t head;
	uint8_t ImgData[IMG_HEIGHT][IMG_WIDTH];//head和ImGdata之间必须连续，发送图像需要

} __attribute__ ((aligned (4)));

struct IMG_frame img_frame;
#if (int!=int32_t)
	#warning "请根据系统位数手动对齐ImgData" 
#endif

uint8_t ImgRawData[IMG_HEIGHT][IMG_WIDTH][2] __attribute__ ((aligned (4)));


uint32_t missedframe = 0;//因为处理不级而错过的帧数
uint32_t missedfps = 0;




/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
enum FrameState{
	FrameNotRefreshed = 0,
	FrameRefreshed = 1,
	FrameDMASyncError = 2
};

uint32_t FrameDMASyncError_cnt = 0;
uint32_t FrameDMASyncError_NDTR;
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	FrameDMASyncError_NDTR =  hdcmi->DMA_Handle->Instance->NDTR;
	if(hdcmi->DMA_Handle->Instance->NDTR != IMG_WIDTH*IMG_HEIGHT*2/4)
	{

		FrameDMASyncError_cnt ++;
		img_frame.FrameState = FrameDMASyncError;
	}else if(img_frame.FrameState	== FrameNotRefreshed)//数据没有更新
	{
		starttime = HAL_GetTick();
		//复制缓冲区图像
		for(uint32_t ii = 0; ii< IMG_HEIGHT ; ii++)
			for(uint32_t jj = 0; jj< IMG_WIDTH ; jj++)
				img_frame.ImgData[ii][jj] = ImgRawData[ii][jj][0];
		img_frame.FrameState = FrameRefreshed;
	}
	else
	{
		missedframe++;
	}
	
	HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_12);
}



void HAL_SYSTICK_Callback(void)
{
//	static uint32_t missedframe_lastsecond = 0;
//	if(HAL_GetTick()%1000 == 0)//到了整数的毫秒处
//	{
//		missedfps = missedframe - missedframe_lastsecond;
//		missedframe_lastsecond = missedframe;
//	}
}
/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
	img_frame.head = 0xff;
	img_frame.FrameState = FrameNotRefreshed;
	
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_DCMI_Init();
  MX_UART4_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();

  /* USER CODE BEGIN 2 */
	
	if(sccb_Init())
		while(1)
			;
	if(ov7725_Init())
		while(1)
			;

		
  __HAL_DCMI_DISABLE_IT(&hdcmi, DCMI_IT_LINE);
  __HAL_DCMI_DISABLE_IT(&hdcmi, DCMI_IT_VSYNC);
 
		
	__HAL_DCMI_DISABLE(&hdcmi);

	HAL_DCMI_ConfigCROP(&hdcmi,0,0,IMG_WIDTH*2-1,IMG_HEIGHT-1);
	HAL_DCMI_EnableCROP(&hdcmi);
	__HAL_DCMI_ENABLE(&hdcmi);
		
	HAL_DCMI_Start_DMA(&hdcmi,DCMI_MODE_CONTINUOUS,(uint32_t)ImgRawData,IMG_WIDTH*IMG_HEIGHT*2/4);
//	HAL_UART_Transmit_DMA(&huart4,(uint8_t*)&(img_frame.head),IMG_WIDTH*IMG_HEIGHT+1);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

		if(img_frame.FrameState == FrameDMASyncError)
		{
//			HAL_DCMI_Stop(&hdcmi);不能调用此函数，CR的bit14为使能，在INIT中置位，不在HAL_DCMI_Start中置位，所以停止后便无法再次开启
			HAL_DMA_Abort(hdcmi.DMA_Handle);
			HAL_DCMI_Start_DMA(&hdcmi,DCMI_MODE_CONTINUOUS,(uint32_t)ImgRawData,IMG_WIDTH*IMG_HEIGHT*2/4);
			img_frame.FrameState = FrameNotRefreshed;
		}
		else if(img_frame.FrameState == FrameRefreshed)
		{	
			//ProcessFrame Begin
			
			
			processImg(img_frame.ImgData);

			//ProcessFrame End
			img_frame.FrameState = FrameNotRefreshed;
		}
	}
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  __PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 24;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
