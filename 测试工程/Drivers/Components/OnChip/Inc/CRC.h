#ifndef __STM32L4xx_UCRC_H
#define __STM32L4xx_UCRC_H

#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_crc.h"

uint32_t Calculate_CRC(uint8_t *pBuf,uint32_t BufferSize);

#endif
