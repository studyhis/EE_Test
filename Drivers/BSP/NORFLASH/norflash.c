/*******************
取地址
0xAB CD EF
   ↓  ↓  ↓

(addr >> 16) & 0xFF  = AB
(addr >> 8)  & 0xFF  = CD
 addr        & 0xFF  = EF

*/



#include "./BSP/SPI/spi.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/NORFLASH/norflash.h"

/**
 * @brief       初始化SPI NOR FLASH
 * @param       无
 * @retval      无
 */
void norflash_init(void)
{
    

    NORFLASH_CS_GPIO_CLK_ENABLE();      /* NORFLASH CS脚 时钟使能 */

    GPIO_InitTypeDef gpio_init_struct;
    gpio_init_struct.Pin = NORFLASH_CS_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull = GPIO_PULLUP;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(NORFLASH_CS_GPIO_PORT, &gpio_init_struct); /* CS引脚模式设置(复用输出) */

    

    spi2_init();                        /* 初始化SPI2 */
    spi2_read_write_byte(0xFF);        //清空数据寄存器(DR)的作用
	
	NORFLASH_CS(1);                     /* 片选拉高 */
   
}

/**
 * @brief       读1字节数据(读操作)
 * @param       无
 * @retval      无
 */
uint8_t norflash_read_data(uint32_t addr)
{
	uint8_t rec_data =0;
	//1.拉低片选线
	NORFLASH_CS(0);
	
	//2 发送读命令
	spi2_read_write_byte(0x03);
	//3 发送地址
	spi2_read_write_byte(addr >> 16);  // 第2字节：地址的高8位  A23~A16
	spi2_read_write_byte(addr >> 8);   // 第3字节：地址的中8位  A15~A8
	spi2_read_write_byte(addr);        // 第4字节：地址的低8位  A7~A0
	
	//4 读取数据                       发送空字节代表，收回来的就是读出的数据   
	rec_data = spi2_read_write_byte(0xFF);//选`0xFF`是行业惯例：全 1 的电平最安全，不会误触发任何指令，也不会干扰总线状态
	//5 拉高片选线
	NORFLASH_CS(1);
	
	return rec_data;
}

/**
 * @brief       因擦除函数里要等待空闲，所以封装该函数可多次调用
 * @param       状态寄1存器函数
 * @retval      无
 */
uint8_t norflash_read_sr(void)
{
	uint8_t rec_data =0;
	 
	NORFLASH_CS(0);
	spi2_read_write_byte(0x05);
	rec_data = spi2_read_write_byte(0xFF);
	NORFLASH_CS(1);
	
	return rec_data;
}

/**
 * @brief       擦除扇区函数
 * @param       无
 * @retval      无
 */
void norflash_erase_sector(uint32_t addr)
{
	
	//1 写使能 
	NORFLASH_CS(0);
	
	spi2_read_write_byte(0x06);
	
	NORFLASH_CS(1);
	
	//2 等待空闲-调用状态寄存器1函数
	while(norflash_read_sr() & 0x01);//若为1，忙；否则，空闲
	
	//3 发送擦除扇区命令
	NORFLASH_CS(0);
	spi2_read_write_byte(0x20);
	
	//4 发送地址
	spi2_read_write_byte(addr >> 16);  // 第2字节：地址的高8位  A23~A16
	spi2_read_write_byte(addr >> 8);   // 第3字节：地址的中8位  A15~A8
	spi2_read_write_byte(addr);        // 第4字节：地址的低8位  A7~A0
	NORFLASH_CS(1);
	
	//5 等待空闲
	while(norflash_read_sr() & 0x01);//若为1，忙；否则，空闲
}

/**
 * @brief       (写1字节数据)
 * @param       无
 * @retval      无
 */
void norflash_write_page(uint8_t data,uint32_t addr)
{
	//1 擦除扇区
	norflash_erase_sector(addr);
	//2 发送写使能
	NORFLASH_CS(0);
	
	spi2_read_write_byte(FLASH_WriteEnable);
	
	NORFLASH_CS(1);
	//3 发送页写命令
	NORFLASH_CS(0);
	spi2_read_write_byte(0x02);
	//4 发送地址
	spi2_read_write_byte(addr >> 16);  // 第2字节：地址的高8位  A23~A16
	spi2_read_write_byte(addr >> 8);   // 第3字节：地址的中8位  A15~A8
	spi2_read_write_byte(addr);        // 第4字节：地址的低8位  A7~A0
	
	//5 发送数据
	spi2_read_write_byte(data);
	NORFLASH_CS(1);
	//6 等待空闲
	while(norflash_read_sr() & 0x01);//若为1，忙；否则，空闲
}


/**
 * @brief       读取SPI FLASH（字符串）
 *   @note      在指定地址开始读取指定长度的数据
 * @param       pbuf    : 数据存储区
 * @param       addr    : 开始读取的地址(最大32bit)
 * @param       datalen : 要读取的字节数(最大65535)
 * @retval      无
 */
void norflash_read(uint8_t *pbuf, uint32_t addr, uint16_t datalen)
{
    uint16_t i;

    NORFLASH_CS(0);
	
    spi2_read_write_byte(FLASH_ReadData);       /* 发送读取命令  FLASH_ReadData在头文件定义*/
    //发送地址
	spi2_read_write_byte(addr >> 16);  // 第2字节：地址的高8位  A23~A16
	spi2_read_write_byte(addr >> 8);   // 第3字节：地址的中8位  A15~A8
	spi2_read_write_byte(addr);        // 第4字节：地址的低8位  A7~A0
    
    for(i=0;i<datalen;i++)
    {
        pbuf[i] = spi2_read_write_byte(0XFF);   /* 循环读取 */
    }
    
    NORFLASH_CS(1);
}

/**
 * @brief       SPI在一页(0~65535)内写入少于256个字节的数据
 *   @note      在指定地址开始写入最大256字节的数据
 * @param       pbuf    : 数据存储区
 * @param       addr    : 开始写入的地址(最大32bit)
 * @param       datalen : 要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!
 * @retval      无
 */
void norflash_write_page_for(uint8_t *pbuf, uint32_t addr, uint16_t datalen)
{
    uint16_t i;
	
	
   //1 写使能 
	NORFLASH_CS(0);
	spi2_read_write_byte(0x06);
	NORFLASH_CS(1);

    NORFLASH_CS(0);
    spi2_read_write_byte(FLASH_PageProgram);    /* 发送写页命令 */
    //发送地址
	spi2_read_write_byte(addr >> 16);  // 第2字节：地址的高8位  A23~A16
	spi2_read_write_byte(addr >> 8);   // 第3字节：地址的中8位  A15~A8
	spi2_read_write_byte(addr);        // 第4字节：地址的低8位  A7~A0

    for(i=0;i<datalen;i++)
    {
        spi2_read_write_byte(pbuf[i]);          /* 循环写入 */
    }
    
    NORFLASH_CS(1);
	//等待空闲
    while(norflash_read_sr() & 0x01);//若为1，忙；否则，空闲
}

















