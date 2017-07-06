/**
    描述: 应用于NOR Flash磨损平衡算法
    文件: WL_Flash.c
    注意: 要移植相应头文件,以及NOR Flash驱动,还有Malloc函数.

    @author TaterLi
    @version 2017/07/06
*/

#include "WL_Flash.h" /* 此文件是这个C的头文件. */
#include "CRC.h" /* 此文件必须实现Calculate_CRC功能. */
#include "N25Q128.h" /* 此文件要实现BSP_QSPI_Erase_Block,BSP_QSPI_Read,BSP_QSPI_Write函数. */


static uint32_t WL_Flash_calcAddr(wl_flash_t *WL_Flash, uint32_t addr);
static void WL_Flash_Erase_RAW(wl_flash_t *WL_Flash, uint32_t start_address, uint32_t size);
static void WL_Flash_recoverPos(wl_flash_t *WL_Flash);
static void WL_Flash_initSections(wl_flash_t *WL_Flash);
static void WL_Flash_Erase_Sector(wl_flash_t *WL_Flash, uint32_t sector);
static void WL_Flash_updateWL(wl_flash_t *WL_Flash);

/**
  * @brief  从虚拟地址计算出物理地址.
  * @param  WL_FLash: 磨损平衡结构体(此时正在初始化,此函数不是用户调用的.).
  * @param  addr: 虚拟地址.
  */
static uint32_t WL_Flash_calcAddr(wl_flash_t *WL_Flash, uint32_t addr)
{
    /* 计算得到物理地址了. */
    uint32_t result = (WL_Flash->flash_size - WL_Flash->state.move_count * WL_Flash->cfg.page_size + addr) % WL_Flash->flash_size;
    /* 计算出对应的dummy_addr. */
    uint32_t dummy_addr = WL_Flash->state.pos * WL_Flash->cfg.page_size;
    /* 如果计算结果没占用到dummy_addr储存区域,就不用管.如果占用到了,那要往下一个page挪动. */
    if (result >= dummy_addr)
    {
        result += WL_Flash->cfg.page_size;
    }
    return result;
}

/**
  * @brief  擦SubSector + 更新WL.
  * @param  WL_FLash: 磨损平衡结构体(此时正在初始化,此函数不是用户调用的.).
  * @param  sector: 需要擦的SubSector的地址.
  */
static void WL_Flash_Erase_Sector(wl_flash_t *WL_Flash, uint32_t sector)
{
    /* 先更新表,这时候pos就移动了,更新WL的时刻只有此时出现. */
    WL_Flash_updateWL(WL_Flash);
    /* 转换虚拟地址,VA -> PA变换. */
    uint32_t virt_addr = WL_Flash_calcAddr(WL_Flash, sector * WL_Flash->cfg.sector_size);
    /* 执行真实擦除. */
    BSP_QSPI_Erase_Block(WL_Flash->cfg.start_addr + virt_addr);
}

/**
  * @brief  初始化这个Flash磨损平衡的底层函数.
  * @param  WL_FLash: 磨损平衡结构体(此时正在初始化,此函数不是用户调用的.).
  */
