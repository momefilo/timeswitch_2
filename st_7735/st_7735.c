// st_7735 Bibliothek
#include "st_7735.h"

uint16_t FgColor=0xFFE0, BgColor=0x0000, SeColor=0xf800;
uint8_t Width, Height;

static inline void cs_select() {
	asm volatile("nop \n nop \n nop");
	gpio_put(PIN_CS, 0);
	asm volatile("nop \n nop \n nop");
}
static inline void cs_deselect() {
	asm volatile("nop \n nop \n nop");
	gpio_put(PIN_CS, 1);
	asm volatile("nop \n nop \n nop");
}
void write_cmd(uint8_t *cmd, int len) {
	cs_select();
	spi_write_blocking(SPI_PORT, cmd, 1);
	if(len > 1){
		gpio_put(PIN_A0, 1);
		spi_write_blocking(SPI_PORT, cmd + (uint8_t)1, len - 1);
		gpio_put(PIN_A0, 0);
	}
	cs_deselect();
}
void set_col(int start, int end){
	uint8_t cmd[5];
	cmd[0] = 0x2A;
	cmd[1] = 0x00;
	cmd[2] = start;
	cmd[3] = 0x00;
	cmd[4] = end;
	write_cmd(cmd, 5);
}
void set_row(int start, int end){
	uint8_t cmd[5];
	cmd[0] = 0x2B;
	cmd[1] = 0x00;
	cmd[2] = start;
	cmd[3] = 0x00;
	cmd[4] = end;
	write_cmd(cmd, 5);
}

