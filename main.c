/*
         Application for Amazfit Bip BipOS
	(C) Maxim Volkov  2019 <Maxim.N.Volkov@ya.ru>
	
	Приложение для управления музыкой с Amazfit Bip
	
	v.1.0
	- релиз управления музыкой
	
*/
//#define BipEmulator
//#define Day_Right
//#define Day_Full_Left
//#define Day_Full_Right
//#define Day_Short_No_Year_Left
//#define Day_Short_No_Year_Right
#ifdef BipEmulator
	// статусы функции get_app_state
	#define APP_STATE_BT_CON		0x200
	#include "libbip.h"
#else
	#include <libbip.h>
#endif
#include "main.h"

//	структура меню экрана - для каждого экрана своя
struct regmenu_ screen_data = {
						55,							//	номер главного экрана, значение 0-255, для пользовательских окон лучше брать от 50 и выше
						1,							//	вспомогательный номер экрана (обычно равен 1)
						0,							//	0
						dispatch_screen,			//	указатель на функцию обработки тача, свайпов, долгого нажатия кнопки
						key_press_screen, 			//	указатель на функцию обработки нажатия кнопки
						screen_job,					//	указатель на функцию-колбэк таймера 
						0,							//	0
						show_screen,				//	указатель на функцию отображения экрана
						0,							//	
						0							//	долгое нажатие кнопки
					};
					
int main(int param0, char** argv){	//	здесь переменная argv не определена
	 (void) argv;
	show_screen((void*) param0);
	
}

void show_screen (void *param0){
struct app_data_**  app_data_p = (struct app_data_ **)get_ptr_temp_buf_2();    //  указатель на указатель на данные экрана
struct app_data_ *	app_data;					//	указатель на данные экрана


// проверка источника запуска процедуры
if ( (param0 == *app_data_p) && get_var_menu_overlay()){ // возврат из оверлейного экрана (входящий звонок, уведомление, будильник, цель и т.д.)

	app_data = *app_data_p;					//	указатель на данные необходимо сохранить для исключения 
											//	высвобождения памяти функцией reg_menu
	#ifdef BipEmulator
		*app_data_p = (app_data_*)NULL;						//	обнуляем указатель для передачи в функцию reg_menu
	#else
		*app_data_p = NULL;						//	обнуляем указатель для передачи в функцию reg_menu	
	#endif
	// 	создаем новый экран, при этом указатель temp_buf_2 был равен 0 и память не была высвобождена	
	reg_menu(&screen_data, 0); 				// 	menu_overlay=0
	
	*app_data_p = app_data;						//	восстанавливаем указатель на данные после функции reg_menu
	
	//   здесь проводим действия при возврате из оверлейного экрана: восстанавливаем данные и т.д.
	
	
} else { // если запуск функции произошел впервые т.е. из меню 

	// создаем экран (регистрируем в системе)
	reg_menu(&screen_data, 0);

	// выделяем необходимую память и размещаем в ней данные, (память по указателю, хранящемуся по адресу temp_buf_2 высвобождается автоматически функцией reg_menu другого экрана)
	*app_data_p = (struct app_data_ *)pvPortMalloc(sizeof(struct app_data_));
	app_data = *app_data_p;		//	указатель на данные
	
	// очистим память под данные
	_memclr(app_data, sizeof(struct app_data_));
	
	//	значение param0 содержит указатель на данные запущенного процесса структура Elf_proc_
	#ifdef BipEmulator
		app_data->proc = (Elf_proc_*)param0;
	#else
		app_data->proc = param0;
	#endif

	
	// запомним адрес указателя на функцию в которую необходимо вернуться после завершения данного экрана
	if ( param0 && app_data->proc->elf_finish ) 			//	если указатель на возврат передан, то возвоащаемся на него
		app_data->ret_f = app_data->proc->elf_finish;
	else					//	если нет, то на циферблат
		app_data->ret_f = show_watchface;
	
	// здесь проводим действия которые необходимо если функция запущена впервые из меню: заполнение всех структур данных и т.д.
	
	app_data->state = STATE_PAUSED;
	app_data->theme = 0;
	app_data->last_tick = get_tick_count();	//	установим последнюю отметку времени активности на текущее время
#ifdef BipEmulator
	app_data->last_bt_con = 1;
#else
	app_data->last_bt_con = check_app_state(APP_STATE_BT_CON);
#endif // !BipEmulator

	//	если запуск был из быстрого меню, не включать экран приветствия
  if ( get_left_side_menu_active() ) {
	  app_data->splash	=	0;	// выключаем экран приветствия
  } else {
	  app_data->splash	=	1;	// включаем экран приветствия
  }
}
// здесь выполняем отрисовку интерфейса, обновление (перенос в видеопамять) экрана выполнять не нужно


// экран гаснет после таймаута, загорается при нажатии на экран
set_display_state_value(8, 1);
//set_display_state_value(4, 1);	//	подсветка
set_display_state_value(2, 1);

draw_screen();

// при необходимости ставим таймер вызова screen_job в мс
set_update_period(1,  app_data->splash?2000:500); // обновляем экран через 2 секунды если активен экран приветствия

}