static void WL_Flash_initSections(wl_flash_t *WL_Flash)
{
    /* 第一次初始化,pos当然是0. */
    WL_Flash->state.pos = 0;
    /* 第一次初始化,pos当然是0. */
    WL_Flash->state.move_count = 0;
    /* 更新后当前state的版本跟用户配置版本肯定是一样的啊. */
    WL_Flash->state.version = WL_Flash->cfg.version;
    /* 粒度大小 = SubSector大小,这样很方便. */
    WL_Flash->state.block_size = WL_Flash->cfg.page_size;
    /* max_pos是由可用SubSector扇区数量决定的,比如16MB的W25Q128就是4000个左右(要减去冗余和计算部分). */
    WL_Flash->state.max_pos = 1 + WL_Flash->flash_size / WL_Flash->cfg.page_size;
    /* 计算state的CRC,因为每此上电都判断这个来表示state的内容是否正确. */
    WL_Flash->state.crc = Calculate_CRC((uint8_t *)&WL_Flash->state, sizeof(wl_state_t) - sizeof(uint32_t));
    /* 把两个state都存起来,以便掉电还能重新取出. */
    WL_Flash_Erase_RAW(WL_Flash, WL_Flash->addr_state1, WL_Flash->state_size);
    BSP_QSPI_Write((uint8_t *)&WL_Flash->state, WL_Flash->addr_state1, sizeof(wl_state_t));
    /* 因为冗余了两份,所以要写两次. */
    WL_Flash_Erase_RAW(WL_Flash, WL_Flash->addr_state2, WL_Flash->state_size);
    BSP_QSPI_Write((uint8_t *)&WL_Flash->state, WL_Flash->addr_state2, sizeof(wl_state_t));
    /* 地址配置也要写进去. */
    WL_Flash_Erase_RAW(WL_Flash, WL_Flash->addr_cfg, WL_Flash->cfg_size);
    BSP_QSPI_Write((uint8_t *)&WL_Flash->cfg, WL_Flash->addr_cfg, sizeof(wl_config_t));
}

/**
  * @brief  恢复正在使用的坐标.
  * @param  WL_FLash: 磨损平衡结构体(必须已经被初始化).
  * @param  start_address: 起始地址.
  * @param  size: 需要擦除长度.
  */
static void WL_Flash_recoverPos(wl_flash_t *WL_Flash)
{
    /* 循环查找所有坐标,时间不确定性. */
    for (size_t i = 0; i < WL_Flash->state.max_pos; i++)
    {
        uint8_t pos_bits = 0;
        /* 把坐标位读出来,如果用了的坐标是0x00的,没用就是0xFF的. */
        BSP_QSPI_Read(&pos_bits, WL_Flash->addr_state1 + sizeof(wl_state_t) + i * WL_Flash->cfg.wr_size, 1);
        /* 开始判断是不是找到了. */
        if (pos_bits == 0xff)
        {
            /* 填坐标,因为找到了,现在这个坐标就是可以用的坐标了. */
            WL_Flash->state.pos = i;
            break; /* 找到坐标了,跳出循环. */
        }
    }
    /* 如果是最大pos了,就减一下,不然就覆盖后面的配置了,到下次更新磨损平衡表就能修复pos的这个问题.所以此处不修复.(因为上电要执行这个函数,费时费力.) */
    if (WL_Flash->state.pos == WL_Flash->state.max_pos)
    {
        WL_Flash->state.pos--;
    }

}

/**
  * @brief  直接物理擦除
  * @param  WL_FLash: 磨损平衡结构体(必须已经被初始化).
  * @param  start_address: 起始地址.
  * @param  size: 需要擦除长度.
  */
static void WL_Flash_Erase_RAW(wl_flash_t *WL_Flash, uint32_t start_address, uint32_t size)
{
    uint32_t i = 0;
    for (i = 0; i < size / WL_Flash->cfg.sector_size; i++)
    {
		/* 已经是物理擦除. */
        BSP_QSPI_Erase_Block(start_address + (i * WL_Flash->cfg.sector_size));
    }
}

/**
  * @brief  磨损平衡表更新
  * @param  WL_FLash: 磨损平衡结构体(必须已经被初始化).
  */
