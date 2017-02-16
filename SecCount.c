#include <reg51.h>
#include <absacc.h>
#include "common.h"
#include "SecCount.h"

#define MEM 4	// 存储时间数
#define PUSH(KEY) delayms(10);if (getkeycode() != (KEY)) break; \
				  while(getkeycode() == (KEY)) display_sc(rank,chour[rank],cmin[rank],csec[rank],cdsec[rank]) //按键暂留

UCHAR code rank_tab[] = {0x00,0x01,0x40,0x08};	//显示第几个的标记
UCHAR chour[MEM]={0,0,0,0}, cmin[MEM]={0,0,0,0}, csec[MEM] = {0,0,0,0}, cdsec[MEM]={0,0,0,0};
unsigned int sc_count = 0; //计数

void display_sc(UCHAR rank, UCHAR hour, UCHAR min, UCHAR sec, UCHAR dsec)
{	// rank=0则正计时，不然就显示次序
	XBYTE[0X8000] = 0X80;
	XBYTE[0X9000] = rank_tab[rank];
	delayms(1);
	XBYTE[0X8000] = 0X40;
	XBYTE[0X9000] = led_table[hour/10];
	delayms(1);
	XBYTE[0X8000] = 0X20;
	XBYTE[0X9000] = led_table[hour%10] | 0x80;
	delayms(1);
	XBYTE[0X8000] = 0X10;
	XBYTE[0X9000] = led_table[min/10];
	delayms(1);
	XBYTE[0X8000] = 0X08;
	XBYTE[0X9000] = led_table[min%10] | 0x80;
	delayms(1);
	XBYTE[0X8000] = 0X04;
	XBYTE[0X9000] = led_table[sec/10];
	delayms(1);
	XBYTE[0X8000] = 0X02;
	XBYTE[0X9000] = led_table[sec%10] | 0x80;
	delayms(1);
	XBYTE[0X8000] = 0X01;
	XBYTE[0X9000] = led_table[dsec];
	delayms(1);
}

void clear(void)
{
	UCHAR i;
	TR1=0;TL1=0;
	for (i=0;i<MEM;i++)
	{
		chour[i] = 0;
		cmin[i] = 0;
		csec[i] = 0;
		cdsec[i] = 0;
	}
	sc_count = 0;
}

void sc_timer1_int(void) /*interrupt 3*/	// 给秒表用的timer1中断
{
	sc_count++;
	if (sc_count == 360)
	{
		sc_count =0;cdsec[0]++;
		if(cdsec[0] ==10)
		{
			cdsec[0] = 0;csec[0]++;
			if(csec[0] ==60)
			{
				csec[0] = 0;cmin[0]++;
				if(cmin[0] ==60)
				{
					cmin[0]=0;chour[0]++;
					if (chour[0] ==24) chour[0]=0;
				}
			}
		}
	}
}
				  
void SecCount(void)
{
	UCHAR sm = 0;	// 状态机
	 				// sm =0,准备计时; =1,正计时;=2,结束计时回看.
	static UCHAR rank = 0,crank = 1; // crank标识记第几个,用static未不清零退出预备
	ptm1 = sc_timer1_int;	// 将ptm1置为秒表功能对应中断函数
	while(1)
	{
		display_sc(rank,chour[rank],cmin[rank],csec[rank],cdsec[rank]);
		switch(getkeycode())
		{													  
			case 0x11: PUSH(0X11);
					   switch(sm)	// 0x11 用于开始或结束计时(推动sm)
					   {
					   	case 0: TR1=1;sm++;break;
						case 1: TR1=0;sm++;break;  // 1计时
						case 2: clear();sm=0;rank=0;crank=1;break;	// 2回看
					   }break;
			case 0x12: PUSH(0X12);			// 0x12 记录和回看
					   if((sm==1)&& TR1 && (crank<MEM)) // sm==1记录
					   {
					   	chour[crank] = chour[0];
						cmin[crank] = chour[0];
						csec[crank] = csec[0];
						cdsec[crank] = cdsec[0];
						crank++;
					   }
					   else if (sm==2)
					   {
					   	rank++;
						if(rank==MEM) rank=0;	// 溢出返回
					   }break;
			case 0x14: PUSH(0X14);TR1=0;clear();sm=0;rank=0;crank=1;return;	// 0x14 退出
		}
	}
}