void write_font6x8(uint8_t *pos, uint8_t zeichen){
	uint8_t font_width = 6;
	uint8_t font_height = 8;
	uint8_t leer_col = 0;// Die linke Spalte ist leer da nur 7 bit
	//create buffer for font
	int len = font_width * font_height * 2 + 1;
	uint8_t buf[len];// buffer fuer das Zeichen
	buf[0] = 0x2C;// Startbyte
	int z = 1; // Schleifen-Zaehlvariable
	//write font in buffer
	for(int y=font_height; y>0; y--){  // 6 Byte umfasst ein 6x8Bit Zeichen
		for(int x=0; x<font_width; x++){ // 8 Bits pro Byte
			if(FONT6x8[zeichen][x] & (0x80 >> y)){ // ist das Bit gesetzt
				buf[z] = (FgColor & 0xFF00)>>8;
				buf[z+1] = (FgColor & 0x00FF);
			}
			else{ // Das Bit ist nicht gesetzt
				buf[z] = (BgColor & 0xFF00)>>8;
				buf[z+1] = (BgColor & 0x00FF);
			}
			z=z+2;
		}
	}
	//write buffer to display
	set_col(pos[0], pos[0] + font_width - 1);
	set_row(pos[1], pos[1] + font_height - 1);
	write_cmd(buf,len);
}
void writeText6x8(uint8_t *pos, char *text, bool sel, bool matrix){
	uint16_t tmp_color = FgColor;
	uint8_t len = strlen(text);
	if(sel) FgColor = SeColor;
	for(int i=0; i<len; i++){
		uint8_t mypos[2];
		if(matrix){
			mypos[0] = (pos[0]+i)*6;
			mypos[1] = pos[1]*8;
		}
		else{
			mypos[0] = pos[0];
			mypos[1] = pos[1];
			mypos[0] = mypos[0] + i*6;
		}
		write_font6x8(mypos, text[i]);
	}
	FgColor = tmp_color;
}
void write_font7x11(uint8_t *pos, uint8_t zeichen){
	uint8_t font_ofset = 0x20;// Zeichensatz-Anfang
	uint8_t font_width = 7;
	uint8_t font_height = 11;
	uint8_t leer_col = 1;// Die linke Spalte ist leer da nur 7 bit
	//create buffer for font
	int len = font_width * font_height * 2 + 1;
	uint8_t buf[len];// buffer fuer das Zeichen
	buf[0] = 0x2C;// Startbyte
	int z = 1; // Schleifen-Zaehlvariable
	//write font in buffer
	if(zeichen > 127 || zeichen < font_ofset) zeichen = font_ofset;
	for(int y=0; y<font_height; y=y+1){  // 11 Byte umfasst ein 7x11Bit Zeichen
		for(int x=leer_col; x<8; x=x+1){ // 7 Bits pro Byte
			if(FONT7x11[zeichen-font_ofset][y] & (0x01 << (x - 1))){ // ist das Bit gesetzt
				buf[z] = (FgColor & 0xFF00)>>8;
				buf[z+1] = (FgColor & 0x00FF);
			}
			else{ // Das Bit ist nicht gesetzt
				buf[z] = (BgColor & 0xFF00)>>8;
				buf[z+1] = (BgColor & 0x00FF);
			}
			z=z+2;
		}
	}
	//write buffer to display
	set_col(pos[0], pos[0] + font_width - 1);
	set_row(pos[1], pos[1] + font_height - 1);
	write_cmd(buf,len);
}
void writeText7x11(uint8_t *pos, char *text, bool sel, bool matrix){
	uint16_t tmp_color = FgColor;
	if(sel) FgColor = SeColor;
	uint8_t len = strlen(text);
	for(int i=0; i<len; i++){
		uint8_t mypos[2];
		if(matrix){
			mypos[0] = (pos[0]+i)*7;
			mypos[1] = pos[1]*11;
		}
		else{
			mypos[0] = pos[0];
			mypos[1] = pos[1];
			mypos[0] = mypos[0] + i*7;
		}
		write_font7x11(mypos, text[i]);
	}
	FgColor = tmp_color;
}
void write_font10x16(uint8_t *pos, uint8_t zeichen){
	uint8_t font_ofset = 0x20;
	uint8_t font_width = 10;
	uint8_t font_height = 16;
	uint8_t leer_col = 6;
	//create buffer for font
	int len = font_width * font_height * 2 + 1;
	uint8_t buf[len];
	buf[0] = 0x2C;// Startbyte
	int z = 1;// Schleifen-Zaehlvariable
	//write font in buffer
	for(int i=0; i<32; i=i+1){ // 32 Byte umfasst ein 10x16Bit Zeichen
		int anfang = 0;
		if(i % 2 == 0) anfang = 6;
		for(int k=anfang; k<8; k=k+1){ // zwei oder acht Bits pro Byte
//			if(zeichen < 0x21) zeichen = 0x21; // THIS IS A BUG!
			if(FONT10x16[zeichen-font_ofset][i] & (0x80 >> k)){ // ist das Bit gesetzt
				buf[z] = (FgColor & 0xFF00)>>8;
				buf[z+1] = (FgColor & 0x00FF);
			}
			else{ // Das Bit ist nicht gesetzt
				buf[z] = (BgColor & 0xFF00)>>8;
				buf[z+1] = (BgColor & 0x00FF);
			}
			z=z+2;
		}
	}
	//write buffer to display
	set_col(pos[0],pos[0] + font_width - 1);
	set_row(pos[1],pos[1] + font_height - 1);
	write_cmd(buf,len);
}
void writeText10x16(uint8_t *pos, char *text, bool sel, bool matrix){
	uint16_t tmp_color = FgColor;
	uint8_t len = strlen(text);
	if(sel) FgColor = SeColor;
	for(int i=0; i<len; i++){
		uint8_t mypos[2];
		if(matrix){
			mypos[0] = (pos[0]+i)*10;
			mypos[1] = pos[1]*16;
		}
		else{
			mypos[0] = pos[0];
			mypos[1] = pos[1];
			if(i>0) mypos[0] = mypos[0] + i*10;
		}
		write_font10x16(mypos, text[i]);
	}
	FgColor = tmp_color;
}
void write_font12x12(uint8_t *pos, uint8_t zeichen){
	//create buffer for font
	int len = 12*12*2+1;
	uint8_t buf[len];
	buf[0] = 0x2C;
	int z = 1; // Schleifen-Zaehlvariable
	//write font in buffer
	for(int i=0; i<18; i=i+1){ // 18 Byte umfasst ein 16x16Bit Zeichen
		for(int k=0; k<8; k=k+1){ // Acht Bits pro Byte
			if(FONT12x12[zeichen*18+i] & (0x80 >> k)){ // ist das Bit gesetzt
				buf[z] = (FgColor & 0xFF00)>>8;
				buf[z+1] = (FgColor & 0x00FF);
			}
			else{ // Das Bit ist nicht gesetzt
				buf[z] = (BgColor & 0xFF00)>>8;
				buf[z+1] = (BgColor & 0x00FF);
			}
			z=z+2;
		}
	}
	//write buffer to display
	set_col(pos[0],pos[0]+11);
	set_row(pos[1],pos[1]+11);
	write_cmd(buf,len);
}
void writeText12x12(uint8_t *pos, char *text, int len, bool sel, bool matrix){
	uint16_t tmp_color = FgColor;
	if(sel) FgColor = SeColor;
	for(int i=0; i<len; i++){
		uint8_t mypos[2];
		if(matrix){
			mypos[0] = (pos[0]+i)*12;
			mypos[1] = pos[1]*12;
		}
		else{
			mypos[0] = pos[0];
			mypos[1] = pos[1];
			if(i>0) mypos[0] = mypos[0] + i*12;
		}
		write_font12x12(mypos, text[i]);
	}
	FgColor = tmp_color;
}
void write_font12x16(uint8_t *pos, uint8_t zeichen){
	uint8_t font_width = 12;
	uint8_t font_height = 16;
	uint8_t leer_col = 4;
	//create buffer for font
	int len = font_width * font_height * 2 + 1;
	uint8_t buf[len];
	buf[0] = 0x2C;// Startbyte
	int z = 1;// Schleifen-Zaehlvariable
	//write font in buffer
	for(int i=0; i<32; i=i+1){ // 32 Byte umfasst ein 16x16Bit Zeichen
		int ofset = i + 1;
		int ende = 8;
		if(ofset % 2 == 0) ende = 4;
		for(int k=0; k<ende; k=k+1){ // acht oder vier Bits pro Byte
			if(FONT12x16[zeichen*32+i] & (0x80 >> k)){ // ist das Bit gesetzt
				buf[z] = (FgColor & 0xFF00)>>8;
				buf[z+1] = (FgColor & 0x00FF);
			}
			else{ // Das Bit ist nicht gesetzt
				buf[z] = (BgColor & 0xFF00)>>8;
				buf[z+1] = (BgColor & 0x00FF);
			}
			z=z+2;
		}
	}
	//write buffer to display
	set_col(pos[0],pos[0] + font_width - 1);
	set_row(pos[1],pos[1] + font_height - 1);
	write_cmd(buf,len);
}
void writeText12x16(uint8_t *pos, char *text, bool sel, bool matrix){
	uint16_t tmp_color = FgColor;
	uint8_t laenge = strlen(text);
	if(sel) FgColor = SeColor;
	for(int i=0; i<laenge; i++){
		uint8_t mypos[2];
		if(matrix){
			mypos[0] = (pos[0]+i)*12;
			mypos[1] = pos[1]*16;
		}
		else{
			mypos[0] = pos[0];
			mypos[1] = pos[1];
			if(i>0) mypos[0] = mypos[0] + i*12;
		}
		write_font12x16(mypos, text[i]);
	}
	FgColor = tmp_color;
}
void write_font14x20(uint8_t *pos, uint8_t zeichen){
	uint8_t font_width = 14;
	uint8_t font_height = 20;
	uint8_t leer_col = 2;
	//create buffer for font
	int len = font_width * font_height * 2 + 1;
	uint8_t buf[len];
	buf[0] = 0x2C;// Startbyte
	int z = 1;// Schleifen-Zaehlvariable
	//write font in buffer
	for(int i=0; i<40; i=i+1){ // 40 Byte umfasst ein 14x20Bit Zeichen
		int anfang = 0;
		if(i % 2 == 0) anfang = leer_col;
		for(int k=anfang; k<8; k=k+1){ // zwei oder acht Bits pro Byte
			if(zeichen <0x20) zeichen = 0x20;
			if(FONT14x20[zeichen-0x20][i] & (0x80 >> k)){ // ist das Bit gesetzt
				buf[z] = (FgColor & 0xFF00)>>8;
				buf[z+1] = (FgColor & 0x00FF);
			}
			else{ // Das Bit ist nicht gesetzt
				buf[z] = (BgColor & 0xFF00)>>8;
				buf[z+1] = (BgColor & 0x00FF);
			}
			z=z+2;
		}
	}
	//write buffer to display
	set_col(pos[0],pos[0] + font_width - 1);
	set_row(pos[1],pos[1] + font_height - 1);
	write_cmd(buf,len);
}
void writeText14x20(uint8_t *pos, char *text, bool sel, bool matrix){
	uint16_t tmp_color = FgColor;
	uint8_t len = strlen(text);
	if(sel) FgColor = SeColor;
	for(int i=0; i<len; i++){
		uint8_t mypos[2];
		if(matrix){
			mypos[0] = (pos[0]+i)*14;
			mypos[1] = pos[1]*20;
		}
		else{
			mypos[0] = pos[0];
			mypos[1] = pos[1];
			if(i>0) mypos[0] = mypos[0] + i*14;
		}
		write_font14x20(mypos, text[i]);
	}
	FgColor = tmp_color;
}

