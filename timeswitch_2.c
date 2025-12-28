// timeswitch_2
#include "onewire_library/onewire_library.h"
#include "momefilo_flash/momefilo_flash.h"
#include "buttons/buttons.h"
#include "st_7735/st_7735.h"
#include <stdio.h>
#include "pico/stdlib.h"

#define OUT_0 26//18
#define OUT_1 27//19
#define OUT_2 28//20
#define OUT_3 29//21
#define T_PIN 8//28

uint8_t Pins[4] = {OUT_0, OUT_1, OUT_2, OUT_3};
int16_t Intervalls[5][3] = { {5, 20, 0},{5, 20, 0},{5, 20, 0},{5, 20, 0},{5, 20, 0} }; //{go, pause, status}
uint32_t Flashdata[15];
uint8_t Count = 0;// Aktueller Kanal
static alarm_id_t Alarm_id;
void initFlash(){
	flash_init(0);
	uint32_t *Flashdata = flash_getData();
	if(Flashdata[0] != 0x0800){
		Flashdata[0] = 0x0800;
		int cnt = 1;
		for(int i=0; i<5; i++){
			for(int k=0; k<3; k++){
				Flashdata[cnt] = Intervalls[i][k];
				cnt++;
			}
		}
		flash_setDataRow(0, cnt, Flashdata);
	}else{
		int cnt = 1;
		for(int i=0; i<5; i++){
			for(int k=0; k<3; k++){
				Intervalls[i][k] = Flashdata[cnt];
				cnt++;
			}
		}
	}
}
void saveIntervalls(){

	int cnt = 0;
	for(int i=0; i<5; i++){
		for(int k=0; k<3; k++){
			Flashdata[cnt] = Intervalls[i][k];
			cnt++;
		}
	}
	flash_setDataRow(1, cnt+1, Flashdata);
}
int64_t callback_pause(alarm_id_t id, __unused void *user_data);
int64_t callback_go(alarm_id_t id, __unused void *user_data) {
	if(Intervalls[Count][2] == 0){
		gpio_put(Pins[Count], true);
		Alarm_id = add_alarm_in_ms(Intervalls[Count][0]*1000, callback_pause, NULL, true);
	}
	else
		Alarm_id = add_alarm_in_ms(100, callback_pause, NULL, true);
    return 0;
}
int64_t callback_pause(alarm_id_t id, __unused void *user_data) {
	if(Intervalls[Count][2] == 0){
		gpio_put(Pins[Count], false);
		Alarm_id = add_alarm_in_ms(1+Intervalls[Count][1]*1000, callback_go, NULL, true);
	}
	else
		Alarm_id = add_alarm_in_ms(100, callback_go, NULL, true);
	Count++;
	if(Count > 3) Count = 0;
    return 0;
}

/* Temperatur */
int16_t Sensoranzahl = 0;
uint64_t SensorId;
uint8_t Tsum = 0, TKurvCnt = 0;
double Temps[160], Temp = 0.0, TempSum = 0.0, TempMid = 0, Tspan = 5.0f, Theight = 38;
struct repeating_timer Timer;
void paintMenu(); //Vorwaertsdeklaration
void updateSpan(double temp){
	for(int i=158; i>=0; i--) Temps[i + 1] = Temps[i];
	Temps[0] = temp;
	double ofset = 0.0f;
	if(TempMid - Temps[0] > Tspan/2){
		ofset = (TempMid - Temps[0]) - Tspan/2;
		TempMid = Temps[0] + Tspan/2;
	}
	else if(TempMid - Temps[0] < (-1*Tspan/2)){
		ofset = Tspan/2 + (TempMid - Temps[0]);
		TempMid = Temps[0] - Tspan/2;
	}
	for(int i=1; i<160; i++) Temps[i] += ofset;
	if(TKurvCnt < 162) TKurvCnt++;
	paintMenu();
}
bool updateTemp(__unused struct repeating_timer *t){
	Temp = ds18b20_getTemps()[0];
	uint8_t pos[] = {100, 72};
	setFgColor(0xFFFF);
	char text[9];
	sprintf(text, "        ");
	writeText7x11(pos, text, false, false);
	sprintf(text, "%.2f'C", Temp);
	writeText7x11(pos, text, false, false);
	TempSum += Temp;
	Tsum++;
	if(Tsum > 28){
		TempSum = TempSum / Tsum;
		updateSpan(TempSum);
		TempSum = 0;
		Tsum = 0;
	}
	return true;
}

