#include <reg51.h>
#include <absacc.h>
#include "common.h"

extern void display_time(void);	// 闹钟时继续要显示当前时间
unsigned char code tune_div1[] = {0,220, 157, 140, 165, 196, 131, 117, 220, 157, 140, 165, 196, 131, 117, 220, 196,  70, 165};
unsigned char code tune_div2[] = {0,  4,  10,  10,   8,   4,   8,   8,   2,   5,   5,   4,   2,   4,   4,   2,   2,   5,   2};
// 利用两次分频，把921600Hz变成     低1    2    3    4     5    6    7 中1    2    3    4    5    6    7  高1    2    3    4
unsigned char code music_tn[] = {5,8,7,8,9,8,9,10,10,11,10,6,9,9,8,8,8,7,6,7,8,0};	// 音乐音调
unsigned char code music_tm[] = {4,4,2,2,4,2,2, 2, 4, 2, 2,4,2,2,4,2,2,4,2,2,4,8};	// 音调时间
unsigned char count_music;		// tune_div2[]中的值通过这个变量传输给timer1中断函数
sbit beep = P1^6;

void play_music(tn,tm) // 该函数播放单拍音符
{
	unsigned char i;
	if (tn == 0)	// tn = 0是空拍 
	{
		TR1 = 0;
		count_music = 0;
		beep = 1;	// BEEP =1为其默认状态
	}
	else 
	{
		TH1 = TL1 = 0xff - tune_div1[tn];  // 装载实现数字分频
		count_music = tune_div2[tn];
		TR1 = 1;
	}
	for (i=0;i<tm*8;i++)
			display_time();	// 这个函数大致耗时delayms(8);
	TR1 = 0;
	beep = 1;
	display_time();
	//display_time();
}

void timer1_music_int(void)	//闹钟音乐的timer1中断
{
	static unsigned char count = 0;
	count++;
	if (count == count_music)
	{
		count = 0;
		beep = ~beep;
	}
}

void BigClock(void)	// BigClock是一个按0x11才结束播放的音乐，极其难听，用作闹钟
{
	unsigned char i;
	ptm1 = timer1_music_int;
	while(1)
	for (i=0;i<22;i++)
	{	
		play_music(music_tn[i],music_tm[i]); // 播放单个音调
		if (getkeycode() == 0x11) 
		{
			while(getkeycode() == 0x11) display_time();	// 到松开时退出
			return;//break;
		}
	}
	TR1 = 0;
	beep = 1; // 关闭timer1和蜂鸣器
}