static void WL_Flash_updateWL(wl_flash_t *WL_Flash)
{
    /* 等下要清零的,先储存一个数据. */
    const uint8_t used_bits = 0;
    /* 下次要访问的pos偏移,要不断移动pos,才能做到平衡.不能总在操作一个地方. */
    size_t data_addr = WL_Flash->state.pos + 1; /* pos + 1 => pos */
    if (data_addr >= WL_Flash->state.max_pos) /* 如果到最大pos了,就返回0的位置继续磨损.max_pos是可用的SubSector数量.用这个限制着,不会超出坐标. */
    {
        data_addr = 0;
    }
    /* 算出真正的需要磨损的下一page地址.实际上要改move_count才真正修改磨损坐标偏移. */
    data_addr = WL_Flash->cfg.start_addr + data_addr * WL_Flash->cfg.page_size;
    /* 根据pos偏移,算出我需要的dummy_addr位置.这是下一个要用的位置.擦掉. */
    WL_Flash->dummy_addr = WL_Flash->cfg.start_addr + WL_Flash->state.pos * WL_Flash->cfg.page_size;
    /* 擦掉下一个要用到的位置. */
    WL_Flash_Erase_RAW(WL_Flash, WL_Flash->dummy_addr, WL_Flash->cfg.page_size);
    /* 根据buff求出需要复制的次数,所以buff越大速度越快,当然也是有理论上限的. */
    size_t copy_count = WL_Flash->cfg.page_size / WL_Flash->cfg.temp_buff_size;
    for (size_t i = 0; i < copy_count; i++)
    {
        /* 先读取当前位置的,然后写到下一位置的.复制数据. */
        BSP_QSPI_Read(WL_Flash->temp_buff, data_addr + i * WL_Flash->cfg.temp_buff_size, WL_Flash->cfg.temp_buff_size);
        BSP_QSPI_Write(WL_Flash->temp_buff, WL_Flash->dummy_addr + i * WL_Flash->cfg.temp_buff_size, WL_Flash->cfg.temp_buff_size);
    }
    /* 求出新pos位置. */
    uint32_t byte_pos = WL_Flash->state.pos * WL_Flash->cfg.wr_size;
    /* 标准当前正在使用的位.(对应的pos位为0表示已经用了.) */
    BSP_QSPI_Write((uint8_t *)&used_bits, WL_Flash->addr_state1 + sizeof(wl_state_t) + byte_pos, 1);
    BSP_QSPI_Write((uint8_t *)&used_bits, WL_Flash->addr_state2 + sizeof(wl_state_t) + byte_pos, 1);
    /* 但是现在用的是新pos位.也就是下一个pos位,每次都挪动一次pos.正常来说只要执行擦除,pos就挪动,使用磨损平衡库依然需要擦除各种,但是这个磨损库不用建FTL对照表. */
    WL_Flash->state.pos++;
    /* 到最大pos的话当然就要归零,不然就可以直接出去了.所以这个功能不是时间确定性的. */
    if (WL_Flash->state.pos >= WL_Flash->state.max_pos)
    {
        /* 把坐标定位0,从头开始. */
        WL_Flash->state.pos = 0;
        /* move_count是已经写了多少圈. */
        WL_Flash->state.move_count++;
        /* 写的圈数也是超出了,每个SubSector有独立的cfg,超出了数量,当然要从零重新开始了. */
        if (WL_Flash->state.move_count >= (WL_Flash->state.max_pos - 1))
        {
            WL_Flash->state.move_count = 0;
        }
        /* 更新state结构,因为这个结构已经改了,另外要写两份,因为两个地方. */
        WL_Flash->state.crc = Calculate_CRC((uint8_t *)&WL_Flash->state, sizeof(wl_state_t) - sizeof(uint32_t));
        WL_Flash_Erase_RAW(WL_Flash, WL_Flash->addr_state1, WL_Flash->state_size);
        BSP_QSPI_Write((uint8_t *)&WL_Flash->state, WL_Flash->addr_state1, sizeof(wl_state_t));
        /* 再写一份,冗余的. */
        WL_Flash_Erase_RAW(WL_Flash, WL_Flash->addr_state2, WL_Flash->state_size);
        BSP_QSPI_Write((uint8_t *)&WL_Flash->state, WL_Flash->addr_state2, sizeof(wl_state_t));

    }
}

/**
  * @brief  磨损平衡擦除
  * @param  WL_FLash: 磨损平衡结构体(必须已经被初始化).
  * @param  start_address: 起始地址.
  * @param  size: 需要擦除长度.
  */
