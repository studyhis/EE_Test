#ifndef __norflash_H
#define __norflash_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* NORFLASH 片选 引脚 定义 */

#define NORFLASH_CS_GPIO_PORT           GPIOB
#define NORFLASH_CS_GPIO_PIN            GPIO_PIN_12
#define NORFLASH_CS_GPIO_CLK_ENABLE()   do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)   /* PB口时钟使能 */

/******************************************************************************************/

/* NORFLASH 片选信号 */
#define NORFLASH_CS(x)      do{ x ? \
                                  HAL_GPIO_WritePin(NORFLASH_CS_GPIO_PORT, NORFLASH_CS_GPIO_PIN, GPIO_PIN_SET) : \
                                  HAL_GPIO_WritePin(NORFLASH_CS_GPIO_PORT, NORFLASH_CS_GPIO_PIN, GPIO_PIN_RESET); \
                            }while(0)

							
/* FLASH芯片列表 */
#define NM25Q128    0x5218  /* 诺存NM25Q128芯片ID */
							
/* 指令表 */							
#define FLASH_ReadData              0x03
#define FLASH_PageProgram           0x02
#define FLASH_WriteEnable           0x06 

							
void norflash_init(void);
uint8_t norflash_read_data(uint32_t addr);//读1个字节数据
void norflash_erase_sector(uint32_t addr);//擦除函数
void norflash_write_page(uint8_t data,uint32_t addr);//写1字节函数
uint8_t norflash_read_sr(void);//读状态寄存器1函数
							
void norflash_read(uint8_t *pbuf, uint32_t addr, uint16_t datalen);							
void norflash_write_page_for(uint8_t *pbuf, uint32_t addr, uint16_t datalen);							
#endif















