#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <stdarg.h>
#include <getopt.h>
#include <time.h>


#include "../rpi_ws281x/clk.h"
#include "../rpi_ws281x/gpio.h"
#include "../rpi_ws281x/dma.h"
#include "../rpi_ws281x/pwm.h"
#include "../rpi_ws281x/version.h"

#include "../rpi_ws281x/ws2811.h"

#include "font5x10.h"



#define ARRAY_SIZE(stuff)       (sizeof(stuff) / sizeof(stuff[0]))

// defaults for cmdline options
#define TARGET_FREQ             WS2811_TARGET_FREQ
#define GPIO_PIN                18
#define DMA                     10
#define STRIP_TYPE            WS2811_STRIP_GRB		// WS2812/SK6812RGB integrated chip+leds
#define LED_COUNT			7*256	
#define MatrixHeight		32
#define MatrixWidth			56

#define ChanceOfTwinkles	70
#define ChanceOfGlitter		5

int led_count = LED_COUNT;

int clear_on_exit = 1;

#include "leddefs.h"
ws2811_led_t *matrix;
#include "fastled.h"
#include "colors.h"

static uint8_t running = 1;
static uint8_t brightness = 30;

static void ctrl_c_handler(int signum)
{
	(void)(signum);
	running = 0;
}

static void setup_handlers(void)
{
    struct sigaction sa =
    {
        .sa_handler = ctrl_c_handler,
    };

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

/********************************************************************************
*
* There are literally hundreds of ways how a two dimensional array may be set up
* and you will have to provide your own XY() function
*
* This one is for my test panel (32x8x7)
*
********************************************************************************/
// XY mapping for sperpentine matrix layout
uint16_t XY(int x, int y) {
        uint16_t i;

        if( x & 0x01) { 
                // Odd rows run backwards
                uint8_t reverseY = (MatrixHeight - 1) - y;
                i = MatrixHeight*MatrixWidth - (x * MatrixHeight) + reverseY;
        } else {
                // Even rows run forwards
                i = MatrixHeight*MatrixWidth - (x * MatrixHeight) + y;
                }       
        return i;
}

void setxypixel(ws2811_led_t m[], int x, int y, struct CRGBW rgb) {
    m[XY(x,y)] = RGB2INT(rgb);
}

// drawchar is currently hardcoded aware of the Width of the font, This needs to be changed
// Currently no protection that we dont write out of bounds !!!!

void drawChar(char c, ws2811_led_t m[], int x, int y, struct CRGBW rgb) {
	uint8_t dot;

	for (int j = 0; j < FontHeight; j++) {
		for (int i = 0; i < FontWidth; i++) {
		
			dot = font[c*FontHeight + j] << i;	
			dot = dot & 0x80;
			if (( dot & 0x80) != 0) {
				setxypixel(m, x+i, y+j, rgb);
			}else{
				setxypixel(m, x+i, y+j, BLACK);
			}; 
		};
	}
	
}

void drawString(char *s, ws2811_led_t m[], int x, int y, struct CRGBW rgb) {
	int len = strlen(s);
	int xpos = x;

	for (int i = 0; i < len; i++) {
		xpos = x + i*FontWidth;
		drawChar(s[i], m, xpos, y, rgb);
	}
}

void readValues(char *temp, char *humidity) {
	FILE *fp;

	fp = fopen("/tmp/temperature","r");
	fscanf(fp,"%4s", temp);
	fclose(fp);
	fp = fopen("/tmp/humidity","r");
	fscanf(fp,"%4s", humidity);
	fclose(fp);
}

int readBrightness() {
	FILE * fp;
	int brightness;

	if ((fp = fopen("/var/www/html/clockBrightness","r")) != NULL) {
		fscanf(fp,"%d",&brightness);
		fclose(fp);
		return(brightness);
		}
	else
		return(-1);
}
					
int main(int argc, char *argv[])
{
	ws2811_return_t ret;
	uint8_t gHue = 0;
	struct CRGBW rgb;
	ws2811_led_t *leds;
	time_t t;
	struct tm *tim;
	char temperature[10];
	char humidity[10];

	sprintf(temperature,"UNKN");
	sprintf(humidity,"UNKN");

    matrix = malloc(sizeof(ws2811_led_t) * led_count);

    setup_handlers();

    if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS)
    {
        fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
        return ret;
    }

    setBrightness(&ledstring, brightness);

    while (running)
    {
		time(&t);				// get system time
		tim = localtime(&t);
		char tstring[20];
		char dstring[20];
		char ystring[20];
		char *mstring;
		int oldsec;


		sprintf(tstring, "%02d:%02d:%02d", tim->tm_hour, tim->tm_min, tim->tm_sec);
		sprintf(dstring, "%02d,",tim->tm_mday);
		sprintf(ystring, "%4d", tim->tm_year+1900);
		switch (tim->tm_mon) {
			case 0:
				mstring="Jan";
				break;
			case 1:
				mstring="Feb";
				break;
			case 2:
				mstring="Mar";
				break;
			case 3:
				mstring="Apr";
				break;
			case 4:
				mstring="May";
				break;
			case 5:
				mstring="Jun";
				break;
			case 6:
				mstring="Jul";
				break;
			case 7:
				mstring="Aug";
				break;
			case 8:
				mstring="Sep";
				break;
			case 9:
				mstring="Oct";
				break;
			case 10:
				mstring="Nov";
				break;
			case 11:
				mstring="Dec";
				break;
			default:
				mstring="UNK";
				break;
		}

		// Read the temperature and humidity values from the filesystem once every minute
		// These values should be written arounbd the second 0 so we read them at second 5

		if ((tim->tm_sec == 5) && (oldsec != tim->tm_sec)) {
			readValues(temperature, humidity);
		}
		if (((tim->tm_sec % 10) == 0) && (oldsec != tim->tm_sec)) {
			if (( brightness = readBrightness()) != -1) 
				setBrightness(&ledstring, brightness);
		}

		if (oldsec != tim->tm_sec)
			oldsec = tim->tm_sec;

		drawString(tstring, matrix, 9,1, K1900);
		drawString(mstring, matrix, 2, FontHeight+1, BLUE);
		drawString(dstring, matrix, FontWidth*4+1, FontHeight+1, BLUE);
		drawString(ystring, matrix, FontWidth*7+2, FontHeight+1, BLUE);
		drawString(temperature, matrix, 2, 2*FontHeight+1, RED);
		drawString("C", matrix, 4*FontWidth+2, 2*FontHeight+1, RED);
		drawString(humidity, matrix, FontWidth*6+2, 2*FontHeight+1, RED);
		drawString("%", matrix, 10*FontWidth+2, 2*FontHeight+1, RED);

	
		Show(matrix, &ledstring);


// 10 frames /sec
		usleep(1000000 / 10);
    }

	if (clear_on_exit) {
		Clear(matrix, 0, LED_COUNT);
		Show(matrix, &ledstring);
	}

    ws2811_fini(&ledstring);
    printf ("\n");
}