/* Menu */
uint16_t gelb = 0xFFE0, green = 0x07E0, blue = 0x001F, red = 0xF800, black = 0x0000;
int8_t Selection = 9;
void paintMenu(){
	char leer[] = "                ";
	int cnt = 0;
	for(int i=0; i<5; i++){// Kanal A bis D und Gesamt
		for(int k=0; k<2; k++){
			if(k == 0){// Run
				uint8_t pos[] = {0, i};
				if(i > 3) pos[1] = 5;
				setFgColor(black);
				writeText7x11(pos, leer, false, true);
				char tab[2];
				if(i < 4){
					sprintf(tab, "%d", Pins[i]);
					if(Pins[i] < 10) sprintf(tab, " %d", Pins[i]);
				}
				else sprintf(tab, "%s", "Gs");
				setFgColor(gelb);
				writeText7x11(pos, tab, false, true);

				pos[0] += 3;
				if(Intervalls[i][k] < 10) pos[0] += 2;
				else if(Intervalls[i][k] < 100) pos[0] += 1;
				char text[3]; sprintf(text, "%d", Intervalls[i][k]);
				setFgColor(green);
				if(Selection == cnt) setFgColor(blue);
				writeText7x11(pos, text, false, true);
			}else if(k == 1){// Pause
				uint8_t pos[] = {7, i};
				if(i > 3) pos[1] = 5;
				if(Intervalls[i][k] < 10) pos[0] += 2;
				else if(Intervalls[i][k] < 100) pos[0] += 1;
				char text[3]; sprintf(text, "%d", Intervalls[i][k]);
				setFgColor(red);
				if(Selection == cnt) setFgColor(blue);
				writeText7x11(pos, text, false, true);
			}
			cnt++;
		}
	}
	for(int i=0; i<5; i++){
		char text[5];
		if(Intervalls[i][2] < 1){
			setFgColor(gelb);
			sprintf(text, "line");
		}else if(Intervalls[i][2] < 2){
			setFgColor(green);
			sprintf(text, "On");
		}else{
			setFgColor(red);
			sprintf(text, "off");
		}
		uint8_t pos[] = {11, i};
		if(i > 3)pos[1] = 5;
		if(Selection == 14 - i) setFgColor(blue);
		writeText7x11(pos, text, false, true);
	}
	// Temperatur
	double ymin = TempMid - Tspan/2;
	uint16_t color = 0x0021;
	uint8_t area[] = {0, 127-Theight, 159, 127};
	paintRect(area, 0);// clear area
	for(int i=0; i<160; i++){
		//paintPixel(i, 127-Theight, color);
		for(int k=0; k<=Tspan; k++){
			double tmp = k * (Theight / Tspan);
			uint8_t y = 127 - tmp;
			paintPixel(i, y, color);
		}
		if(i > 0 && i % 20 == 0) for(int k=127; k>127-Theight; k--)paintPixel(i, k, color);
		double tmp = (Temps[i] - ymin) * (Theight/Tspan);
		uint8_t y = 127 - tmp;
		if(y < 128 && y > 127-Theight && TKurvCnt >= i) paintPixel(159 - i, y, 0xFFFF);
	}
}
void changeMenu(bool up){
	int sel = 0;
	for(int i=0; i<5; i++){
		for(int k=0; k<2; k++){
			if(Selection == sel){
				if(up) Intervalls[i][k] += 1;
				else  Intervalls[i][k] -= 1;
				if(Intervalls[i][k] > 999) Intervalls[i][k] = 0;
				else if(Intervalls[i][k] < 0){
					if(k <1) Intervalls[i][k] = 999;
					else Intervalls[i][k] = 0;
				}
				if(k < 1 & Intervalls[i][k] == 0)Intervalls[i][k] = 1;
			}
			sel++;
		}
	}
	for(int i=0; i<5; i++){
		if(Selection == 14 - i){
			if(up) Intervalls[i][2] += 1;
			else  Intervalls[i][2] -= 1;
			if(Intervalls[i][2] > 2) Intervalls[i][2] = 0;
			else if(Intervalls[i][2] < 0) Intervalls[i][2] = 2;
			if(i < 4){// Kanalstatus
				if(Intervalls[i][2] == 0 | Intervalls[i][2] == 2) gpio_put(Pins[i], false);
				else  gpio_put(Pins[i], true);
			}
		}
	}
	if(Selection == 8) for(int i=0; i<4; i++) Intervalls[i][0] = Intervalls[4][0];
	if(Selection == 9) for(int i=0; i<4; i++) Intervalls[i][1] = Intervalls[4][1];
	if(Selection == 10){
		for(int i=0; i<4; i++){
			Intervalls[i][2] = Intervalls[4][2];
			if(Intervalls[i][2] == 0 | Intervalls[i][2] == 2)gpio_put(Pins[i], false);
			else if(Intervalls[i][2] == 1)gpio_put(Pins[i], true);
		}
	}
	saveIntervalls();
}

int main() {
	stdio_init_all();
	busy_wait_ms(100);
	initFlash();
	buttons_init();
	st7735_init();
	for(int i=0; i<4; i++){ gpio_init(Pins[i]); gpio_set_dir(Pins[i], true); gpio_pull_down(Pins[i]);}
	// Init Temperatur
	Sensoranzahl = ds18b20_init(pio0, T_PIN);
	if(Sensoranzahl > 0){;
		Temp = ds18b20_getTemps()[0];//updateTemp();
		TempMid = Temp;
		for(int i=0; i<160; i++) Temps[i] = Temp;
		add_repeating_timer_ms(-2000, updateTemp, NULL, &Timer);
	}
	paintMenu();
	add_alarm_in_ms(1, callback_go, NULL, true);
	while (true) {
		uint8_t button = get_Button();
		if(button == BUTTON_U){ changeMenu(true); paintMenu();}
		else if(button == BUTTON_D){ changeMenu(false); paintMenu();}
		else if(button == BUTTON_L){
			Selection--;
			if(Selection < 0) Selection = 14;
			paintMenu();
		}
		else if(button == BUTTON_R){
			Selection++;
			if(Selection > 14) Selection = 0;
			paintMenu();
		}

	}
}

