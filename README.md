# 好用的NOR Flash磨损平衡算法

------

因为SPI Flash也是有寿命的,所以要磨损平衡.一般Flash自己擦改写,总在一个位置嘛,容易死,不总在又不知道怎么写算法,我现在写了一个简单的算法,有个算法例子和核心文件.

需要移植以下内容.

> * SPI Flash的写(Multi-Byte),读(Multi-Byte),擦(SubSector).
> * Malloc的实现,我用FreeRTOS了.
> * CRC的实现,一般单片机有硬件支持.

欢迎访问我的网站：

### [TaterLi 个人博客](https://www.lijingquan.net/)

> 这个程序不具备完全的时间确定性.