void WL_Flash_Erase_Range(wl_flash_t *WL_Flash, uint32_t start_address, uint32_t size)
{
    uint32_t i = 0;
    /* 需要擦的块数量.用单位块大小来计算. */
    uint32_t erase_count = (size + WL_Flash->cfg.sector_size - 1) / WL_Flash->cfg.sector_size;
    /* 起始块所在地址. */
    uint32_t start_sector = start_address / WL_Flash->cfg.sector_size;
    for (i = 0; i < erase_count; i++)
    {
        /* 循环擦除. */
        WL_Flash_Erase_Sector(WL_Flash, start_sector + i);
    }
}

/**
  * @brief  初始化Flash磨损平衡模块
  * @param  WL_FLash: 磨损平衡结构体.
  *
  *  WL_Flash.cfg.start_addr = 0x00000000; -- 起始地址
  *  WL_Flash.cfg.full_mem_size = 0x01000000; -- 需要Flash的大小
  *  WL_Flash.cfg.page_size = 0x0001000; -- 页(SubSector)大小,一般为4K.
  *  WL_Flash.cfg.sector_size = 0x000010000; -- 扇区大小,一般为64K.
  *  WL_Flash.cfg.wr_size = 0x00000010; -- 写入大小
  *  WL_Flash.cfg.version = 0x00000001; -- 版本
  *  WL_Flash.cfg.temp_buff_size = 0x00000020; -- 缓冲大小
  *
  */
