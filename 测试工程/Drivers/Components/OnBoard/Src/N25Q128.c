#include "N25Q128.h"

/**
  * @brief  This function read the SR of the memory and wait the EOP.
  * @param  None
  * @retval None
  */
void BSP_QSPI_AutoPollingMemReady(void)
{
    QSPI_CommandTypeDef     sCommand;
    QSPI_AutoPollingTypeDef sConfig;

    /* Configure automatic polling mode to wait for memory ready */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = READ_STATUS_REG_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    sConfig.Match           = 0;
    sConfig.Mask            = N25Q128A_SR_WIP;
    sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
    sConfig.StatusBytesSize = 1;
    sConfig.Interval        = 0x10;
    sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

    QSPI_AutoPolling(&sCommand, &sConfig);
}

/**
  * @brief  This function reset the QSPI memory.
  * @param  None
  * @retval None
  */
void BSP_QSPI_ResetMemory(void)
{
    QSPI_CommandTypeDef sCommand;

    /* Initialize the reset enable command */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = RESET_ENABLE_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Send the command */
    QSPI_Command(&sCommand);
    /* Send the reset memory command */
    sCommand.Instruction = RESET_MEMORY_CMD;
    QSPI_Command(&sCommand);

    /* Configure automatic polling mode to wait the memory is ready */
    BSP_QSPI_AutoPollingMemReady();
}


/**
  * @brief  This function send a Write Enable and wait it is effective.
  * @param  None
  * @retval None
  */
void BSP_QSPI_WriteEnable(void)
{
    QSPI_CommandTypeDef     sCommand;
    QSPI_AutoPollingTypeDef sConfig;

    /* Enable write operations */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = WRITE_ENABLE_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    QSPI_Command(&sCommand);

    /* Configure automatic polling mode to wait for write enabling */
    sConfig.Match           = N25Q128A_SR_WREN;
    sConfig.Mask            = N25Q128A_SR_WREN;
    sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
    sConfig.StatusBytesSize = 1;
    sConfig.Interval        = 0x10;
    sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

    sCommand.Instruction    = READ_STATUS_REG_CMD;
    sCommand.DataMode       = QSPI_DATA_1_LINE;

    QSPI_AutoPolling(&sCommand, &sConfig);
}

/**
  * @brief  This function configure the dummy cycles on memory side.
  * @param  None
  * @retval None
  */
void BSP_QSPI_DummyCyclesCfg(void)
{
    QSPI_CommandTypeDef sCommand;
    uint8_t reg;

    /* Initialize the read volatile configuration register command */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = READ_VOL_CFG_REG_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.DummyCycles       = 0;
    sCommand.NbData            = 1;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Configure the command */
    QSPI_Command(&sCommand);

    /* Reception of the data */
    QSPI_Receive(&reg);

    /* Enable write operations */
    BSP_QSPI_WriteEnable();

    /* Update volatile configuration register (with new dummy cycles) */
    sCommand.Instruction = WRITE_VOL_CFG_REG_CMD;
    MODIFY_REG(reg, N25Q128A_VCR_NB_DUMMY, (N25Q128A_DUMMY_CYCLES_READ_QUAD << POSITION_VAL(N25Q128A_VCR_NB_DUMMY)));

    /* Configure the write volatile configuration register command */
    QSPI_Command(&sCommand);

    /* Transmission of the data */
    QSPI_Transmit(&reg);

}

void BSP_QSPI_Init(void)
{
    QSPI_MspInit(4, 0, QSPI_SAMPLE_SHIFTING_NONE, 23, QSPI_CS_HIGH_TIME_1_CYCLE, QSPI_CLOCK_MODE_0);

    /* QSPI memory reset */
    BSP_QSPI_ResetMemory();

    /* Configuration of the dummy cucles on QSPI memory side */
    BSP_QSPI_DummyCyclesCfg();
}


/**
  * @brief  Reads an amount of data from the QSPI memory.
  * @param  pData: Pointer to data to be read
  * @param  ReadAddr: Read start address
  * @param  Size: Size of data to read
  * @retval None
  */
