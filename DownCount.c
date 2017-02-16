#include <reg51.h>
#include <absacc.h>
#include "common.h"

#define PUSH(KEY) delayms(10);if (getkeycode() != (KEY)) break; \
				  while(getkeycode() == (KEY)) display_setdown(sm) // 消抖，虽然我觉得挺鸡肋

extern unsigned int count0;
unsigned int count1 = 0;
sbit beep = P1^6;
UCHAR dhour = 0,dmin = 0,dsec =0; // 倒计时计时变量
bit ring_flag = 0;
void display_down(void)
{
	XBYTE[0X8000] = 0X80;
	XBYTE[0X9000] = led_table[0X0D]; // 字母d表明在倒数
	delayms(1);
	XBYTE[0X8000] = 0X40;
	XBYTE[0X9000] = 0X00; // 第二个地方空出
	delayms(1);
}
void display_dhour(void)
{
	XBYTE[0x8000] = 0x20;
	XBYTE[0X9000] = led_table[dhour/10];
	delayms(1);
	XBYTE[0x8000] = 0x10;
	XBYTE[0X9000] = led_table[dhour%10] | 0x80;
	delayms(1);
}
void display_dmin(void)
{
	XBYTE[0x8000] = 0x08;
	XBYTE[0X9000] = led_table[dmin/10];
	delayms(1);
	XBYTE[0x8000] = 0x04;
	XBYTE[0X9000] = led_table[dmin%10] | 0x80;
	delayms(1);
}
void display_dsec(void)
{
	XBYTE[0x8000] = 0x02;
	XBYTE[0X9000] = led_table[dsec/10];
	delayms(1);
	XBYTE[0x8000] = 0x01;
	XBYTE[0X9000] = led_table[dsec%10];
	delayms(1);
}

void display_all(void)
{
	
	display_down();	// 依次显示
	display_dhour();
	display_dmin();
	display_dsec();
}

void display_setdown(UCHAR sm)
{
	display_down();
	if(count0<1800) display_all();
	else
		switch(sm)
		{
			case 0: display_down();	// 小时闪烁
					XBYTE[0x8000] = 0x30;
					XBYTE[0X9000] = 0X00;
					delayms(2);display_dmin();display_dsec();break;
			case 1: display_down();	display_dhour();// 分钟闪烁
					XBYTE[0x8000] = 0x0c;
					XBYTE[0X9000] = 0X00;
					delayms(2);display_dsec();break;
			case 2: display_down();display_dhour();display_dmin();	// 秒闪烁
					XBYTE[0x8000] = 0x03;
					XBYTE[0X9000] = 0X00;
					delayms(2);break;
			default:display_all();break;

		}	
}

void ring(UCHAR sm)
{
	UCHAR i,j;
	for (i=0;i<10;i++)	// 正反各5次
	{
		beep = ~beep;
		for (j=0x20;j>0;j--)
			display_setdown(sm);
	}
}

void timer1_downcount_int(void)
{ 
	count1++;
	if (count1 == 3600)
	{
		count1 =0;
		if (dsec>0) dsec--;
		else
		{
			if (dmin>0) {dmin--;dsec=59;} // 倒计要退位
			else
			{
				if (dhour>0){dhour--;dmin=59;dsec=59;}
				else // 倒计时结束
				{TR1 =0; ring_flag=1;};
			}
		}
	}
}

void DownCount(void) // 倒计时主程序
{
	UCHAR sm = 0; // sm =0,改时; =1,改分; =2,改秒; =3,倒计时及预备
	              // sm =3后，按0x14倒计时，按0x18暂停
	ptm1 = timer1_downcount_int;
	while(1)
	{
		display_setdown(sm);
		switch(getkeycode())
		{
			case 0x11: PUSH(0x11);
					   TR1 =0;dhour = dmin = dsec =0;count1=0;return; // 0x11是退出
			case 0x12: PUSH(0x12);
					   if (sm<3) sm++;else if (TR1==0)sm =0;break;	// 0x12设定时间推动sm
			case 0x14: PUSH(0x14); // 0x14加数和开始倒计时
					   switch(sm)
					   {
					   	case 0: if (dhour<23)dhour++;break;
						case 1: if (dmin<59)dmin++;break;
					   	case 2: if (dsec<59)dsec++;break;
						case 3: TR1 =1;break;
						default:break;
					   }break;
			case 0x18: PUSH(0x18); // 0x18减数和结束倒计时
					   switch(sm)
					   {
					   	case 0: if (dhour>0)dhour--;break;
						case 1: if (dmin>0)dmin--;break;
					   	case 2: if (dsec>0)dsec--;break;
						case 3: TR1 =0;break;
						default:break;
					   }break;
			default:break;
		}
		if (ring_flag ==1) {ring_flag = 0;ring(sm);}	
	}
}
