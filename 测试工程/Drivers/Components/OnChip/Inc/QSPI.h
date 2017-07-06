#ifndef __STM32L4xx_QSPI_H
#define __STM32L4xx_QSPI_H

/* Includes ------------------------------------------------------------------*/
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


/** 
  * @brief  QSPI Command structure definition  
  */
typedef struct
{
  uint32_t Instruction;        /* Specifies the Instruction to be sent
                                  This parameter can be a value (8-bit) between 0x00 and 0xFF */
  uint32_t Address;            /* Specifies the Address to be sent (Size from 1 to 4 bytes according AddressSize)
                                  This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF */
  uint32_t AlternateBytes;     /* Specifies the Alternate Bytes to be sent (Size from 1 to 4 bytes according AlternateBytesSize)
                                  This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF */
  uint32_t AddressSize;        /* Specifies the Address Size
                                  This parameter can be a value of @ref QSPI_AddressSize */
  uint32_t AlternateBytesSize; /* Specifies the Alternate Bytes Size
                                  This parameter can be a value of @ref QSPI_AlternateBytesSize */
  uint32_t DummyCycles;        /* Specifies the Number of Dummy Cycles.
                                  This parameter can be a number between 0 and 31 */
  uint32_t InstructionMode;    /* Specifies the Instruction Mode
                                  This parameter can be a value of @ref QSPI_InstructionMode */
  uint32_t AddressMode;        /* Specifies the Address Mode
                                  This parameter can be a value of @ref QSPI_AddressMode */
  uint32_t AlternateByteMode;  /* Specifies the Alternate Bytes Mode
                                  This parameter can be a value of @ref QSPI_AlternateBytesMode */
  uint32_t DataMode;           /* Specifies the Data Mode (used for dummy cycles and data phases)
                                  This parameter can be a value of @ref QSPI_DataMode */
  uint32_t NbData;             /* Specifies the number of data to transfer. (This is the number of bytes)
                                  This parameter can be any value between 0 and 0xFFFFFFFF (0 means undefined length 
                                  until end of memory)*/
  uint32_t DdrMode;            /* Specifies the double data rate mode for address, alternate byte and data phase
                                  This parameter can be a value of @ref QSPI_DdrMode */
  uint32_t DdrHoldHalfCycle;   /* Specifies the DDR hold half cycle. It delays the data output by one half of 
                                  system clock in DDR mode. Not available on STM32L4x6 devices but in future devices.
                                  This parameter can be a value of @ref QSPI_DdrHoldHalfCycle */
  uint32_t SIOOMode;           /* Specifies the send instruction only once mode
                                  This parameter can be a value of @ref QSPI_SIOOMode */
}QSPI_CommandTypeDef;

/** 
  * @brief  QSPI Auto Polling mode configuration structure definition  
  */
typedef struct
{
  uint32_t Match;              /* Specifies the value to be compared with the masked status register to get a match.
                                  This parameter can be any value between 0 and 0xFFFFFFFF */
  uint32_t Mask;               /* Specifies the mask to be applied to the status bytes received. 
                                  This parameter can be any value between 0 and 0xFFFFFFFF */
  uint32_t Interval;           /* Specifies the number of clock cycles between two read during automatic polling phases.
                                  This parameter can be any value between 0 and 0xFFFF */
  uint32_t StatusBytesSize;    /* Specifies the size of the status bytes received.
                                  This parameter can be any value between 1 and 4 */
  uint32_t MatchMode;          /* Specifies the method used for determining a match.
                                  This parameter can be a value of @ref QSPI_MatchMode */
  uint32_t AutomaticStop;      /* Specifies if automatic polling is stopped after a match.
                                  This parameter can be a value of @ref QSPI_AutomaticStop */
}QSPI_AutoPollingTypeDef;
                           
/** 
  * @brief  QSPI Memory Mapped mode configuration structure definition  
  */
typedef struct
{
  uint32_t TimeOutPeriod;      /* Specifies the number of clock to wait when the FIFO is full before to release the chip select.
                                  This parameter can be any value between 0 and 0xFFFF */
  uint32_t TimeOutActivation;  /* Specifies if the timeout counter is enabled to release the chip select. 
                                  This parameter can be a value of @ref QSPI_TimeOutActivation */
}QSPI_MemoryMappedTypeDef;

