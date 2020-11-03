/*
	Application for Amazfit Bip BipOS
	(C) Maxim Volkov  2020 <Maxim.N.Volkov@ya.ru>
	
	Приложение для управления музыкой с Amazfit Bip
	(C) trace 2020
	(C) ugers 2020	
	MOD
	Модифицированная версия приложения для управления музыкой. Изменены ресусры,добавлен эффект нажатия кнопок, добавлен статусбар
*/
#include "libbip.h"
#ifndef __APP_MUSIC_H__
#define __APP_MUSIC_H__

#define ARROW_UP_Y		5
#define ARROW_DOWN_Y	66

#define BTN_NONE		-1
#define	BTN_PLAY		0
#define	BTN_PAUSE		1
#define	BTN_NEXT		3
#define	BTN_PREV		4
#define	BTN_VOL_UP		5
#define	BTN_VOL_DOWN	6

/*
// номера ресурсов для 1.1.5.12
#define RES_PLAYER_BG		886
#define RES_PLAYER_BTN_BG	885
#define RES_PLAYER_EQ		887
#define RES_VOL_DOWN		888
#define RES_VOL_UP			890
#define RES_VOL_DOWN_BG		889
#define RES_VOL_UP_BG		891
#define RES_NEXT			892
#define RES_PREV			895
#define RES_PLAY			894
#define RES_PAUSE			893

*/

// номера ресурсов собственных
#define RES_PLAYER_BG		2
#define RES_PLAYER_BTN_BG	1
#define RES_PLAYER_EQ		3
#define RES_VOL_DOWN		4
#define RES_VOL_UP			5
#define RES_VOL_DOWN_BG		10
#define RES_VOL_UP_BG		11
#define RES_NEXT			6
#define RES_PREV			9
#define RES_PLAY			8
#define RES_PAUSE			7
#define RES_NEXT_BLACK		13
#define RES_PREV_BLACK		12
#define RES_PLAY_BLACK		15
#define RES_PAUSE_BLACK		14

// состояние плеера
#define STATE_PAUSED		0
#define STATE_PLAYING		1

// команды плеера
#define	CMD_PLAY		0
#define	CMD_PAUSE		1
#define	CMD_NEXT		3
#define	CMD_PREV		4
#define	CMD_VOL_UP		5
#define	CMD_VOL_DOWN	6
#define	CMD_AMC_ENABLE	0xE0
#define	CMD_AMC_DISABLE 0xE1	

// таймаут выхода
#define TIMEOUT_PLAYING	18000000	//	300 минут
#define TIMEOUT_PAUSED	1800000		//	30 минут


// состояние подсветки
#define BACKLIGHT_OFF		0
#define BACKLIGHT_ON		1


// структура данных для нашего экрана
struct app_data_ {
			void* 	ret_f;								//	адрес функции возврата
			Elf_proc_* proc;							//	указатель на данные процесса
			int 				state; 					//	состояние плеера
			int 				theme;					//	тема раскладка кнопок
			int 				last_tick;				//	отметка времени последней активности
			int 				last_bt_con;			//	последнее значение соединения BT
			int 				splash;					//	активный приветственный экран
			unsigned int		sec;
};

// template.c
void 	show_screen (void *return_screen);
void 	key_press_screen();
int 	dispatch_screen (void *param);
void 	screen_job();
void	draw_screen();

int send_music_command(int cmd);

#endif