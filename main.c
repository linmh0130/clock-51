#include <reg51.h>
#include <absacc.h>
#include "common.h"
#include "SecCount.h"
#include "music.h"

extern void DownCount(void); // 倒计时主程序，不想为倒计时专门写头文件，我懒

UCHAR sec = 0, min = 0, hour = 0;
UCHAR day=1, month = 1, year = 17; // 以2001年为起点元年，这有个好处，就是400年内不会有整百年是闰年的情况
								   // 400年之后？你觉得400年后人类还存在吗:) 而且我这么定义也只支持到2255年:)
								   // 2255年还有没有人类都说不定呢:) 有的话也不会再用这可怜的时钟了:)
								   // 哦，不对，他还在。+1s o-o
UCHAR amin = 0, ahour=0;	// 闹钟的时和分
bit alarm_flag = 0;			// 闹钟到达标志
bit alarm_on_flag = 0;		// 闹钟开关标志

UCHAR month_table[]={0,31,28,31,30,31,30,31,31,90,31,30,31}; // 月份表，从1开始为1月。要让程序修改二月
unsigned int count0 = 0;	// count0 作为timer0的计数结果公用

// 以下是分解显示年月日时分秒的函数，用于缩减代码长度
void display_year(void)
{
	XBYTE[0X8000] = 0X80;
	XBYTE[0X9000] = led_table[2];	// 最高位是2
	delayms(1);
	XBYTE[0x8000] = 0x40;
	XBYTE[0x9000] = led_table[year/100]; // 年的百位
	delayms(1);
	XBYTE[0x8000] = 0x20;
	XBYTE[0x9000] = led_table[(year%100)/10]; // 年的十位
	delayms(1);
	XBYTE[0x8000] = 0x10;
	XBYTE[0x9000] = led_table[year%10]; // 年的个位
	delayms(1);
}

void display_month(void)
{
	XBYTE[0x8000] = 0x08;
	XBYTE[0x9000] = led_table[month/10];
	delayms(1);
	XBYTE[0x8000] = 0x04;
	XBYTE[0x9000] = led_table[month%10];
	delayms(1);
}

void display_day(void)
{
	XBYTE[0x8000] = 0x02;
	XBYTE[0x9000] = led_table[day/10];
	delayms(1);
	XBYTE[0x8000] = 0x01;
	XBYTE[0x9000] = led_table[day%10];
	delayms(1);
}

void display_hour(void)
{
	XBYTE[0X8000] = 0X80;
	XBYTE[0X9000] = led_table[hour/10];
	delayms(1);
	XBYTE[0X8000] = 0X40;
	XBYTE[0X9000] = led_table[hour%10];
	delayms(1);
	XBYTE[0X8000] = 0X20;
	XBYTE[0X9000] = 0x40;
	delayms(1);
}
void display_min(void)
{
	XBYTE[0X8000] = 0X10;
	XBYTE[0X9000] = led_table[min/10];
	delayms(1);
	XBYTE[0X8000] = 0X08;
	XBYTE[0X9000] = led_table[min%10];
	delayms(1);
	XBYTE[0X8000] = 0X04;
	XBYTE[0X9000] = 0x40;
	delayms(1);
}
void display_sec(void)
{
	XBYTE[0X8000] = 0X02;
	XBYTE[0X9000] = led_table[sec/10];
	delayms(1);
	XBYTE[0X8000] = 0X01;
	XBYTE[0X9000] = led_table[sec%10];
	delayms(1);
}

void display_date(void)	   // 显示日期
{
	unsigned char date[12];
	unsigned char i,j,k;

	for (i=0;i<27;i++)	  // 循环3次真TM烦
	{
		for(k=0x40;k>0;k--)
		{
			date[0] = led_table[2];date[1] = led_table[year/100];
			date[2] = led_table[(year%100)/10];	date[3] = led_table[year%10];
			date[4] = 0x08;date[5] = led_table[month/10];
			date[6] = led_table[month%10]; date[7] = 0x08;
			date[8] = led_table[day/10]; date[9] = led_table[day%10];
			date[10] = date[11] = 0x00;
			for(j=0;j<8;j++)
			{
				XBYTE[0X8000] = 0X80>>j;
				XBYTE[0X9000] = date[(i+j)%12];
				delayms(1);
				XBYTE[0x9000] = 0x00;
			}
			if (getkeycode() == 0x11)  // 按0x11退出去
			{
				while(getkeycode() == 0x11);
				return;
			}
		}
		
	}
}