/* Exported constants --------------------------------------------------------*/
/** @defgroup QSPI_Exported_Constants QSPI Exported Constants
  * @{
  */


/** @defgroup QSPI_SampleShifting QSPI Sample Shifting
  * @{
  */
#define QSPI_SAMPLE_SHIFTING_NONE      ((uint32_t)0x00000000)        /*!<No clock cycle shift to sample data*/
#define QSPI_SAMPLE_SHIFTING_HALFCYCLE ((uint32_t)QUADSPI_CR_SSHIFT) /*!<1/2 clock cycle shift to sample data*/
/**
  * @}
  */ 

/** @defgroup QSPI_ChipSelectHighTime QSPI ChipSelect High Time
  * @{
  */
#define QSPI_CS_HIGH_TIME_1_CYCLE      ((uint32_t)0x00000000)                              /*!<nCS stay high for at least 1 clock cycle between commands*/
#define QSPI_CS_HIGH_TIME_2_CYCLE      ((uint32_t)QUADSPI_DCR_CSHT_0)                      /*!<nCS stay high for at least 2 clock cycles between commands*/
#define QSPI_CS_HIGH_TIME_3_CYCLE      ((uint32_t)QUADSPI_DCR_CSHT_1)                      /*!<nCS stay high for at least 3 clock cycles between commands*/
#define QSPI_CS_HIGH_TIME_4_CYCLE      ((uint32_t)QUADSPI_DCR_CSHT_0 | QUADSPI_DCR_CSHT_1) /*!<nCS stay high for at least 4 clock cycles between commands*/
#define QSPI_CS_HIGH_TIME_5_CYCLE      ((uint32_t)QUADSPI_DCR_CSHT_2)                      /*!<nCS stay high for at least 5 clock cycles between commands*/
#define QSPI_CS_HIGH_TIME_6_CYCLE      ((uint32_t)QUADSPI_DCR_CSHT_2 | QUADSPI_DCR_CSHT_0) /*!<nCS stay high for at least 6 clock cycles between commands*/
#define QSPI_CS_HIGH_TIME_7_CYCLE      ((uint32_t)QUADSPI_DCR_CSHT_2 | QUADSPI_DCR_CSHT_1) /*!<nCS stay high for at least 7 clock cycles between commands*/
#define QSPI_CS_HIGH_TIME_8_CYCLE      ((uint32_t)QUADSPI_DCR_CSHT)                        /*!<nCS stay high for at least 8 clock cycles between commands*/
/**
  * @}
  */

/** @defgroup QSPI_ClockMode QSPI Clock Mode
  * @{
  */
#define QSPI_CLOCK_MODE_0              ((uint32_t)0x00000000)         /*!<Clk stays low while nCS is released*/
#define QSPI_CLOCK_MODE_3              ((uint32_t)QUADSPI_DCR_CKMODE) /*!<Clk goes high while nCS is released*/
/**
  * @}
  */

/** @defgroup QSPI_AddressSize QSPI Address Size
  * @{
  */
#define QSPI_ADDRESS_8_BITS            ((uint32_t)0x00000000)           /*!<8-bit address*/
#define QSPI_ADDRESS_16_BITS           ((uint32_t)QUADSPI_CCR_ADSIZE_0) /*!<16-bit address*/
#define QSPI_ADDRESS_24_BITS           ((uint32_t)QUADSPI_CCR_ADSIZE_1) /*!<24-bit address*/
#define QSPI_ADDRESS_32_BITS           ((uint32_t)QUADSPI_CCR_ADSIZE)   /*!<32-bit address*/
/**
  * @}
  */  

/** @defgroup QSPI_AlternateBytesSize QSPI Alternate Bytes Size
  * @{
  */
#define QSPI_ALTERNATE_BYTES_8_BITS    ((uint32_t)0x00000000)           /*!<8-bit alternate bytes*/
#define QSPI_ALTERNATE_BYTES_16_BITS   ((uint32_t)QUADSPI_CCR_ABSIZE_0) /*!<16-bit alternate bytes*/
#define QSPI_ALTERNATE_BYTES_24_BITS   ((uint32_t)QUADSPI_CCR_ABSIZE_1) /*!<24-bit alternate bytes*/
#define QSPI_ALTERNATE_BYTES_32_BITS   ((uint32_t)QUADSPI_CCR_ABSIZE)   /*!<32-bit alternate bytes*/
/**
  * @}
  */