void BSP_QSPI_Read(uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
    QSPI_CommandTypeDef sCommand;

    /* Initialize the read command */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = QUAD_INOUT_FAST_READ_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.Address           = ReadAddr;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
    sCommand.DummyCycles       = N25Q128A_DUMMY_CYCLES_READ_QUAD;
    sCommand.NbData            = Size;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Configure the command */
    QSPI_Command(&sCommand);

    /* Reception of the data */
    QSPI_Receive(pData);
}

/**
  * @brief  Writes an amount of data to the QSPI memory.
  * @param  pData: Pointer to data to be written
  * @param  WriteAddr: Write start address
  * @param  Size: Size of data to write
  * @retval None
  */
void BSP_QSPI_Write(uint8_t *pData, uint32_t WriteAddr, uint32_t Size)
{
    QSPI_CommandTypeDef sCommand;
    uint32_t end_addr, current_size, current_addr;

    /* Calculation of the size between the write address and the end of the page */
    current_size = N25Q128A_PAGE_SIZE - (WriteAddr % N25Q128A_PAGE_SIZE);

    /* Check if the size of the data is less than the remaining place in the page */
    if (current_size > Size)
    {
        current_size = Size;
    }

    /* Initialize the adress variables */
    current_addr = WriteAddr;
    end_addr = WriteAddr + Size;

    /* Initialize the program command */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = EXT_QUAD_IN_FAST_PROG_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Perform the write page by page */
    do
    {
        sCommand.Address = current_addr;
        sCommand.NbData  = current_size;

        /* Enable write operations */
        BSP_QSPI_WriteEnable();
        /* Configure the command */
        QSPI_Command(&sCommand);

        /* Transmission of the data */
        QSPI_Transmit(pData);

        /* Configure automatic polling mode to wait for end of program */
        BSP_QSPI_AutoPollingMemReady();

        /* Update the address and size variables for next page programming */
        current_addr += current_size;
        pData += current_size;
        current_size = ((current_addr + N25Q128A_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : N25Q128A_PAGE_SIZE;
    }
    while (current_addr < end_addr);

}


/**
  * @brief  Erases the specified block of the QSPI memory.
  * @param  BlockAddress: Block address to erase
  * @retval None
  */
void BSP_QSPI_Erase_Block(uint32_t BlockAddress)
{
    QSPI_CommandTypeDef sCommand;

    /* Initialize the erase command */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = SUBSECTOR_ERASE_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.Address           = BlockAddress;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Enable write operations */
    BSP_QSPI_WriteEnable();

    /* Send the command */
    QSPI_Command(&sCommand);

    /* Configure automatic polling mode to wait for end of erase */
    BSP_QSPI_AutoPollingMemReady();
}

/**
  * @brief  Erases the specified sector of the QSPI memory.
  * @param  Sector: Sector address to erase (0 to 255)
  * @retval None
  * @note This function is non blocking meaning that sector erase
  *       operation is started but not completed when the function
  *       returns. Application has to call BSP_QSPI_GetStatus()
  *       to know when the device is available again (i.e. erase operation
  *       completed).
  */
void BSP_QSPI_Erase_Sector(uint32_t Sector)
{
    QSPI_CommandTypeDef sCommand;

    /* Initialize the erase command */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = SECTOR_ERASE_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.Address           = (Sector * N25Q128A_SECTOR_SIZE);
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Enable write operations */
    BSP_QSPI_WriteEnable();

    QSPI_Command(&sCommand);

    /* Configure automatic polling mode to wait for end of erase */
    BSP_QSPI_AutoPollingMemReady();
}


/**
  * @brief  Erases the entire QSPI memory.
  * @retval None
  */
void BSP_QSPI_Erase_Chip(void)
{
    QSPI_CommandTypeDef sCommand;

    /* Initialize the erase command */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = BULK_ERASE_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Enable write operations */
    BSP_QSPI_WriteEnable();

    /* Send the command */
    QSPI_Command(&sCommand);

    /* Configure automatic polling mode to wait for end of erase */
    BSP_QSPI_AutoPollingMemReady();
}


/**
  * @brief  Reads current status of the QSPI memory.
  * @retval QSPI memory status
  */
uint8_t BSP_QSPI_GetStatus(void)
{
    QSPI_CommandTypeDef sCommand;
    uint8_t reg;

    /* Initialize the read flag status register command */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = READ_FLAG_STATUS_REG_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.DummyCycles       = 0;
    sCommand.NbData            = 1;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Configure the command */
    QSPI_Command(&sCommand);
    /* Reception of the data */
    QSPI_Receive(&reg);
    /* Check the value of the register */
    if ((reg & (N25Q128A_FSR_PRERR | N25Q128A_FSR_VPPERR | N25Q128A_FSR_PGERR | N25Q128A_FSR_ERERR)) != 0)
    {
        return QSPI_ERROR;
    }
    else if ((reg & (N25Q128A_FSR_PGSUS | N25Q128A_FSR_ERSUS)) != 0)
    {
        return QSPI_SUSPENDED;
    }
    else if ((reg & N25Q128A_FSR_READY) != 0)
    {
        return QSPI_OK;
    }
    else
    {
        return QSPI_BUSY;
    }
}

/**
  * @brief  Configure the QSPI in memory-mapped mode
  * @retval None
  */
void BSP_QSPI_EnableMemoryMappedMode(void)
{
    QSPI_CommandTypeDef      sCommand;
    QSPI_MemoryMappedTypeDef sMemMappedCfg;

    /* Configure the command for the read instruction */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = QUAD_INOUT_FAST_READ_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
    sCommand.DummyCycles       = N25Q128A_DUMMY_CYCLES_READ_QUAD;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Configure the memory mapped mode */
    sMemMappedCfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;

    QSPI_MemoryMapped(&sCommand, &sMemMappedCfg);
}

/**
  * @brief  Configure the QSPI in memory-mapped mode
  * @param  BSP_QSPI_ID_TypeDef: EDID + UID Struct
  * @retval None
  */
void BSP_QSPI_RDID(BSP_QSPI_ID_TypeDef *pID)
{
    QSPI_CommandTypeDef sCommand;
    uint8_t *pRxBuffPtr = (uint8_t *)pID;

    /* Initialize the erase command */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = READ_ID_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.DummyCycles       = 0;
    sCommand.NbData						 = 20;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Send the command */
    QSPI_Command(&sCommand);

    /* Configure automatic polling mode to wait for end of erase */
    QSPI_Receive(pRxBuffPtr);
}



/**
  * @brief  Configure the QSPI in memory-mapped mode
  * @param  SFDP_TypeDef:
  * @retval None
  */
void BSP_QSPI_Read_SPDF(BSP_QSPI_SFDP_TypeDef *pID)
{
    QSPI_CommandTypeDef sCommand;
    uint8_t *pRxBuffPtr = (uint8_t *)pID;

    /* Initialize the read command */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = 0x5A;
    sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.Address           = 0x00000000;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.DummyCycles       = 8;
    sCommand.NbData            = 16;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Configure the command */
    QSPI_Command(&sCommand);

    /* Reception of the data */
    QSPI_Receive(pRxBuffPtr);

    pID->PTP = pID->PTP & 0xFFFFFF;

    /* 此处获得对应位置指针. */

    /* Initialize the read command */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = 0x5A;
    sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.Address           = pID->PTP;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.DummyCycles       = 8;
    sCommand.NbData            = 36;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Configure the command */
    QSPI_Command(&sCommand);

    /* Reception of the data */
    QSPI_Receive(pRxBuffPtr + 0x10);
}

/* 下面是HAL API,为了提供给SPIFFS临时使用,实际上修改SPIFFS也可以兼容我的专用API. */

int32_t BSP_QSPI_Write_HAL(uint32_t WriteAddr,uint32_t NumByteToWrite,uint8_t * pBuffer)
{	
  BSP_QSPI_Write(pBuffer, WriteAddr, NumByteToWrite);
	return 0;
} 

int32_t BSP_QSPI_Read_HAL(uint32_t WriteAddr,uint32_t NumByteToWrite,uint8_t * pBuffer)
{	
  BSP_QSPI_Write(pBuffer, WriteAddr, NumByteToWrite);
	return 0;
} 


int32_t BSP_QSPI_Erase_HAL(uint32_t Addr,uint32_t Num)  
{
			BSP_QSPI_Erase_Block(Addr >> 12);
	return 0;
} 
