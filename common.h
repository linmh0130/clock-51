#ifndef COMMON_H
#define COMMON_H

#ifndef UCHAR
#define UCHAR unsigned char
#endif /*UCHAR*/

#ifndef NULL
#define NULL 0
#endif

extern void (*ptm1)(void);
extern unsigned char code led_table[];
extern void delayms(unsigned char  p);
extern unsigned char getkeycode(void);

#endif /*COMMON_H*/
