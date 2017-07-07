/**
    描述: 应用于NOR Flash磨损平衡算法
    文件: WL_Flash.h
    注意: 要移植相应头文件,以及NOR Flash驱动,还有Malloc函数.

    @author TaterLi
    @version 2017/07/06
*/

#ifndef _WL_Flash_H_
#define _WL_Flash_H_

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

typedef struct WL_State_s
{
    uint16_t pos;           /*!< 当前的dummy_block的地址 */
    uint16_t max_pos;       /*!< 最大用户可到达尺寸 */
    uint16_t move_count;    /*!< 已经挪动的写入指针位置 */
    uint16_t block_size;    /*!< 块大小 */
    uint8_t version;       /*!< 配置版本 */
    uint32_t crc;           /*!< CRC 校验 */
} wl_state_t;

typedef struct WL_Config_s
{
    uint32_t start_addr;      /*!< 储存器的起始地址,如过是整片利用,就是0地址了. */
    uint32_t full_mem_size; /*!< 储存器的结束地址(尺寸),如果是整片,就是整片大小了. */
    uint16_t page_size;     /*!< Page大小,另外说法是SubSector.*/
    uint16_t sector_size;   /*!< 一次性擦除大小,通常都支持4K擦,就是page_size = sector_size,不支持时候这两个不相等. */
    uint16_t wr_size;       /*!< 最小写入大小 */
    uint8_t version;       /*!< 配置版本 */
    uint16_t temp_buff_size;  /*!< Buffer的大小,与sector_size求余为0.*/
    uint32_t crc;           /*!< CRC 校验 */

} wl_config_t;

typedef struct WL_Flash
{
    wl_state_t state; /* 状态配置 */
    wl_config_t cfg; /* Flash参数 */

    uint32_t addr_cfg; /* 配置的储存地址 */
    uint32_t addr_state1; /* state1的储存地址 */
    uint32_t addr_state2; /* state2的储存地址 */

    uint32_t flash_size; /* flash大小,这是用户能用的,已经扣减了冗余,配置部分. */
    uint32_t state_size; /* state结构大小. */
    uint16_t cfg_size; /* cfg结构大小 */
    uint8_t *temp_buff; /* 缓冲区指针 */
    uint32_t dummy_addr; /* dummy数据配置地址 */
} wl_flash_t;

void WL_Flash_Config(wl_flash_t *WL_Flash);
void WL_Flash_Erase_Range(wl_flash_t *WL_Flash, uint32_t start_address, uint32_t size);
void WL_Flash_Write(wl_flash_t *WL_Flash, uint32_t dest_addr, const uint8_t *src, size_t size);
void WL_Flash_Read(wl_flash_t *WL_Flash, uint32_t src_addr, uint8_t *dest, size_t size);

#endif