void paintRect(uint8_t *area, uint16_t color){
	int len = (area[2] - area[0] + 1)*(area[3] - area[1] + 1)*2+1;
	uint8_t pixarea[len];
	pixarea[0] = 0x2C;
	for(int i=1; i<len-2; i=i+2){
		pixarea[i] = (color & 0xFF00)>>8;
		pixarea[i+1] = (color & 0x00FF);
	 }
	set_col(area[0], area[2]);
	set_row(area[1], area[3]);
	write_cmd(pixarea, (len));
}
void drawRect(uint8_t *area, uint8_t *data){
	int len = (area[2] - area[0] + 1)*(area[3] - area[1] + 1)*2+1;
	set_col(area[0], area[2]);
	set_row(area[1], area[3]);
	write_cmd(data, (len));
}

void clearScreen(){
	int len = Height * Width * 2 +1;
	uint8_t area[len];
	for(int i=1; i<len-1; i=i+2){
		area[i] = (BgColor & 0xFF00)>>8;
		area[i+1] = (BgColor & 0x00FF);
	}
	area[0] = 0x2C;
	set_col(0, Width -1);
	set_row(0, Height -1);
	write_cmd(area, len);
}
void st7735_init(){
	if(true){//spi setup
		spi_init(SPI_PORT, 100* 1000 * 1000);
		gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
		gpio_set_function(PIN_SDA, GPIO_FUNC_SPI);
		gpio_init(PIN_CS);
		gpio_set_dir(PIN_CS, GPIO_OUT);
		gpio_put(PIN_CS, 1);
		gpio_init(PIN_A0);
		gpio_set_dir(PIN_A0, GPIO_OUT);
		gpio_put(PIN_A0, 0);
		gpio_init(PIN_RST);
		gpio_set_dir(PIN_RST, GPIO_OUT);
		gpio_put(PIN_RST, 1);
	}
	sleep_ms(200);
	if(true){//display init
		uint8_t cmd[2];
		cmd[1] = 0;
		//sleep out
		cmd[0] = 0x11;
		write_cmd(cmd,1);
		sleep_ms(60);
		//display on
		cmd[0]=0x29;
		write_cmd(cmd,1);
		//16-bit colormode
		cmd[0] = 0x3A;
		cmd[1] = 0x05;
		write_cmd(cmd,2);
		sleep_ms(100);
		//flip vertikal&hoizontal
		cmd[0] = 0x36;
		cmd[1] = 0x60;
		write_cmd(cmd,2);
		Width = 160;
		Height = 128;
	}
	clearScreen();
}
void setOrientation(uint8_t ori){
	uint8_t cmd[2];
	cmd[0] = 0x36;
	cmd[1] = 0x60;
	Width = 160;
	Height = 128;
	if(ori > 0){
		cmd[1] = 0;
		Width = 128;
		Height = 160;
	}
	write_cmd(cmd,2);
}
void setSeColor(uint16_t color){
	SeColor = color;
}
void setFgColor(uint16_t color){
	FgColor = color;
}
void setBgColor(uint16_t color){
	BgColor = color;
}

