#include <reg51.h>
#include <absacc.h>
#include "common.h"

unsigned char code led_table[]={0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
// 十六进制字符表				0     1     2     3     4     5     6     7 
								0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x00};
//								8     9     A     b     C     d      E    F

void (*ptm1)(void) = NULL; // ptm1是Timer1中断使用的函数，指向函数的指针便于重载

void delayms(unsigned char  p)				//延时函数，大致大于1ms(11.0592MHz)
{
	unsigned char  i,j; 				
	for(;p>0;p--) 				
		for(i=5;i>0;i--)
			for(j=50;j>0;j--);
}

unsigned char getkeycode(void)
{
	unsigned char col = 0x00;		// 列码
	unsigned char scancode = 0x01;	// 行扫描码
	unsigned char keycode = 0x00;	// 键码，供返回

	XBYTE[0X8000] = 0xff;			// 初始化，为矩阵加高电平
	col = XBYTE[0X8000] & 0x0f;		// 读键盘，从电路图看出，读的地方是一个双向缓冲器
	if (col == 0x00)
	{	// 没键被按时是0
		keycode = 0x00;
	}
	else
	{	// 有键被按，需扫描确定位置
		while((scancode&0x0f)!=0)
		{
			XBYTE[0X8000] = scancode;	// 单行单行扫描
			if ((XBYTE[0x8000]&0x0f) == col) break;	// 找到和之前相同col的地方，即被按键
			scancode = scancode <<1;	// 没找到则移位准备下次扫
		}
		keycode = (scancode<<4)|col;	// 行码在高4位，列码在低4位
	}
	return keycode;
}
