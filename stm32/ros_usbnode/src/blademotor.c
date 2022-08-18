/****************************************************************************
* Title                 :   
* Filename              :   blademotor.c
* Author                :   Nekraus
* Origin Date           :   17/08/2022
* Version               :   1.0.0

*****************************************************************************/
/** \file blademotor.c
*  \brief 
*
*/
/******************************************************************************
* Includes
*******************************************************************************/
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"
#include "main.h"

#include "board.h"

#include "blademotor.h" 

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/
#define LENGTH_INIT_MSG 22
#define LENGTH_RQST_MSG 7
/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/

/******************************************************************************
* Module Typedefs
*******************************************************************************/
typedef enum {
    BLADEMOTOR_INIT_1,
    BLADEMOTOR_INIT_2,
    BLADEMOTOR_RUN
}BLADEMOTOR_STATE_e;

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
UART_HandleTypeDef BLADEMOTOR_USART_Handler; // UART  Handle

DMA_HandleTypeDef hdma_uart3_rx;
DMA_HandleTypeDef hdma_uart3_tx;

BLADEMOTOR_STATE_e blademotor_eState = BLADEMOTOR_INIT_1;


uint8_t blademotor_pu8ReceivedData[10] = {0};
uint8_t blademotor_pu8RqstMessage[LENGTH_RQST_MSG]  = {0x55, 0xaa, 0x03, 0x20, 0x80, 0x00, 0xAA};

const uint8_t blademotor_pcu8InitMsg[LENGTH_INIT_MSG] =  { 0x55, 0xaa, 0x12, 0x20, 0x80, 0x00, 0xac, 0x0d, 0x00, 0x02, 0x32, 0x50, 0x1e, 0x04, 0x00, 0x15, 0x21, 0x05, 0x0a, 0x19, 0x3c, 0xaa };
/******************************************************************************
* Function Prototypes
*******************************************************************************/

/******************************************************************************
*  Public Functions
*******************************************************************************/

/**
 * @brief Init the Blade Motor Serial Port (PAC5223)
 * @retval None
 */
void BLADEMOTOR_Init()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* PAC 5223 Reset Line (Blade Motor) */
    PAC5223RESET_GPIO_CLK_ENABLE();
    GPIO_InitStruct.Pin = PAC5223RESET_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(PAC5223RESET_GPIO_PORT, &GPIO_InitStruct);

    // enable port and usart clocks
    BLADEMOTOR_USART_GPIO_CLK_ENABLE();
    BLADEMOTOR_USART_USART_CLK_ENABLE();
    
    // RX
    GPIO_InitStruct.Pin = BLADEMOTOR_USART_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(BLADEMOTOR_USART_RX_PORT, &GPIO_InitStruct);

    // TX
    GPIO_InitStruct.Pin = BLADEMOTOR_USART_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(BLADEMOTOR_USART_TX_PORT, &GPIO_InitStruct);

    BLADEMOTOR_USART_Handler.Instance = BLADEMOTOR_USART_INSTANCE;// USART3
    BLADEMOTOR_USART_Handler.Init.BaudRate = 115200;               // Baud rate
    BLADEMOTOR_USART_Handler.Init.WordLength = UART_WORDLENGTH_8B; // The word is  8  Bit format
    BLADEMOTOR_USART_Handler.Init.StopBits = USART_STOPBITS_1;     // A stop bit
    BLADEMOTOR_USART_Handler.Init.Parity = UART_PARITY_NONE;       // No parity bit
    BLADEMOTOR_USART_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE; // No hardware flow control
    BLADEMOTOR_USART_Handler.Init.Mode = USART_MODE_TX_RX;         // Transceiver mode
    
    HAL_UART_Init(&BLADEMOTOR_USART_Handler); 

    // enable IRQ
    HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART3_IRQn);     
    __HAL_UART_ENABLE_IT(&BLADEMOTOR_USART_Handler, UART_IT_TC);


    /* UART4 DMA Init */
    /* UART4_RX Init */    
    hdma_uart3_rx.Instance = DMA1_Channel3;
    hdma_uart3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_uart3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart3_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart3_rx.Init.Mode = DMA_NORMAL;
    hdma_uart3_rx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_uart3_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(&BLADEMOTOR_USART_Handler,hdmarx,hdma_uart3_rx);
    
    /* UART4 DMA Init */
    /* UART4_TX Init */
    hdma_uart3_tx.Instance = DMA1_Channel2;
    hdma_uart3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_uart3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart3_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart3_tx.Init.Mode = DMA_NORMAL;
    hdma_uart3_tx.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&hdma_uart3_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(&BLADEMOTOR_USART_Handler,hdmatx,hdma_uart3_tx);

    blademotor_eState = BLADEMOTOR_INIT_1;
}

void  BLADEMOTOR_App(void){

    switch (blademotor_eState)
    {
    case BLADEMOTOR_INIT_1:
        HAL_GPIO_WritePin(PAC5223RESET_GPIO_PORT, PAC5223RESET_PIN, 1);     // take Blade PAC out of reset if HIGH
        HAL_UART_Transmit_DMA(&BLADEMOTOR_USART_Handler, (uint8_t*)blademotor_pcu8InitMsg, LENGTH_INIT_MSG);
        blademotor_eState = BLADEMOTOR_RUN;

        DB_TRACE(" * Blade Motor initialized\r\n");
     
        break;
    
    case BLADEMOTOR_RUN:
        
        /* prepare to receive the message before to lauch the command */
        HAL_UART_Receive_DMA(&BLADEMOTOR_USART_Handler,blademotor_pu8ReceivedData,10);
        HAL_UART_Transmit_DMA(&BLADEMOTOR_USART_Handler, (uint8_t*)blademotor_pu8RqstMessage, LENGTH_RQST_MSG);

        break;
    
    default:
        break;
    }
}
/*
 * Activate or not the Blade motor
 *
 * <on_off> - no speed settings available
 */
void BLADEMOTOR_Set(uint8_t on_off)
{
    if (on_off)
    {
        blademotor_pu8RqstMessage[5] = 0x80;
    }
    else
    {
        blademotor_pu8RqstMessage[5] = 0x00;
    }
}


void ULTRASONICSENSOR_ReceiceIT(void)
{
    /* decode the frame */
    if(memcmp(ultrasonic_PreAmbule,ultrasonic_pu8ReceivedData,5) == 0){
        /* todo calculate the CRC */
        ultrasonic_u32LeftDistance = (ultrasonic_pu8ReceivedData[5] << 8) + ultrasonic_pu8ReceivedData[6];
        ultrasonic_u32RightDistance  = (ultrasonic_pu8ReceivedData[7] << 8) + ultrasonic_pu8ReceivedData[8];

        debug_printf(" R: %dmm, L: %dmm \r\n",ultrasonic_u32RightDistance/10,ultrasonic_u32LeftDistance/10);
  
    }
}

/******************************************************************************
*  Private Functions
*******************************************************************************/