void WL_Flash_Config(wl_flash_t *WL_Flash)
{
    wl_state_t sa_copy; /* 储存的第二份state结构体. */
    wl_state_t *state_copy = &sa_copy; /* sa_copy对应的指针. */
    uint32_t check_size = 0; /* CRC需要计算的尺寸. */
    static uint32_t crc1; /* 需要计算的CRC1 */
    static uint32_t crc2; /* 需要计算的CRC2 */

    /* 计算配置结构体的CRC.最后一个结构体是CRC,所以最后一个结构体不算. */
    WL_Flash->cfg.crc = Calculate_CRC((uint8_t *)(&WL_Flash->cfg), sizeof(wl_config_t) - sizeof(WL_Flash->cfg.crc));

    /* 检查Buffer对齐. */
    if(((&WL_Flash->cfg)->sector_size % (&WL_Flash->cfg)->temp_buff_size) != 0)
    {
        /* 出错原因,Buf存不下一个Sector. */
    }

    /* SPI Flash 都满足条件 SubSector < Sector */
    if(((&WL_Flash->cfg)->page_size < (&WL_Flash->cfg)->sector_size))
    {
        /* 出错原因,Page(一般4K)比Sector(一般64K)还小. */
    }

    /* 申请内存,如果不使用FreeRTOS,那么要移植这个函数. */
    WL_Flash->temp_buff = (uint8_t *)pvPortMalloc(WL_Flash->cfg.temp_buff_size);
    /* 使得一个state_size占用一个sector.这样可以先判断是否一个sector能存下这个东西. */
    WL_Flash->state_size = WL_Flash->cfg.sector_size;

    /* wl_state_t所占大小 +  有多少个sector * 写入大小 = state所需大小,之前state_size已经赋值了1个sector,如果超过一个sector就进入这个判断. */
    if (WL_Flash->state_size < (sizeof(wl_state_t) + (WL_Flash->cfg.full_mem_size / WL_Flash->cfg.sector_size)*WL_Flash->cfg.wr_size))
    {
        /* 占用sector数量:((wl_state_t所占大小 +  有多少个sector * 写入大小) + 1扇区大小 - 1) / 扇区大小 = 所需扇区数量,为什么要+1扇区大小,不然就等于0了.  */
        WL_Flash->state_size = ((sizeof(wl_state_t) + (WL_Flash->cfg.full_mem_size / WL_Flash->cfg.sector_size) * WL_Flash->cfg.wr_size) + WL_Flash->cfg.sector_size - 1) / WL_Flash->cfg.sector_size;
        /* 然后这里再乘以扇区大小,等于所需占用字节数量. */
        WL_Flash->state_size = WL_Flash->state_size * WL_Flash->cfg.sector_size;
    }

    /* 计算出cfg结构的大小.用同样的方法. */
    WL_Flash->cfg_size = (sizeof(wl_config_t) + WL_Flash->cfg.sector_size - 1) / WL_Flash->cfg.sector_size;
    WL_Flash->cfg_size = WL_Flash->cfg_size * WL_Flash->cfg.sector_size;
    /* 计算出三个结构体占用Flash的位置. */
    WL_Flash->addr_cfg = WL_Flash->cfg.start_addr + WL_Flash->cfg.full_mem_size - WL_Flash->cfg_size; /* 末端存配置 */
    WL_Flash->addr_state1 = WL_Flash->cfg.start_addr + WL_Flash->cfg.full_mem_size - WL_Flash->state_size * 2 - WL_Flash->cfg_size; /* 同上 */
    WL_Flash->addr_state2 = WL_Flash->cfg.start_addr + WL_Flash->cfg.full_mem_size - WL_Flash->state_size * 1 - WL_Flash->cfg_size; /* 同上 */

    /* 所剩可用 */
    WL_Flash->flash_size = ((WL_Flash->cfg.full_mem_size - WL_Flash->state_size * 2 - WL_Flash->cfg_size) / WL_Flash->cfg.page_size - 1) * WL_Flash->cfg.page_size; // 再让出一个区(dummy)

    /* 进入初始化流程,先把两个都读出来,这里存的就是数据,这两个块磨损很大,所以需要备份,以免其中一个挂掉了. */
    BSP_QSPI_Read((uint8_t *)&WL_Flash->state, WL_Flash->addr_state1, sizeof(wl_state_t)); /* 读取两个状态寄存器 */
    BSP_QSPI_Read((uint8_t *)state_copy, WL_Flash->addr_state2, sizeof(wl_state_t));

    /* CRC计算,确保数据正确(或者区块磨损极限了). */
    check_size = sizeof(wl_state_t) - sizeof(uint32_t);
    crc1 = Calculate_CRC((uint8_t *)&WL_Flash->state, check_size);
    crc2 = Calculate_CRC((uint8_t *)state_copy, check_size);

    /* 判断是不是两个都正常.一般Flash都正常.如果全新的就两个都不对,如果是损坏的就其中一个不对. */
    if ((crc1 == WL_Flash->state.crc) && (crc2 == state_copy->crc))
    {
        /* 一切正常,判断下现在储存的版本跟配置版本是否一致,是一致的,那么就不用重新初始化,否则重新初始化. */
        if (WL_Flash->state.version != WL_Flash->cfg.version)
        {
            /* 现在存在Flash的版本和我们需要使用的版本不同,于是初始化之. */
            WL_Flash_initSections(WL_Flash);
            /* 恢复一下现在的写指针. */
            WL_Flash_recoverPos(WL_Flash);
        }
        else
        {
            /* CRC1 不等于 CRC2,但是他们都等于他们各自储存的CRC,所以区块没有坏,只是需要更新第二结构体. */
            if (crc1 != crc2)
            {
                /* 擦掉第二结构体. */
                WL_Flash_Erase_RAW(WL_Flash, WL_Flash->addr_state2, WL_Flash->state_size);
                /* 把第一结构体内容放到第二结构体里面去. */
                BSP_QSPI_Write((uint8_t *)&WL_Flash->state, WL_Flash->addr_state2, sizeof(wl_state_t));
                for (size_t i = 0; i < ((WL_Flash->cfg.full_mem_size / WL_Flash->cfg.sector_size)*WL_Flash->cfg.wr_size); i++)
                {
                    uint8_t pos_bits = 0;
                    /* 从1号开始读,然后如果有数据,就写到2号,当然不判断0xFF也行,但是浪费编程时间啊. */
                    BSP_QSPI_Read(&pos_bits, WL_Flash->addr_state1 + sizeof(wl_state_t) + i, 1);
                    if (pos_bits != 0xff)
                    {
                        /* 从1号有数据,要写到2号的,所以开始写了. */
                        BSP_QSPI_Write(&pos_bits, WL_Flash->addr_state2 + sizeof(wl_state_t) + i, 1);
                    }
                }

                /* 把这个执行过后,1和2号的内容就一样了. */
            }
            /* 如果两个都是好的,直接到这里,恢复写指针. */
            WL_Flash_recoverPos(WL_Flash);
        }
    }
    else if ((crc1 != WL_Flash->state.crc) && (crc2 != state_copy->crc))
    {
        /* 新的Flash.因为两个都是0xFF,所以进入这里,当然也可以是Flash坏得太厉害. */
        /* 然后进入初始化. */
        WL_Flash_initSections(WL_Flash);
        /* 初始化(恢复)坐标. */
        WL_Flash_recoverPos(WL_Flash);
    }
    else
    {
        /* Flash 老化但是能用,或者数据有点乱. */
        if (crc1 == WL_Flash->state.crc)  /* CRC1是对的,证明第一个结构体是没问题的,那么就是第二个结构体有问题. */
        {
            /* 擦掉第二结构体.因为第二结构体有问题. */
            WL_Flash_Erase_RAW(WL_Flash, WL_Flash->addr_state2, WL_Flash->state_size);
            /* 把一号结构体换到二号结构体. */
            BSP_QSPI_Write((uint8_t *)&WL_Flash->state, WL_Flash->addr_state2, sizeof(wl_state_t));
            /* 把第一结构体内容放到第二结构体里面去. */
            for (size_t i = 0; i < ((WL_Flash->cfg.full_mem_size / WL_Flash->cfg.sector_size) * WL_Flash->cfg.wr_size); i++)
            {
                uint8_t pos_bits = 0;
                /* 从1号开始读,然后如果有数据,就写到2号,当然不判断0xFF也行,但是浪费编程时间啊. */
                BSP_QSPI_Read(&pos_bits, WL_Flash->addr_state1 + sizeof(wl_state_t) + i, 1);
                if (pos_bits != 0xff)
                {
                    /* 从1号有数据,要写到2号的,所以开始写了. */
                    BSP_QSPI_Write(&pos_bits, WL_Flash->addr_state2 + sizeof(wl_state_t) + i, 1);
                }
            }
            /* 读取,以便后续比较. */
            BSP_QSPI_Read((uint8_t *)&WL_Flash->state, WL_Flash->addr_state2, sizeof(wl_state_t));
        }
        else    /* CRC1是错的,证明第一个结构体是有问题的,那么就是第二个结构体无问题. */
        {
            /* 擦掉第一结构体,因为第一结构体有问题. */
            WL_Flash_Erase_RAW(WL_Flash, WL_Flash->addr_state1, WL_Flash->state_size);
            /* 把第二结构体内容写进去,state_copy在上面判断前已经提起到了. */
            BSP_QSPI_Write((uint8_t *)state_copy, WL_Flash->addr_state1, sizeof(wl_state_t));
            /* 把第二结构体内容放到第一结构体里面去. */
            for (size_t i = 0; i < ((WL_Flash->cfg.full_mem_size / WL_Flash->cfg.sector_size) * WL_Flash->cfg.wr_size); i++)
            {
                uint8_t pos_bits = 0;
                /* 从2号开始读,然后如果有数据,就写到1号,当然不判断0xFF也行,但是浪费编程时间啊. */
                BSP_QSPI_Read(&pos_bits, WL_Flash->addr_state2 + sizeof(wl_state_t) + i, 1);
                if (pos_bits != 0xff)
                {
                    /* 从2号有数据,要写到1号的,所以开始写了. */
                    BSP_QSPI_Write(&pos_bits, WL_Flash->addr_state1 + sizeof(wl_state_t) + i, 1);
                }
            }
            /* 读取,以便后续比较. */
            BSP_QSPI_Read((uint8_t *)&WL_Flash->state, WL_Flash->addr_state1, sizeof(wl_state_t));
            /* 移动pos坐标,因为第一个有问题,当做是储存芯片被重初始化(坐标). */
            WL_Flash->state.pos = WL_Flash->state.max_pos - 1;
        }
        /* 判断下配置版本对不对,不对就要更新配置版本了. */
        if (WL_Flash->state.version != WL_Flash->cfg.version)
        {
            /* 重新初始化,更新配置. */
            WL_Flash_initSections(WL_Flash);
        }
    }
}