void change_display_date(void)
{
	display_year();
	display_month();
	display_day();
}

void display_time(void)	// 显示时间字符的程序。专门提出，这样在显示、消抖等场合都能用
{
	display_hour();
	display_min();
	display_sec();
}
//------------------------------------------------

void change_display(UCHAR sm) // 修改时间时display单独列出
{
	switch (sm)
	{
	case 0: if (count0<1800) change_display_date();  // 实现年数字要闪烁
			else{
				XBYTE[0x8000] = 0xf0;
				XBYTE[0x9000] = 0x00;
				delayms(4);
				display_month();
				display_day();
			}break;
	case 1: if (count0<1800) change_display_date();  // 实现月数字要闪烁
			else{
				display_year();
				XBYTE[0x8000] = 0x0C;
				XBYTE[0x9000] = 0x00;
				delayms(2);
				display_day();
			}break;
	case 2: if (count0<1800) change_display_date();  // 实现日数字要闪烁
			else{
				display_year();
				display_month();
				XBYTE[0x8000] = 0x03;
				XBYTE[0x9000] = 0x00;
				delayms(2);
			}break;
	case 3: if (count0<1800) display_time();  // 实现时数字要闪烁
			else{
				XBYTE[0X8000] = 0X30;
				XBYTE[0X9000] = 0x00;
				delayms(2);
				display_min();
				display_sec();
			}break;
	case 4: if (count0<1800) display_time();  // 实现分数字要闪烁
			else{
				display_hour();
				XBYTE[0X8000] = 0X0C;
				XBYTE[0X9000] = 0x00;
				delayms(2);
				display_sec();
			}break;
	case 5: if (count0<1800) display_time();  // 实现秒数字要闪烁
			else{
				display_hour();
				display_min();
				XBYTE[0X8000] = 0X03;
				XBYTE[0X9000] = 0x00;
				delayms(2);
			}break; 
	default: break;
	}		
}

void change_leap_year(void)	//处理闰年的函数。在每次年份变化时必须使用
{
	if ((year%4 == 0) && (year%100 !=0)) 
	{month_table[2] = 29;}
	else
	{month_table[2] = 28;}
}

#define PUSH_CHGTIM(KEY) delayms(10);if (getkeycode() != (KEY)) break; \
				  while(getkeycode() == (KEY)) change_display(sm) // 为改时间专设消抖
void change_time(void)	// 调时间程序。要能调年月日时分
{
	UCHAR sm = 0;	// sm 即 state machine
			  		// sm =0,改年; =1,改月; =2,改日; =3,改时; =4,改分; =5改秒
	while(sm < 6) // sm =6即退出
	{
		change_display(sm);
		switch(getkeycode())
		{
			case 0x12: PUSH_CHGTIM(0x12); 
					   sm++;
					   break;

			case 0x14: PUSH_CHGTIM(0x14); // 0x14键用于加
					   switch(sm)
					   {
					   	case 0: if (year <255)year++;
								change_leap_year();
								break;
						case 1: month++;
								if (month >12) month =1;	//溢出复位
								break;
						case 2: day++;
								if (day > month_table[month]) day = 1;
								break;
						case 3: hour++;
								if (hour == 24) hour =0;
								break;
						case 4: min++;
								if (min == 60) min =0;
								break;
						default :break;
					   }
					   break;

			case 0x18: PUSH_CHGTIM(0x18); // 0x18键用于减
					   switch(sm)
					   {
					   	case 0: if (year>1) year--;change_leap_year();break;	  // 防止减溢出
						case 1: if (month>1) month--;break;
						case 2: if (day>1) day--;break;
						case 3: if (hour>0) hour--;break;
						case 4: if (min>0) min--;break;
						case 5: sec=0;break;	// 改秒只有清零一个选项
						default :break;
					   }break;
			default: break;
		} 
	}
}

void AddDay(void)
{
	day++;
	if(day>month_table[month])
	{
		day = 1;
		month++;
		if (month>12)
		{
			month = 1;
			year++;change_leap_year();
		}
	}
}