/** @defgroup QSPI_InstructionMode QSPI Instruction Mode
* @{
*/
#define QSPI_INSTRUCTION_NONE          ((uint32_t)0x00000000)          /*!<No instruction*/
#define QSPI_INSTRUCTION_1_LINE        ((uint32_t)QUADSPI_CCR_IMODE_0) /*!<Instruction on a single line*/
#define QSPI_INSTRUCTION_2_LINES       ((uint32_t)QUADSPI_CCR_IMODE_1) /*!<Instruction on two lines*/
#define QSPI_INSTRUCTION_4_LINES       ((uint32_t)QUADSPI_CCR_IMODE)   /*!<Instruction on four lines*/
/**
  * @}
  */

/** @defgroup QSPI_AddressMode QSPI Address Mode
* @{
*/
#define QSPI_ADDRESS_NONE              ((uint32_t)0x00000000)           /*!<No address*/
#define QSPI_ADDRESS_1_LINE            ((uint32_t)QUADSPI_CCR_ADMODE_0) /*!<Address on a single line*/
#define QSPI_ADDRESS_2_LINES           ((uint32_t)QUADSPI_CCR_ADMODE_1) /*!<Address on two lines*/
#define QSPI_ADDRESS_4_LINES           ((uint32_t)QUADSPI_CCR_ADMODE)   /*!<Address on four lines*/
/**
  * @}
  */  

/** @defgroup QSPI_AlternateBytesMode QSPI Alternate Bytes Mode
* @{
*/
#define QSPI_ALTERNATE_BYTES_NONE      ((uint32_t)0x00000000)           /*!<No alternate bytes*/
#define QSPI_ALTERNATE_BYTES_1_LINE    ((uint32_t)QUADSPI_CCR_ABMODE_0) /*!<Alternate bytes on a single line*/
#define QSPI_ALTERNATE_BYTES_2_LINES   ((uint32_t)QUADSPI_CCR_ABMODE_1) /*!<Alternate bytes on two lines*/
#define QSPI_ALTERNATE_BYTES_4_LINES   ((uint32_t)QUADSPI_CCR_ABMODE)   /*!<Alternate bytes on four lines*/
/**
  * @}
  */  

/** @defgroup QSPI_DataMode QSPI Data Mode
  * @{
  */
#define QSPI_DATA_NONE                 ((uint32_t)0X00000000)           /*!<No data*/
#define QSPI_DATA_1_LINE               ((uint32_t)QUADSPI_CCR_DMODE_0) /*!<Data on a single line*/
#define QSPI_DATA_2_LINES              ((uint32_t)QUADSPI_CCR_DMODE_1) /*!<Data on two lines*/
#define QSPI_DATA_4_LINES              ((uint32_t)QUADSPI_CCR_DMODE)   /*!<Data on four lines*/
/**
  * @}
  */  

/** @defgroup QSPI_DdrMode QSPI DDR Mode
  * @{
  */
#define QSPI_DDR_MODE_DISABLE          ((uint32_t)0x00000000)       /*!<Double data rate mode disabled*/
#define QSPI_DDR_MODE_ENABLE           ((uint32_t)QUADSPI_CCR_DDRM) /*!<Double data rate mode enabled*/
/**
  * @}
  */

/** @defgroup QSPI_DdrHoldHalfCycle QSPI DDR Data Output Delay
  * @{
  */
#define QSPI_DDR_HHC_ANALOG_DELAY      ((uint32_t)0x00000000)       /*!<Delay the data output using analog delay in DDR mode*/

/**
  * @}
  */

/** @defgroup QSPI_SIOOMode QSPI Send Instruction Mode
  * @{
  */
#define QSPI_SIOO_INST_EVERY_CMD       ((uint32_t)0x00000000)       /*!<Send instruction on every transaction*/
#define QSPI_SIOO_INST_ONLY_FIRST_CMD  ((uint32_t)QUADSPI_CCR_SIOO) /*!<Send instruction only for the first command*/
/**
  * @}
  */

