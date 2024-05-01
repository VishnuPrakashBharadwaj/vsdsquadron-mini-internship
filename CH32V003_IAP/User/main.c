// Importing header files
#include "debug.h"
#include "string.h"
#include "iap.h"

// Function which jumps from BOOT area (1920B) to USER area (16K)
void IAP_2_APP(void)
{
    RCC_ClearFlag();                        // Clearing all the reset and clock control flags
    SystemReset_StartMode(Start_Mode_USER); // Setting the Start mode to USER
    NVIC_SystemReset();                     // Performing a System reset
}


int main(void)
{
    // Enable GPIOD,USART1 clock
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOD| RCC_APB2Periph_USART1;

    // Setting the UART baud rate to 38400
    USART1_CFG(38400);

    // A temporary count variable used for creating delay
    u32 count = 0;

    // Loop
    while(1)
    {
        // If no data is recieved through UART, keep on counting
        while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != SET){
            count++;
            // If the count exceeds this threshold, then the execution will be redirected to USER area
            if(count >= 3000000){
                IAP_2_APP();
                while(1);
            }
        }

        // If the above loop terminates, it means data is recieved through UART
        count = 0;          // Resetting the counter
        UART_Rx_Deal();     // Deal with the incoming data
    }
}
