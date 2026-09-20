#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"

#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"
#include "./BSP/NORFLASH/norflash.h"



int main(void)
{
    uint8_t key;
    uint16_t i = 0;
//    uint8_t rec_data =0;//读单个字节数据存储
    
	uint8_t write_buf[] = "云南农业大学！Hello NM25Q128! SPI Flash Test";
	uint8_t read_buf[50]; // 存放读出的数据
	/* 测试使用的Flash地址 0x123456 */
    const uint32_t test_addr = 0x123456;

    HAL_Init();                         /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* 设置时钟, 72Mhz */
    delay_init(72);                     /* 延时初始化 */
    usart_init(115200);                 /* 串口初始化为115200 */

    led_init();                         /* 初始化LED */
    lcd_init();                         /* 初始化LCD */
    key_init();                         /* 初始化按键 */
    norflash_init();                    /* 初始化NORFLASH */

    

   

    while (1)
    {
        key = key_scan(0);

        if (key == KEY1_PRES) /* KEY1按下,写入 */
        {
			printf("开始擦除扇区...\r\n");
            norflash_erase_sector(test_addr);   // 擦除test_addr所在4KB扇区
            
//          norflash_write_page('a',0x123456);//写1个字节
//			printf("write finish!\n");
			
            /* 写入字符串，注意长度不能超过256字节 */
            norflash_write_page_for(write_buf, test_addr, sizeof(write_buf));
        
			printf("write finish!\n");
		}

        if (key == KEY0_PRES) /* KEY0按下,读取字符串并显示 */
        {
            
//          rec_data = norflash_read_data(0x123456);//读1个字节
//			printf("read data:%c \r\n",rec_data);
			
			//读字符串
			norflash_read(read_buf, test_addr, sizeof(write_buf));
			printf("read String:%s \r\n",read_buf);
			
		}

        i++;//计数器

        if (i == 20)
        {
            LED0_TOGGLE(); /* LED0闪烁 */
//			delay_ms(500);//加延时来控制两灯的交替亮
			LED1_TOGGLE();
            i = 0;
        }

        delay_ms(10);
    }
}