/** @defgroup QSPI_MatchMode QSPI Match Mode
  * @{
  */
#define QSPI_MATCH_MODE_AND            ((uint32_t)0x00000000)     /*!<AND match mode between unmasked bits*/
#define QSPI_MATCH_MODE_OR             ((uint32_t)QUADSPI_CR_PMM) /*!<OR match mode between unmasked bits*/
/**
  * @}
  */  

/** @defgroup QSPI_AutomaticStop QSPI Automatic Stop
  * @{
  */
#define QSPI_AUTOMATIC_STOP_DISABLE    ((uint32_t)0x00000000)      /*!<AutoPolling stops only with abort or QSPI disabling*/
#define QSPI_AUTOMATIC_STOP_ENABLE     ((uint32_t)QUADSPI_CR_APMS) /*!<AutoPolling stops as soon as there is a match*/
/**
  * @}
  */  

/** @defgroup QSPI_TimeOutActivation QSPI Timeout Activation
  * @{
  */
#define QSPI_TIMEOUT_COUNTER_DISABLE   ((uint32_t)0x00000000)      /*!<Timeout counter disabled, nCS remains active*/
#define QSPI_TIMEOUT_COUNTER_ENABLE    ((uint32_t)QUADSPI_CR_TCEN) /*!<Timeout counter enabled, nCS released when timeout expires*/
/**
  * @}
  */  

/** @defgroup QSPI_Flags QSPI Flags
  * @{
  */
#define QSPI_FLAG_BUSY                 QUADSPI_SR_BUSY /*!<Busy flag: operation is ongoing*/
#define QSPI_FLAG_TO                   QUADSPI_SR_TOF  /*!<Timeout flag: timeout occurs in memory-mapped mode*/
#define QSPI_FLAG_SM                   QUADSPI_SR_SMF  /*!<Status match flag: received data matches in autopolling mode*/
#define QSPI_FLAG_FT                   QUADSPI_SR_FTF  /*!<Fifo threshold flag: Fifo threshold reached or data left after read from memory is complete*/
#define QSPI_FLAG_TC                   QUADSPI_SR_TCF  /*!<Transfer complete flag: programmed number of data have been transferred or the transfer has been aborted*/
#define QSPI_FLAG_TE                   QUADSPI_SR_TEF  /*!<Transfer error flag: invalid address is being accessed*/
/**
  * @}
  */

/** @defgroup QSPI_Interrupts QSPI Interrupts
  * @{
  */  
#define QSPI_IT_TO                     QUADSPI_CR_TOIE /*!<Interrupt on the timeout flag*/
#define QSPI_IT_SM                     QUADSPI_CR_SMIE /*!<Interrupt on the status match flag*/
#define QSPI_IT_FT                     QUADSPI_CR_FTIE /*!<Interrupt on the fifo threshold flag*/
#define QSPI_IT_TC                     QUADSPI_CR_TCIE /*!<Interrupt on the transfer complete flag*/
#define QSPI_IT_TE                     QUADSPI_CR_TEIE /*!<Interrupt on the transfer error flag*/

