#include "CRC.h"

uint32_t Calculate_CRC(uint8_t *pBuf,uint32_t BufferSize)
{
  register uint32_t data = 0;
  register uint32_t index = 0;
	
	LL_CRC_ResetCRCCalculationUnit(CRC);
	
  /* Compute the CRC of Data Buffer array*/
  for (index = 0; index < (BufferSize / 4); index++)
  {
    data = (uint32_t)((pBuf[4 * index + 3] << 24) | (pBuf[4 * index + 2] << 16) | (pBuf[4 * index + 1] << 8) | pBuf[4 * index]);
    LL_CRC_FeedData32(CRC, data);
  }
  
  /* Last bytes specific handling */
  if ((BufferSize % 4) != 0)
  {
    if  (BufferSize % 4 == 1)
    {
      LL_CRC_FeedData8(CRC, pBuf[4 * index]);
    }
    if  (BufferSize % 4 == 2)
    {
      LL_CRC_FeedData16(CRC, (uint16_t)((pBuf[4 * index + 1]<<8) | pBuf[4 * index]));
    }
    if  (BufferSize % 4 == 3)
    {
      LL_CRC_FeedData16(CRC, (uint16_t)((pBuf[4 * index + 1]<<8) | pBuf[4 * index]));
      LL_CRC_FeedData8(CRC, pBuf[4 * index + 2]);
    }
  }

  /* Return computed CRC value */
  return(LL_CRC_ReadData32(CRC));
	
}
