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


#define ARRAY_SIZE(stuff)       (sizeof(stuff) / sizeof(stuff[0]))

// defaults for cmdline options
#define TARGET_FREQ             WS2811_TARGET_FREQ
#define GPIO_PIN                18
#define DMA                     10
#define STRIP_TYPE            WS2811_STRIP_GRB		// WS2812/SK6812RGB integrated chip+leds
#define LED_COUNT		64*5

#define ChanceOfTwinkles	70
#define ChanceOfGlitter		5

int led_count = LED_COUNT;

int clear_on_exit = 1;

#include "leddefs.h"
ws2811_led_t *matrix;
#include "fastled.h"

static uint8_t running = 1;
static uint8_t brightness = 50;

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

void addGlitter(ws2811_led_t m[]) {
	if (crandom(100) > (100-ChanceOfGlitter)) { 			// give it a 5% chance
		m[crandom(LED_COUNT)] = 0xFFFFFF; 	// random pixel bright white
	}
}

// turn on random led, fade the rest slowly to black
void twinkles(ws2811_led_t m[]) {
	// first fade all to black
	for ( int i=0; i< LED_COUNT; i++ ) {
		fadeToBlackBy(&m[i], 30);
	}
	if (crandom(100) > (100-ChanceOfTwinkles)) {		// at a frame rate of 50frames/sec approx 5 every second (10% chance every time)
		struct CHSV hsv;
		struct CRGBW rgb;

		int index = crandom(LED_COUNT);		// which led?
		hsv.v = 255;
        	hsv.s = 255;
        	hsv.h = crandom(255);
        	hsv2rgb(&hsv, &rgb);
        	m[index] = RGB2INT(rgb);		
	}

}

// XY mapping for sperpentine matrix layout
/*
uint16_t XY(int x, int y) {
        uint16_t i;

        if( y & 0x01) { 
                // Odd rows run backwards
                uint8_t reverseX = (MatrixWidth - 1) - x;
                i = (y * MatrixWidth) + reverseX;
        } else {
                // Even rows run forwards
                i = (y * MatrixWidth) + x;
                }       
        }       
        return i;
}
*/

					
int main(int argc, char *argv[])
{
    ws2811_return_t ret;
    uint8_t gHue = 0;

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

	// Until we find a better solution, this block of code emulates EVERY_N_MILLISECONDS
	{
	struct timespec tp;
	static int 	oldms = 0;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
	int ms=tp.tv_sec*1000 + tp.tv_nsec/1000000;			// get milliseconds
	if (ms < oldms) oldms=oldms-1000;				
	if ((ms-oldms) >= 50 || (oldms== 0)) {
		oldms=ms;
		// your code goes here
		gHue++;
		}
	}

	twinkles(matrix);
	addGlitter(matrix);

//	fill_rainbow(matrix, LED_COUNT, gHue, 10);
	
        Show(matrix, &ledstring);


        // 50 frames /sec
  //      usleep(1000000 / 50);
    }

    if (clear_on_exit) {
	Clear(matrix, 0, LED_COUNT);
	Show(matrix, &ledstring);
    }

    ws2811_fini(&ledstring);
    printf ("\n");
}