/**
  * @}
  */  

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup QSPI_Exported_Macros QSPI Exported Macros
  * @{
  */


/** @brief  Enable the specified QSPI interrupt.
  * @param  __HANDLE__: specifies the QSPI Handle.
  * @param  __INTERRUPT__: specifies the QSPI interrupt source to enable.
  *          This parameter can be one of the following values:
  *            @arg QSPI_IT_TO: QSPI Timeout interrupt
  *            @arg QSPI_IT_SM: QSPI Status match interrupt
  *            @arg QSPI_IT_FT: QSPI FIFO threshold interrupt
  *            @arg QSPI_IT_TC: QSPI Transfer complete interrupt
  *            @arg QSPI_IT_TE: QSPI Transfer error interrupt
  * @retval None
  */
#define __QSPI_ENABLE_IT(__INTERRUPT__)     SET_BIT(QUADSPI->CR, (__INTERRUPT__))


/** @brief  Disable the specified QSPI interrupt.
  * @param  __HANDLE__: specifies the QSPI Handle.
  * @param  __INTERRUPT__: specifies the QSPI interrupt source to disable.
  *          This parameter can be one of the following values:
  *            @arg QSPI_IT_TO: QSPI Timeout interrupt
  *            @arg QSPI_IT_SM: QSPI Status match interrupt
  *            @arg QSPI_IT_FT: QSPI FIFO threshold interrupt
  *            @arg QSPI_IT_TC: QSPI Transfer complete interrupt
  *            @arg QSPI_IT_TE: QSPI Transfer error interrupt
  * @retval None
  */
#define __QSPI_DISABLE_IT(__HANDLE__, __INTERRUPT__)    CLEAR_BIT(QUADSPI->CR, (__INTERRUPT__))

/** @brief  Check whether the specified QSPI interrupt source is enabled or not.
  * @param  __HANDLE__: specifies the QSPI Handle.
  * @param  __INTERRUPT__: specifies the QSPI interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg QSPI_IT_TO: QSPI Timeout interrupt
  *            @arg QSPI_IT_SM: QSPI Status match interrupt
  *            @arg QSPI_IT_FT: QSPI FIFO threshold interrupt
  *            @arg QSPI_IT_TC: QSPI Transfer complete interrupt
  *            @arg QSPI_IT_TE: QSPI Transfer error interrupt
  * @retval The new state of __INTERRUPT__ (TRUE or FALSE).
  */
#define __QSPI_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__) (READ_BIT(QUADSPI->CR, (__INTERRUPT__)) == (__INTERRUPT__)) 

/**
  * @brief  Check whether the selected QSPI flag is set or not.
  * @param  __HANDLE__: specifies the QSPI Handle.
  * @param  __FLAG__: specifies the QSPI flag to check.
  *          This parameter can be one of the following values:
  *            @arg QSPI_FLAG_BUSY: QSPI Busy flag
  *            @arg QSPI_FLAG_TO:   QSPI Timeout flag
  *            @arg QSPI_FLAG_SM:   QSPI Status match flag
  *            @arg QSPI_FLAG_FT:   QSPI FIFO threshold flag
  *            @arg QSPI_FLAG_TC:   QSPI Transfer complete flag
  *            @arg QSPI_FLAG_TE:   QSPI Transfer error flag
  * @retval None
  */
#define __QSPI_GET_FLAG(__FLAG__)           (READ_BIT(QUADSPI->SR, (__FLAG__)) != 0)

/** @brief  Clears the specified QSPI's flag status.
  * @param  __FLAG__: specifies the QSPI clear register flag that needs to be set
  *          This parameter can be one of the following values:
  *            @arg QSPI_FLAG_TO: QSPI Timeout flag
  *            @arg QSPI_FLAG_SM: QSPI Status match flag
  *            @arg QSPI_FLAG_TC: QSPI Transfer complete flag
  *            @arg QSPI_FLAG_TE: QSPI Transfer error flag
  * @retval None
  */
#define __QSPI_CLEAR_FLAG(__FLAG__)         WRITE_REG(QUADSPI->FCR, (__FLAG__))
/**
  * @}
  */

void QSPI_MspInit(uint8_t FifoThreshold, uint8_t ClockPrescaler, uint32_t SampleShifting, uint8_t FlashSize, uint32_t ChipSelectHighTime, uint32_t ClockMode);

void     QSPI_Command      (QSPI_CommandTypeDef *cmd); 
void     QSPI_Transmit     (uint8_t *pData);
void		 QSPI_Receive			 (uint8_t *pData);

void     QSPI_Transmit_DMA (uint8_t *pData);
void     QSPI_Receive_DMA  (uint8_t *pData);

/* QSPI status flag polling mode */
void     QSPI_AutoPolling   (QSPI_CommandTypeDef *cmd, QSPI_AutoPollingTypeDef *cfg);
void     QSPI_AutoPolling_IT(QSPI_CommandTypeDef *cmd, QSPI_AutoPollingTypeDef *cfg);

/* QSPI memory-mapped mode */
void     QSPI_MemoryMapped(QSPI_CommandTypeDef *cmd, QSPI_MemoryMappedTypeDef *cfg);

void QSPI_Abort(void);

#endif