void key_press_screen(){
struct app_data_**  app_data_p = (struct app_data_ **)get_ptr_temp_buf_2();    //  указатель на указатель на данные экрана
struct app_data_ *	app_data = *app_data_p;				//	указатель на данные экрана

send_music_command(CMD_AMC_DISABLE);

//	если запуск был из быстрого меню, при нажатии кнопки выходим на циферблат
if ( get_left_side_menu_active() ) 		
    app_data->proc->ret_f = show_watchface;


// вызываем функцию возврата (обычно это меню запуска), в качестве параметра указываем адрес функции нашего приложения
show_menu_animate(app_data->ret_f, (unsigned int)show_screen, ANIMATE_RIGHT);	
};

void draw_time(){
		set_fg_color(COLOR_BLACK);
		draw_filled_rect(0,55,176,120);
		// отрисовка значений
		char clock_time[8]; 			//	текст время		12:34
		char data[12]; 			//	текст дата		день.месяц.год
		struct datetime_ dt;
		get_current_date_time(&dt);
		_sprintf(clock_time, "%02d:%02d", dt.hour, dt.min);
#ifndef Day_Short_No_Year
		unsigned short year_short = dt.year-2000;
		_sprintf(data, "%02d.%02d.%02d", dt.day, dt.month, year_short);
#endif
		
		char text_buffer[24];


#if defined Day_Full_Left || defined Day_Full_Right
		char *weekday_string_ru[] = {"??", "Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота", "Воскресенье"};
		char *weekday_string_en[] = {"??", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
		char *weekday_string_it[] = {"??", "Lunedì", "Martedì", "Mercoledì", "Giovedì", "Venerdì", "Sabato", "Domenica"};
		char *weekday_string_fr[] = {"??", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi", "Dimanche"};				
		char *weekday_string_es[] = {"??", "Lunes", "Martes", "Miércoles", "Jueves", "Viernes", "Sábado", "Domingo"};
#else
		char *weekday_string_ru[] = {"??", "Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс"};
		char *weekday_string_en[] = {"??", "Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"};
		char *weekday_string_it[] = {"??", "Lu", "Ma", "Me", "Gi", "Ve", "Sa", "Do"};
		char *weekday_string_fr[] = {"??", "Lu", "Ma", "Me", "Je", "Ve", "Sa", "Di"};
		char *weekday_string_es[] = {"??", "Lu", "Ma", "Mi", "Ju", "Vi", "Sá", "Do"};
#endif
		char**	weekday_string;
		
		switch (get_selected_locale()){
			case locale_ru_RU:{
				weekday_string = weekday_string_ru;
				break;
			}
			case locale_it_IT:{
				weekday_string = weekday_string_it;
				break;
			}
			case locale_fr_FR:{
				weekday_string = weekday_string_fr;
				break;
			}
			case locale_es_ES:{
				weekday_string = weekday_string_es;
				break;
			}
			default:{
				weekday_string = weekday_string_en;
				break;
			}
		}
		_memclr(&text_buffer, 24);

		set_fg_color(COLOR_WHITE);
		show_big_digit(3, clock_time, 34, 58, 8); // печатаем результат(время) большими цифрами
		#if defined Day_Full_Left
			text_out_center(data,143,100);	// печатаем дату
			text_out(weekday_string[dt.weekday],3,100);	// печатаем день недели
		#elif defined Day_Full_Right
			text_out_center(data,36,100);	// печатаем дату
			text_out(weekday_string[dt.weekday],72,100);	// печатаем день недели
		#else
			#if defined Day_Short_No_Year_Left
				_sprintf(data, "%02s.%02d.%02d",weekday_string[dt.weekday], dt.day, dt.month);
				text_out_center(data,88,100);	// печатаем день недели
			#elif defined Day_Short_No_Year_Right
				_sprintf(data, "%02d.%02d.%02s", dt.day, dt.month,weekday_string[dt.weekday]);
				text_out_center(data,88,100);	// печатаем день недели
			#else
				text_out_center(data,88,100);	// печатаем дату
				#ifdef Day_Right
					text_out(weekday_string[dt.weekday],155,100);	// печатаем день недели
				#else
					text_out(weekday_string[dt.weekday],4,100);	// печатаем день недели
				#endif
			#endif
		#endif
		
		repaint_screen_lines(55, 120);
}

void screen_job(){
// при необходимости можно использовать данные экрана в этой функции
struct app_data_**  app_data_p = (struct app_data_ **)get_ptr_temp_buf_2();    //  указатель на указатель на данные экрана 
struct app_data_ *	app_data = *app_data_p;				//	указатель на данные экрана

// делаем периодическое действие: анимация, увеличение счетчика, обновление экрана,
// отрисовку интерфейса, обновление (перенос в видеопамять) экрана выполнять нужно

// отключаем экран приветствия
if (app_data->splash) {
	app_data->splash = 0;
	draw_screen();
	repaint_screen_lines(0, 176);
}

//	сделаем проверку на истечение таймаута выхода, если время истекло - выходим
if ( (get_tick_count() - app_data->last_tick) > ((app_data->state==STATE_PAUSED)?TIMEOUT_PAUSED:TIMEOUT_PLAYING) ){
	
	vibrate(3, 100, 100);
	
	// если сработал таймаут - выходим
	send_music_command(CMD_AMC_DISABLE);
	
	//	если запуск был из быстрого меню, выходим на циферблат
	if ( get_left_side_menu_active() ) 		
		app_data->proc->ret_f = show_watchface;
	
	// вызываем функцию возврата (обычно это меню запуска), в качестве параметра указываем адрес функции нашего приложения
	show_menu_animate(app_data->ret_f, (unsigned int)show_screen, ANIMATE_RIGHT);	
	set_update_period(0, 0);
	return;
	}


// если состояние BT изменилось, зафиксируем это и перересуем экран
#ifndef BipEmulator
if (app_data->last_bt_con != check_app_state(APP_STATE_BT_CON)){
	
	app_data->last_bt_con = check_app_state(APP_STATE_BT_CON);	
	draw_screen();
	repaint_screen_lines(0, 176);
	
	// отправим команду готовности управлять музыкой
	if (app_data->last_bt_con)
		send_music_command(CMD_AMC_ENABLE);
	
}
#endif
draw_time();
//vibrate(4, 100, 100);
set_update_period(1, 60000); // обновляем экран через минуту
//set_update_period(1, 350); // обновляем экран через время
}


int dispatch_screen (void *param){
struct app_data_**  app_data_p = (struct app_data_ **)get_ptr_temp_buf_2();    //  указатель на указатель на данные экрана
struct app_data_ *	app_data = *app_data_p;				//	указатель на данные экрана

// в случае отрисовки интерфейса, обновление (перенос в видеопамять) экрана выполнять нужно
app_data->last_tick = get_tick_count();	//	установим последнюю отметку времени активности на текущее время


#ifdef BipEmulator
	 struct gesture_ *gest = (gesture_*) param;
#else
	 struct gesture_ *gest = param;
#endif
int result = 0;

switch (gest->gesture){

	case GESTURE_CLICK: {	
	
		
		// при нажатии во время приветственного экрана закрываем его
		if (app_data->splash) {
			app_data->splash = 0;
			draw_screen();
			repaint_screen_lines(0, 176);
			return result;
		}
		#ifdef BipEmulator
		app_data->last_bt_con = 1;
		#else
		app_data->last_bt_con = check_app_state(APP_STATE_BT_CON);
		#endif
		if (app_data->last_bt_con){
	
			int btn = BTN_NONE;
			
			if ( ( gest->touch_pos_y > (88) ) ) {	//	нажатие на панель кнопок управления
								
				int btn_index = gest->touch_pos_x / (176/3);
				
				//определяем какая кнопка нажата
				if (btn_index == 0) {	//	левая кнопка
					btn = BTN_PREV;
				} else {
					if (btn_index == 1) {	//	средняя кнопка		
						btn = (app_data->state==STATE_PAUSED)?	BTN_PLAY	:	BTN_PAUSE;
					} else {					//	то что осталось - правая кнопка
						btn = BTN_NEXT;
					}
				}
			}  else 
			{			
				if (gest->touch_pos_x > 88){							
					btn = BTN_VOL_UP;
				} else {
					btn = BTN_VOL_DOWN;	
				}
			}
			
			if (btn != BTN_NONE)
				vibrate(1,70,0);
			
			switch (btn){
				case BTN_VOL_UP:		{	send_music_command(CMD_VOL_UP);		break;}
				case BTN_VOL_DOWN:		{	send_music_command(CMD_VOL_DOWN); 	break;}
				case BTN_PREV:			{	send_music_command(CMD_PREV);  app_data->state = STATE_PLAYING;		break;}
				case BTN_NEXT:			{	send_music_command(CMD_NEXT);  app_data->state = STATE_PLAYING;		break;}
				case BTN_PAUSE:			{	send_music_command(CMD_PAUSE); app_data->state = STATE_PAUSED;		break;}
				case BTN_PLAY: 			{	send_music_command(CMD_PLAY);  app_data->state = STATE_PLAYING;		break;}
				default: break;
			}
			
		}
		
		draw_screen();
		repaint_screen_lines(0, 176);
	
		break;
		};
		
		case GESTURE_SWIPE_RIGHT: 	//	свайп направо
		case GESTURE_SWIPE_LEFT: {	// справа налево
	
			if ( get_left_side_menu_active()){
					// в случае запуска через быстрое меню в proc->ret_f содержится dispatch_left_side_menu
					// и после отработки elf_finish (на который указывает app_data->ret_f, произойдет запуск dispatch_left_side_menu c
					// параметром структуры события тачскрина, содержащемся в app_data->proc->ret_param0
					
					// запускаем dispatch_left_side_menu с параметром param в результате произойдет запуск соответствующего бокового экрана
					// при этом произойдет выгрузка данных текущего приложения и его деактивация.
					void* show_f = get_ptr_show_menu_func();
						#ifdef BipEmulator
							dispatch_left_side_menu((gesture_*)param);
						#else
							dispatch_left_side_menu(param);
						#endif
										
					if ( get_ptr_show_menu_func() == show_f ){
						// если dispatch_left_side_menu отработал безуспешно (листать некуда) то в show_menu_func по прежнему будет 
						// содержаться наша функция show_screen, тогда просто игнорируем этот жест
					//	vibrate(1, 100, 100);
						return 0;
					}
					
					//	если dispatch_left_side_menu отработал, то завершаем наше приложение, т.к. данные экрана уже выгрузились
					// на этом этапе уже выполняется новый экран (тот куда свайпнули)
					Elf_proc_* proc = get_proc_by_addr(main);
					proc->ret_f = NULL;
					
					elf_finish(main);	//	выгрузить Elf из памяти
					return 0;
				} else { 	//	если запуск не из быстрого меню, обрабатываем свайпы по отдельности
					switch (gest->gesture){
						case GESTURE_SWIPE_RIGHT: {	//	свайп направо
							
							return show_menu_animate(app_data->ret_f, (unsigned int)main, ANIMATE_RIGHT);
							break;
						}
						case GESTURE_SWIPE_LEFT: {	// справа налево
							//	действие при запуске из меню и дальнейший свайп влево
							//vibrate(2,50,70);
							break;
						}
					} /// switch (gest->gesture)
				}

			break;
		};	//	case GESTURE_SWIPE_LEFT/RIGHT:
		
		case GESTURE_SWIPE_UP: {	// свайп вверх
		/*
			// действия при свайпе вверх
			if (app_data->theme < 1) 
				app_data->theme++;
			else 
				app_data->theme = 0;
			
			draw_screen();
			repaint_screen_lines(0, 176);
		*/
			break;
		};
		case GESTURE_SWIPE_DOWN: {	// свайп вниз
		/*
			// действия при свайпе вниз
			if (app_data->theme > 0) 
				app_data->theme--;
			else 
				app_data->theme = 1;
			
			draw_screen();
			repaint_screen_lines(0, 176);
		*/	
			break;
		};		
		default:{	// что-то пошло не так...
			
			break;
		};		
		
	}


	
// при необходимости ставим таймер вызова screen_job в мс
set_update_period(1, 500);

return result;
};

// пользовательская функция
void draw_screen(){
struct app_data_**  app_data_p = (struct app_data_ **)get_ptr_temp_buf_2();    //  указатель на указатель на данные экрана
struct app_data_ *	app_data = *app_data_p;				//	указатель на данные экрана
struct res_params_ res_params;							//	параметры графического реурса

set_bg_color(COLOR_BLACK);
fill_screen_bg();
set_graph_callback_to_ram_1();
// подгружаем шрифты
load_font();
set_fg_color(COLOR_WHITE);

// на стадии запуска приложения отрисовываем приветственный экран
if (app_data->splash){

	int result = get_res_params(app_data->proc->index_listed, 0, &res_params);
	if (result){
		text_out_center("Music control\nby Volkov Maxim", 88, 70);
		return;
	};

	show_elf_res_by_id(app_data->proc->index_listed , 0, (176-res_params.width)/2, (176-res_params.height)/2 );
	return;
}


switch (app_data->theme){

	case 0:{
		show_elf_res_by_id(app_data->proc->index_listed , RES_PLAYER_BG,  0,  2);			// 176x51
		show_elf_res_by_id(app_data->proc->index_listed , RES_PLAYER_BTN_BG,  0, 121);		// 176x51
		
		if (app_data->last_bt_con){
			show_elf_res_by_id(app_data->proc->index_listed , RES_PREV,  	  12, 133);		// 28x28
			show_elf_res_by_id(app_data->proc->index_listed , RES_NEXT, 	 135, 133);		// 28x28
			
			show_elf_res_by_id(app_data->proc->index_listed , RES_VOL_DOWN,  36, 23);		// 15x12
			show_elf_res_by_id(app_data->proc->index_listed , RES_VOL_UP,   125, 23);		// 15x12
			
			switch (app_data->state){
				case STATE_PAUSED:	{
					show_elf_res_by_id(app_data->proc->index_listed , RES_PLAY, 69, 128);	//38x38
					break;
				}
				case STATE_PLAYING:{
					show_elf_res_by_id(app_data->proc->index_listed , RES_PAUSE, 69, 128);		//38x38
					//show_elf_res_by_id(app_data->proc->index_listed , RES_PLAYER_EQ, 12, 73);	//38x38
					break;
				}
				default: break;
			}
		} else {
			show_elf_res_by_id(app_data->proc->index_listed , RES_VOL_DOWN_BG,  36, 23);		// 15x12
			show_elf_res_by_id(app_data->proc->index_listed , RES_VOL_UP_BG,   125, 23);		// 15x12
			
		}
		draw_time();
		break;
	
	}
	
	default: break;
/*	case 1:{
		show_res_by_id(RES_PLAYER_BG,  0, 176-51-51);		// 176x51
		show_res_by_id(RES_PLAYER_BTN_BG,  0, 176-51-25);		// 176x51
		show_res_by_id(RES_PLAYER_BG,  0, 176-51);		// 176x51
		
		show_res_by_id(RES_PREV,  		  12, 176-51-25+12);	// 28x28
		show_res_by_id(RES_NEXT, 		 135, 176-51-25+12);	// 28x28
		
		switch (app_data->state){
			case STATE_PAUSED:	{
				show_res_by_id(RES_PLAY, 69, 176-51-25+7);	//38x38
				break;
			}
			case STATE_PLAYING:{
				show_res_by_id(RES_PAUSE, 69, 176-51-25+7);		//38x38
				break;
			}
			default: break;
		}
		
		set_fg_color(COLOR_BLUE);
		draw_rect( 5, 176-51-51-40,  88, 176-51-51);
		draw_rect(88, 176-51-51-40, 171, 176-51-51);
		
		show_res_by_id(RES_VOL_DOWN, 	  30,  176-51-51-40+(40-12)/2);	// 15x12
		show_res_by_id(RES_VOL_UP, 176-30-15,  176-51-51-40+(40-12)/2);	// 15x12
		break;
	}
*/
}

};

int send_music_command(int cmd){
unsigned char d[4];
int size = 2;
d[0] = 0xFE;	//	команда управления музыкой
d[1] = cmd;

// если установлено BT соежинение, отправляем команду
#ifndef BipEmulator
if (check_app_state(APP_STATE_BT_CON)){
	send_host_app_data(0x42, 0x41, size, &d[0], 0);
}
#endif // !BipEmulator

return 0;
}