**LCD st_7735**\
Diese Bibliothek bietet je eine Textfunktion mit 12x12 und einr mit 16x16
Pixel Zeichengroesse. Die Text- und Hintergrundfarbe sind änderbar.
Und ein Funktion um beliebige Pixelinformation auf das display zu schreiben.\
Die GPIO-Zuordnng ist zu Beginn in der "ili_9341.c"-Datei anzupassen.\
Die Funktionen der Bibliothek sind folgend erlaeutert

**void Display_init()**\
Ist einmalig vor Benutzung der Funktionen aufzurufen

**void setOrientation(uint8_t ori)**\
Setzt die Orientierung des Displays, Parameter ori sind HORIZONTAL und VERTICAL

**void setFgColor(uint8_t *color)**\
Textfarbe 12-Bit Farbwert color={r,g,b}
color ist eine Array aus drei uint8-Werten, wobei nur die ersten vier Bits
der Werte die Farbinformation tragen

**void setSeColor(uint8_t *color)**\
Alternativ-Textfarbe 12-Bit Farbwert color={r,g,b}
color ist eine Array aus drei uint8-Werten, wobei nur die ersten vier Bits
der Werte die Farbinformation tragen

**void setBgColor(uint8_t *color)**\
Hintergrundfarbe 12-Bit Farbwert color={r,g,b}
color ist eine Array aus drei uint8-Werten, wobei nur die ersten vier Bits
der Werte die Farbinformation tragen

**void clearScreen()**\
Fuellt das Display mit der Hintergrundfarbe

**void writeText16x16(uint8_t *pos, char *text, int len, bool sel, bool matrix)**\
Schreibt char \*text mit 16x16 Pixeln\
pos[0]: ist die Spalte, pos[1] die Zeile, in Pixel wenn matrix = false.\
text: der Text der angezeigt wird\
len: ist die anzahl der Zeichen des Textes\
sel: wenn true wird der Text statt in FGground in Alternativfarbe angezeigt\
matrix: wenn true dann beziehen sich die Koordinatn in pos auf eine Matrix\
deren Felder Quadrate mit der Kantelänge von 16 Pixeln sind.

**void writeText12x12(uint8_t *pos, char *text, int len, bool sel, bool matrix)**\
Schreibt char \*text mit 12x12 Pixel\
pos[0]: ist die Spalte, pos[1] die Zeile, in Pixel wenn matrix = false.\
text: der der Text der angezeigt wird\
len: ist die anzahl der Zeichen des Textes\
sel: wenn true wird der Text statt in FGground in SelColor angezeigt.\
matrix: wenn true dann beziehen sich die Koordinatn in pos auf eine Matrix
deren Felder Quadrate mit der Kantelänge von 12 Pixeln sind.

**void drawRect(uint8_t *area, uint8_t *data)**\
zeichnet die in \*data uebergebenen Daten in den
durch area bezeichneten Bereich\
area[0]: position x.start, area[1] position y.start\
area[2] position x.end, area[3] position y.end\
Data ist die Anfangsadresse das 12 Farbbits pro Pixel umfassenden
Speicherbereichs der auf das Display geschrieben wird\
Eine genaue Beschreibung findet sich in den Komentaren in st_7735.h