/**
  * @brief  磨损平衡表写入
  * @param  WL_FLash: 磨损平衡结构体(必须已经被初始化).
  * @param  dest_addr: 目标地址.
  * @param  src: 需要写入的内容,需要转类型到uint8_t *,当然这里传void类型也是可以的.
  * @param  size: 需要些的长度(单位:Byte).
  */
void WL_Flash_Write(wl_flash_t *WL_Flash, uint32_t dest_addr, const uint8_t *src, size_t size)
{
    /* 看看要读取的有多少个Page.如果size不足一个Page,那么count是0,下面的for被短路了. */
    uint32_t count = (size - 1) / WL_Flash->cfg.page_size;
    for (uint32_t i = 0; i < count; i++)
    {
        /* 要计算出虚拟地址,因为VA -> PA转换,才能保证每次写的VA都不会一直磨一个块,而用户不用管VA要不要变. */
        uint32_t virt_addr = WL_Flash_calcAddr(WL_Flash, dest_addr + i * WL_Flash->cfg.page_size);
        /* 真正要写入的是VA地址,每次写一个SubSector,然后下次继续轮奸. */
        BSP_QSPI_Write(&((uint8_t *)src)[i * WL_Flash->cfg.page_size], WL_Flash->cfg.start_addr + virt_addr, WL_Flash->cfg.page_size);
    }
    /* 要计算出虚拟地址,因为VA -> PA转换,才能保证每次写的VA都不会一直磨一个块,而用户不用管VA要不要变. */
    uint32_t virt_addr_last = WL_Flash_calcAddr(WL_Flash, dest_addr + count * WL_Flash->cfg.page_size);
    /* 要写的大小,这里分两种情况,如果count为0,那么size就是size,因为后面没有减少任何东西,如果count不为0,就要减掉上面写的数据量,传剩下部分. */
    BSP_QSPI_Write(&((uint8_t *)src)[count * WL_Flash->cfg.page_size], WL_Flash->cfg.start_addr + virt_addr_last, size - count * WL_Flash->cfg.page_size);
}