void dis_set_alarm(UCHAR sm)
{
	XBYTE[0X8000] = 0X80;
	XBYTE[0X9000] = led_table[0X0A]; //显示字母A
	delayms(1);
	XBYTE[0X8000] = 0X40;
	if(alarm_on_flag == 0)	XBYTE[0X9000] = led_table[0X0F]; //闹钟关显示字母F
	else XBYTE[0X9000] = 0X37;	// 闹钟开显示字母n
	delayms(1);
	XBYTE[0X8000] = 0X04;
	XBYTE[0X9000] = 0X40; // 显示-
	delayms(1);
	if (count0<1800)
	{
		XBYTE[0X8000] = 0X10;
		XBYTE[0X9000] = led_table[ahour/10];
		delayms(1);
		XBYTE[0X8000] = 0X08;
		XBYTE[0X9000] = led_table[ahour%10];
		delayms(1);
		XBYTE[0X8000] = 0X02;
		XBYTE[0X9000] = led_table[amin/10];
		delayms(1);
		XBYTE[0X8000] = 0X01;
		XBYTE[0X9000] = led_table[amin%10];
		delayms(1);
	}
	else
	{
		if (sm ==0)
		{
			XBYTE[0X8000] = 0X18;
			XBYTE[0X9000] = 0x00;
			delayms(2);		//闹钟小时闪烁
			XBYTE[0X8000] = 0X02;
			XBYTE[0X9000] = led_table[amin/10];
			delayms(1);
			XBYTE[0X8000] = 0X01;
			XBYTE[0X9000] = led_table[amin%10];
			delayms(1);
		}
		else
		{
			XBYTE[0X8000] = 0X10;
			XBYTE[0X9000] = led_table[ahour/10];
			delayms(1);
			XBYTE[0X8000] = 0X08;
			XBYTE[0X9000] = led_table[ahour%10];
			delayms(1);
			XBYTE[0X8000] = 0X03;
			XBYTE[0X9000] = 0x00;
			delayms(2);	// 分钟小时闪烁
		}
	} 
}

#define PUSH_CHGALM(KEY) delayms(10);if (getkeycode() != (KEY)) break; \
				  while(getkeycode() == (KEY)) dis_set_alarm(sm) // 为改时间专设消抖
void SetAlarm(void)
{
	UCHAR sm =0; //sm=0,设闹钟小时;sm=1,设闹钟分钟
	while (sm<2) //sm=2就退出
	{
		dis_set_alarm(sm);
		switch(getkeycode())
		{
			case 0x11: PUSH_CHGALM(0x11);
					   alarm_on_flag = ~alarm_on_flag;break;	// 开关闹钟
			case 0x12: PUSH_CHGALM(0x12);
					   sm++;break;
			case 0x14: PUSH_CHGALM(0x14);
					   if (sm==0)
					   {ahour++;if(ahour == 24)ahour=0;}
					   else	// sm=1
					   {amin++;if (amin == 60) amin =0;}
					   break;
			case 0x18: PUSH_CHGALM(0x18);
					   if (sm==0)
					   {if(ahour >0)ahour--;}
					   else	// sm=1
					   {if (amin>0) amin--;}
					   break;	
		}
	}

}

#define PUSH_MAIN(KEY) delayms(10);if (getkeycode() != (KEY)) break; \
				  while(getkeycode() == (KEY)) display_time() // 为主函数专设消抖
void main(void)
{
	EA = 1;
	TMOD = 0X22;	   // MODE 2
	ET0 = ET1 =1;
	//TH0 = TL0= 0X00;
	TR0 = 1;		   // START TIMING

	while(1)
	{
		display_time();
		if(alarm_flag==1){alarm_flag=0;BigClock();} // 响闹钟前关掉置位
		switch(getkeycode())
		{
			case 0x11: PUSH_MAIN(0x11);
					   display_date();break;// 0x11显示日期
			case 0x12: PUSH_MAIN(0x12);
					   change_time();SetAlarm();break; // 0x12修改时间
			case 0x14: PUSH_MAIN(0x14);
					   SecCount();break;	// 0x14秒表
			case 0x18: PUSH_MAIN(0x18);
					   DownCount();break;  
			default: break;
		}										 
	}
}

void timer0_int(void) interrupt 1
{
	count0++;
	if (count0 == 3600) // 11059200/12/256 = 3600 即每秒会中断3600次。这样+1s
	{
		count0=0;
		sec++;
		if (sec == 60)
		{
			sec = 0;
			min++;
			if (min ==60)
			{
				min = 0;
				hour++;
				if (hour == 24)
				{
					hour =0;
					AddDay();
				}
			}
		}
		if((alarm_on_flag==1) && (hour == ahour) && (min == amin) && (sec == 0)) alarm_flag=1;
				// 置闹钟开，由main函数响闹钟
	}
}

void timer1_int(void) interrupt 3
{
	ptm1(); //调用被指向函数
}
