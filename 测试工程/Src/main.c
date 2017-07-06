/* LL drivers common to all LL examples */
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_exti.h"
#include "stm32l4xx_ll_gpio.h"
/* LL drivers specific to LL examples IPs */
#include "stm32l4xx_ll_adc.h"
#include "stm32l4xx_ll_comp.h"
#include "stm32l4xx_ll_cortex.h"
#include "stm32l4xx_ll_crc.h"
#include "stm32l4xx_ll_dac.h"
#include "stm32l4xx_ll_dma.h"
#include "stm32l4xx_ll_i2c.h"
#include "stm32l4xx_ll_iwdg.h"
#include "stm32l4xx_ll_lptim.h"
#include "stm32l4xx_ll_lpuart.h"
#include "stm32l4xx_ll_opamp.h"
#include "stm32l4xx_ll_rng.h"
#include "stm32l4xx_ll_rtc.h"
#include "stm32l4xx_ll_spi.h"
#include "stm32l4xx_ll_swpmi.h"
#include "stm32l4xx_ll_tim.h"
#include "stm32l4xx_ll_usart.h"
#include "stm32l4xx_ll_wwdg.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

#include "N25Q128.h"

#include "WL_Flash.h"

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

    LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);

    if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_4)
    {

    }
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);

    LL_RCC_MSI_Enable();

    /* Wait till MSI is ready */
    while(LL_RCC_MSI_IsReady() != 1)
    {

    }
    LL_RCC_MSI_EnableRangeSelection();

    LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_6);

    LL_RCC_MSI_SetCalibTrimming(0);

    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_MSI, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2);

    LL_RCC_PLL_EnableDomain_SYS();

    LL_RCC_PLL_Enable();

    /* Wait till PLL is ready */
    while(LL_RCC_PLL_IsReady() != 1)
    {

    }
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    /* Wait till System clock is ready */
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
    {

    }

    LL_SetSystemCoreClock(80000000);
}

wl_flash_t MWL_Flash;
uint8_t pBuf[12 * 1024];
uint8_t aBuf[12 * 1024];


void MWL_Main(void)
{
    BSP_QSPI_Init();

    MWL_Flash.cfg.start_addr = 0x00000000;
    MWL_Flash.cfg.full_mem_size = 0x01000000;
    MWL_Flash.cfg.page_size = 0x00001000;
    MWL_Flash.cfg.sector_size = 0x00001000;
    MWL_Flash.cfg.wr_size = 0x00000010;
    MWL_Flash.cfg.version = 0x00000001;
    MWL_Flash.cfg.temp_buff_size = 0x00000020;

    WL_Flash_Config(&MWL_Flash);

		for(uint16_t i = 0;i<5*1024;i++){
			pBuf[i] = 0xFF & i;
		}

    for(;;)
    {
        pBuf[0] = pBuf[0] + 5;
        vTaskDelay(100);
        WL_Flash_Erase_Range(&MWL_Flash, 0x00000000, 12 * 1024);
        vTaskDelay(100);
        WL_Flash_Write(&MWL_Flash, 0x00000000, pBuf, 12 * 1024);
        vTaskDelay(100);
        WL_Flash_Read(&MWL_Flash, 0x00000000, aBuf, 12 * 1024);
    }
}

int main(void)
{
    SystemClock_Config();
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_CRC);

    xTaskCreate((TaskFunction_t)MWL_Main, "MWL_Main", 128, NULL, 0, NULL);
    vTaskStartScheduler();
    while (1)
    {
    }

}