/**
  * @brief  磨损平衡表读取
  * @param  WL_FLash: 磨损平衡结构体(必须已经被初始化).
  * @param  src_addr: 目标地址.
  * @param  dest: 需要读取的内容,需要转类型到uint8_t *.
  * @param  size: 需要些的长度(单位:Byte).
  */
void WL_Flash_Read(wl_flash_t *WL_Flash, uint32_t src_addr, uint8_t *dest, size_t size)
{
    uint32_t count = (size - 1) / WL_Flash->cfg.page_size;
    for (uint32_t i = 0; i < count; i++)
    {
        /* 要计算出虚拟地址,因为地址已经是乱的了. */
        uint32_t virt_addr = WL_Flash_calcAddr(WL_Flash, src_addr + i * WL_Flash->cfg.page_size);
        /* 真正要读出的是VA地址,每次读一个SubSector,然后下次继续轮奸. */
        BSP_QSPI_Read(&((uint8_t *)dest)[i * WL_Flash->cfg.page_size], WL_Flash->cfg.start_addr + virt_addr, WL_Flash->cfg.page_size);
    }
    /* 要计算出虚拟地址,因为地址已经是乱的了. */
    uint32_t virt_addr_last = WL_Flash_calcAddr(WL_Flash, src_addr + count * WL_Flash->cfg.page_size);
    /* 要读的大小,这里分两种情况,如果count为0,那么size就是size,因为后面没有减少任何东西,如果count不为0,就要减掉上面写的数据量,读剩下部分. */
    BSP_QSPI_Read(&((uint8_t *)dest)[count * WL_Flash->cfg.page_size], WL_Flash->cfg.start_addr + virt_addr_last, size - count * WL_Flash->cfg.page_size);
}

