#ifndef MUSIC_H
#define MUSIC_H

extern unsigned char code music_tn[];
unsigned char code music_tm[];
extern void play_music(tn,tm);
extern void timer1_music_int(void);
extern void BigClock(void);

#endif /*MUSIC_H*/