void paintPixel(uint8_t x, uint8_t y, uint16_t color){
	uint8_t area[] = { 0x2C, (color & 0xFF00) >> 8, color & 0x00FF};
	set_col(x, x);
	set_row(y, y);
	write_cmd(area, 3);
}
static int16_t iliVektor[320][2];
uint8_t getVektorWinkel(uint16_t grad, uint8_t begin, uint8_t end){
	int8_t fak = 1; if(grad > 179){ grad -= 180; fak = -1;}
	double_t sinus = sin(grad * (2 * 3.14) / 360);
	double_t cosin = cos(grad * (2 * 3.14) / 360);
	int cnt = 0;
	for(int i=begin; i<end; i++){
		if(grad == 90){
			iliVektor[cnt][0] = fak * i;
			iliVektor[cnt][1] = 0;
			cnt++;
		}
		else if(grad == 0){
			iliVektor[cnt][0] = 0;
			iliVektor[cnt][1] = fak * -i;
			cnt++;
		}
		else{
			double_t cos_ = -i * cosin;
			double_t sin_ = i * sinus;
			int16_t y = cos_*fak, x = sin_*fak;
			if(i < begin + 1){
				iliVektor[cnt][0] = x;
				iliVektor[cnt][1] = y;
				cnt++;
			}
			else if(i > begin && ((iliVektor[cnt-1][0] != x) || (iliVektor[cnt-1][1] != y))){
				iliVektor[cnt][0] = x;
				iliVektor[cnt][1] = y;
				cnt++;
			}
		}
	}
	return cnt;
}
// Zeichnet vom Pumkt (x,y) im eine Linie im Winkel a im Abschnitt von begin bis end in der Dicke width und Farbe Color
void paintLine(uint8_t x, uint8_t y, uint16_t a, uint8_t begin, uint8_t end, uint8_t width, uint16_t color){
	uint8_t vlen = getVektorWinkel(a, begin, end);
	uint8_t mid  = (width - 1) / 2;
	uint8_t cnt = 0;
	for(int i=0; i<vlen; i++){
		uint16_t startx = iliVektor[i][0] + x;
		uint16_t endx = startx;
		uint16_t starty = iliVektor[i][1] + y;
		uint16_t endy = starty;
		if((a > (7*6) && a < (23*6)) || (a > (37*6) && a < (53*6))){
			starty = iliVektor[i][1] + y - width + mid;
			endy = iliVektor[i][1] + y + width - mid;
		}
		else{
			startx = iliVektor[i][0] + x - width + mid;
			endx = iliVektor[i][0] + x + width - mid;
		}
		uint8_t area[] = { startx, starty, endx, endy};
		paintRect(area, color);
	}
}
