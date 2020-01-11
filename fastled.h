#define scale8(i, s) ((i*s) >> 8)
#define scale8_video(i, scale) (((int)i * (int)scale) >> 8) + ((i&&scale)?1:0)
#define crandom(x) (random() % x)

typedef uint8_t   fract8;   ///< ANSI: unsigned short _Fract
typedef uint16_t   fract16;

/// Pre-defined hue values for HSV objects
typedef enum {
    HUE_RED = 0,
    HUE_ORANGE = 32,
    HUE_YELLOW = 64,
    HUE_GREEN = 96,
    HUE_AQUA = 128,
    HUE_BLUE = 160,
    HUE_PURPLE = 192,
    HUE_PINK = 224
} HSVHue;

void setBrightness( struct ws2811_t *l, uint8_t b) {
	l[0].channel[0].brightness = b;
}

uint8_t qadd8( uint8_t i, uint8_t j)
{
    unsigned int t = i + j;
    if( t > 255) t = 255;
    return t;
}

 int8_t qadd7( int8_t i, int8_t j)
{
    int16_t t = i + j;
    if( t > 127) t = 127;
    return t;
}

 uint8_t qsub8( uint8_t i, uint8_t j)
{
    int t = i - j;
    if( t < 0) t = 0;
    return t;
}

 uint8_t add8( uint8_t i, uint8_t j)
{
    int t = i + j;
    return t;
}

 uint16_t add8to16( uint8_t i, uint16_t j)
{
    uint16_t t = i + j;
    return t;
}

 uint8_t sub8( uint8_t i, uint8_t j)
{
    int t = i - j;
    return t;
}

 uint8_t avg8( uint8_t i, uint8_t j)
{
    return (i + j) >> 1;
}

 uint16_t avg16( uint16_t i, uint16_t j)
{
    return (uint32_t)((uint32_t)(i) + (uint32_t)(j)) >> 1;
}

 int8_t avg7( int8_t i, int8_t j)
{
    return ((i + j) >> 1) + (i & 0x1);
}

 int16_t avg15( int16_t i, int16_t j)
{
    return ((int32_t)((int32_t)(i) + (int32_t)(j)) >> 1) + (i & 0x1);
}

 uint8_t mod8( uint8_t a, uint8_t m)
{
    while( a >= m) a -= m;
    return a;
}

 uint8_t addmod8( uint8_t a, uint8_t b, uint8_t m)
{
    a += b;
    while( a >= m) a -= m;
    return a;
}

 uint8_t submod8( uint8_t a, uint8_t b, uint8_t m)
{
    a -= b;
    while( a >= m) a -= m;
    return a;
}

 uint8_t mul8( uint8_t i, uint8_t j)
{
    return ((int)i * (int)(j) ) & 0xFF;
}

 uint8_t qmul8( uint8_t i, uint8_t j)
{
    int p = ((int)i * (int)(j) );
    if( p > 255) p = 255;
    return p;
}

 int8_t abs8( int8_t i)
{
    if( i < 0) i = -i;
    return i;
}

uint8_t sqrt16(uint16_t x)
{
    if( x <= 1) {
        return x;
    }

    uint8_t low = 1; // lower bound
    uint8_t hi, mid;

    if( x > 7904) {
        hi = 255;
    } else {
        hi = (x >> 5) + 8; // initial estimate for upper bound
    }

    do {
        mid = (low + hi) >> 1;
        if ((uint16_t)(mid * mid) > x) {
            hi = mid - 1;
        } else {
            if( mid == 255) {
                return 255;
            }
            low = mid + 1;
        }
    } while (hi >= low);

    return low - 1;
}

uint8_t blend8( uint8_t a, uint8_t b, uint8_t amountOfB)
{
    uint16_t partial;
    uint8_t result;
    
    uint8_t amountOfA = 255 - amountOfB;
    
    partial = (a * amountOfA);
    partial += (b * amountOfB);
    result = partial >> 8;
    
    return result;
    
}

struct CHSV {
	union {
		struct {
			union {
				uint8_t hue;
				uint8_t h;
			};
			union {
				uint8_t saturation;
				uint8_t sat;
				uint8_t s;
			};
			union {
				uint8_t value;
				uint8_t val;
				uint8_t v;
			};
		};
		uint8_t raw[3];
	};
};


struct CRGBW {
	union {
		struct {
			union {
				uint8_t	w;
				uint8_t white;
			};
			union {
				uint8_t r;
				uint8_t Red;
			};
			union {
				uint8_t g;
				uint8_t green;
			};
			union {
				uint8_t b;
				uint8_t	blue;
			};
		};
	};
	uint32_t raw;
} ;

void led2rgb( ws2811_led_t m, struct CRGBW *color) 
{
	color->w = (m >> 24) & 0xff;
	color->r = ((m & 0x00ff0000) >> 16);
	color->g = ((m & 0x0000ff00) >> 8);
	color->b = m & 0x000000ff;
}


void nscale8x3( uint8_t *r, uint8_t *g, uint8_t *b, fract8 scale)
{
    *r = ((int)*r * (int)(scale) ) >> 8;
    *g = ((int)*g * (int)(scale) ) >> 8;
    *b = ((int)*b * (int)(scale) ) >> 8;
}

void nscale8( ws2811_led_t m[] , uint16_t num_leds, uint8_t scale)
{
    for( uint16_t i = 0; i < num_leds; i++) {
	struct CRGBW color;
	led2rgb(m[i], &color);		// extract RGB values
	nscale8x3(&color.r, &color.g, &color.b, scale);
	m[i] = color.w << 24 | color.r << 16 | color.g << 8 | color.b;
    }
}

void fadeToBlackBy(ws2811_led_t *m, uint8_t fadefactor) {
	nscale8(m, 1, 255-fadefactor);
}


/*
void nscale8x3_video( uint8_t& r, uint8_t& g, uint8_t& b, fract8 scale)
{
    uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
    r = (r == 0) ? 0 : (((int)r * (int)(scale) ) >> 8) + nonzeroscale;
    g = (g == 0) ? 0 : (((int)g * (int)(scale) ) >> 8) + nonzeroscale;
    b = (b == 0) ? 0 : (((int)b * (int)(scale) ) >> 8) + nonzeroscale;
}

void nscale8x2( uint8_t& i, uint8_t& j, fract8 scale)
{
    i = ((uint16_t)i * (uint16_t)(scale) ) >> 8;
    j = ((uint16_t)j * (uint16_t)(scale) ) >> 8;
}

void nscale8x2_video( uint8_t& i, uint8_t& j, fract8 scale)
{
    uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
    i = (i == 0) ? 0 : (((int)i * (int)(scale) ) >> 8) + nonzeroscale;
    j = (j == 0) ? 0 : (((int)j * (int)(scale) ) >> 8) + nonzeroscale;
}
*/

uint16_t scale16by8( uint16_t i, fract8 scale )
{
    uint16_t result;
    result = (i * scale) / 256;
    return result;
}

uint16_t scale16( uint16_t i, fract16 scale )
{
    uint16_t result;
    result = ((uint32_t)(i) * (uint32_t)(scale)) / 65536;
    return result;
}

uint8_t dim8_raw( uint8_t x)
{
    return scale8( x, x);
}

uint8_t dim8_video( uint8_t x)
{
    return scale8_video( x, x);
}

uint8_t dim8_lin( uint8_t x )
{
    if( x & 0x80 ) {
        x = scale8( x, x);
    } else {
        x += 1;
        x /= 2;
    }
    return x;
}

uint8_t brighten8_raw( uint8_t x)
{
    uint8_t ix = 255 - x;
    return 255 - scale8( ix, ix);
}

uint8_t brighten8_video( uint8_t x)
{
    uint8_t ix = 255 - x;
    return 255 - scale8_video( ix, ix);
}

uint8_t brighten8_lin( uint8_t x )
{
    uint8_t ix = 255 - x;
    if( ix & 0x80 ) {
        ix = scale8( ix, ix);
    } else {
        ix += 1;
        ix /= 2;
    }
    return 255 - ix;
}


#define APPLY_DIMMING(X) (X)
#define HSV_SECTION_6 (0x20)
#define HSV_SECTION_3 (0x40)
#define RGB2INT(x) (x.w << 24 | x.r << 16 | x.g << 8 | x.b);

#define MIN(a,b)      ((a) < (b) ? (a) : (b))
#define MAX(a,b)      ((a) > (b) ? (a) : (b))

#define MIN3(a,b,c)   MIN((a), MIN((b), (c)))
#define MAX3(a,b,c)   MAX((a), MAX((b), (c)))

// ************
// 
// shamelessly borrowed from FastLED
//
// ************

#define FIXFRAC8(N,D) (((N)*256)/(D))

// This function is only an approximation, and it is not
// nearly as fast as the normal HSV-to-RGB conversion.
// See extended notes in the .h file.
void rgb2hsv( struct CRGBW *rgb, struct CHSV *hsv)
{
    uint8_t r = rgb->r;
    uint8_t g = rgb->g;
    uint8_t b = rgb->b;
    uint8_t h, s, v;
    
    // find desaturation
    uint8_t desat = 255;
    if( r < desat) desat = r;
    if( g < desat) desat = g;
    if( b < desat) desat = b;
    
    // remove saturation from all channels
    r -= desat;
    g -= desat;
    b -= desat;
    
    // saturation is opposite of desaturation
    s = 255 - desat;
    
    if( s != 255 ) {
        // undo 'dimming' of saturation
        s = 255 - sqrt16( (255-s) * 256);
    }
    
    
    // at least one channel is now zero
    // if all three channels are zero, we had a
    // shade of gray.
    if( (r + g + b) == 0) {
        // we pick hue zero for no special reason
	hsv->h = 0;
	hsv->s = 0;
	hsv->v = 255 - s;
        return ;
    }
    
    // scale all channels up to compensate for desaturation
    if( s < 255) {
        if( s == 0) s = 1;
        uint32_t scaleup = 65535 / (s);
        r = ((uint32_t)(r) * scaleup) / 256;
        g = ((uint32_t)(g) * scaleup) / 256;
        b = ((uint32_t)(b) * scaleup) / 256;
    }
    
    uint16_t total = r + g + b;
    
    // scale all channels up to compensate for low values
    if( total < 255) {
        if( total == 0) total = 1;
        uint32_t scaleup = 65535 / (total);
        r = ((uint32_t)(r) * scaleup) / 256;
        g = ((uint32_t)(g) * scaleup) / 256;
        b = ((uint32_t)(b) * scaleup) / 256;
    }
    
    if( total > 255 ) {
        v = 255;
    } else {
        v = qadd8(desat,total);
        // undo 'dimming' of brightness
        if( v != 255) v = sqrt16( v * 256);
        // without lib8tion: float ... ew ... sqrt... double ew, or rather, ew ^ 0.5
        // if( v != 255) v = (256.0 * sqrt( (float)(v) / 256.0));
        
    }
    
#if 0
    
    //#else
    if( v != 255) {
        // this part could probably use refinement/rethinking,
        // (but it doesn't overflow & wrap anymore)
        uint16_t s16;
        s16 = (s * 256);
        s16 /= v;
        //Serial.print("s16="); Serial.print(s16); Serial.println("");
        if( s16 < 256) {
            s = s16;
        } else {
            s = 255; // clamp to prevent overflow
        }
    }
#endif
    
    // since this wasn't a pure shade of gray,
    // the interesting question is what hue is it
    
    
    
    // start with which channel is highest
    // (ties don't matter)
    uint8_t highest = r;
    if( g > highest) highest = g;
    if( b > highest) highest = b;
    
    if( highest == r ) {
        // Red is highest.
        // Hue could be Purple/Pink-Red,Red-Orange,Orange-Yellow
        if( g == 0 ) {
            // if green is zero, we're in Purple/Pink-Red
            h = (HUE_PURPLE + HUE_PINK) / 2;
            h += scale8( qsub8(r, 128), FIXFRAC8(48,128));
        } else if ( (r - g) > g) {
            // if R-G > G then we're in Red-Orange
            h = HUE_RED;
            h += scale8( g, FIXFRAC8(32,85));
        } else {
            // R-G < G, we're in Orange-Yellow
            h = HUE_ORANGE;
            h += scale8( qsub8((g - 85) + (171 - r), 4), FIXFRAC8(32,85)); //221
        }
        
    } else if ( highest == g) {
        // Green is highest
        // Hue could be Yellow-Green, Green-Aqua
        if( b == 0) {
            // if Blue is zero, we're in Yellow-Green
            //   G = 171..255
            //   R = 171..  0
            h = HUE_YELLOW;
            uint8_t radj = scale8( qsub8(171,r),   47); //171..0 -> 0..171 -> 0..31
            uint8_t gadj = scale8( qsub8(g,171),   96); //171..255 -> 0..84 -> 0..31;
            uint8_t rgadj = radj + gadj;
            uint8_t hueadv = rgadj / 2;
            h += hueadv;
            //h += scale8( qadd8( 4, qadd8((g - 128), (128 - r))),
            //             FIXFRAC8(32,255)); //
        } else {
            // if Blue is nonzero we're in Green-Aqua
            if( (g-b) > b) {
                h = HUE_GREEN;
                h += scale8( b, FIXFRAC8(32,85));
            } else {
                h = HUE_AQUA;
                h += scale8( qsub8(b, 85), FIXFRAC8(8,42));
            }
        }
        
    } else /* highest == b */ {
        // Blue is highest
        // Hue could be Aqua/Blue-Blue, Blue-Purple, Purple-Pink
        if( r == 0) {
            // if red is zero, we're in Aqua/Blue-Blue
            h = HUE_AQUA + ((HUE_BLUE - HUE_AQUA) / 4);
            h += scale8( qsub8(b, 128), FIXFRAC8(24,128));
        } else if ( (b-r) > r) {
            // B-R > R, we're in Blue-Purple
            h = HUE_BLUE;
            h += scale8( r, FIXFRAC8(32,85));
        } else {
            // B-R < R, we're in Purple-Pink
            h = HUE_PURPLE;
            h += scale8( qsub8(r, 85), FIXFRAC8(32,85));
        }
    }
    
    h += 1;
    hsv->v = v;
    hsv->s = s;
    hsv->h = h;
}

void hsv2rgb( struct CHSV *hsv, struct CRGBW *rgb ) {
    // Convert hue, saturation and brightness ( HSV/HSB ) to RGB
    // "Dimming" is used on saturation and brightness to make
    // the output more visually linear.

    // Apply dimming curves
    uint8_t value = APPLY_DIMMING( hsv->val);
    uint8_t saturation = hsv->sat;

    // The brightness floor is minimum number that all of
    // R, G, and B will be set to.
    uint8_t invsat = APPLY_DIMMING( 255 - saturation);
    uint8_t brightness_floor = (value * invsat) / 256;

    // The color amplitude is the maximum amount of R, G, and B
    // that will be added on top of the brightness_floor to
    // create the specific hue desired.
    uint8_t color_amplitude = value - brightness_floor;

    // Figure out which section of the hue wheel we're in,
    // and how far offset we are withing that section
    uint8_t section = hsv->hue / HSV_SECTION_3; // 0..2
    uint8_t offset = hsv->hue % HSV_SECTION_3;  // 0..63

    uint8_t rampup = offset; // 0..63
    uint8_t rampdown = (HSV_SECTION_3 - 1) - offset; // 63..0


    // compute color-amplitude-scaled-down versions of rampup and rampdown
    uint8_t rampup_amp_adj   = (rampup   * color_amplitude) / (256 / 4);
    uint8_t rampdown_amp_adj = (rampdown * color_amplitude) / (256 / 4);

    // add brightness_floor offset to everything
    uint8_t rampup_adj_with_floor   = rampup_amp_adj   + brightness_floor;
    uint8_t rampdown_adj_with_floor = rampdown_amp_adj + brightness_floor;


    if( section ) {
        if( section == 1) {
            // section 1: 0x40..0x7F
            rgb->r = brightness_floor;
            rgb->g = rampdown_adj_with_floor;
            rgb->b = rampup_adj_with_floor;
        } else {
            // section 2; 0x80..0xBF
            rgb->r = rampup_adj_with_floor;
            rgb->g = brightness_floor;
            rgb->b = rampdown_adj_with_floor;
        }
    } else {
        // section 0: 0x00..0x3F
        rgb->r = rampdown_adj_with_floor;
        rgb->g = rampup_adj_with_floor;
        rgb->b = brightness_floor;
    }
}

struct CRGBW RED=   {00, 0xff, 00, 00};
struct CRGBW GREEN= {00, 00, 0xff, 00};
struct CRGBW BLUE = {00, 00, 00, 0xff};

void Show(ws2811_led_t m[], struct ws2811_t *l) {
	int ret;

	for (int i = 0; i < LED_COUNT; i++)
		l[0].channel[0].leds[i] = m[i];	

	if ((ret = ws2811_render(l)) != WS2811_SUCCESS)
        {
            fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
        }
}


// Clear the matrix from start s to end e
void Clear(ws2811_led_t m[], int s, int e) {
	for (int i = s; i < e ; i++)
	        m[i] = 0;
}


void fill_solid(ws2811_led_t m[], int numToFill, struct CRGBW *color) {
	for (int i = 0; i < numToFill; i++) {
		m[i] = color->w << 24 | color->r << 16 | color->g << 8 | color->b;
	}
}

void fill_rainbow(ws2811_led_t m[], int numToFill, uint8_t hue, uint8_t hue_step) {
	struct CHSV hsv;
        struct CRGBW rgb;

	hsv.v = 255;
	hsv.s = 255;
	hsv.h = hue;
	for (int i = 0; i < numToFill; i++) {
		hsv2rgb(&hsv, &rgb);
                m[i] = RGB2INT(rgb);
		hsv.h += hue_step;
        }
}

uint32_t millis() {
	struct timespec tp;
        static int      oldms = 0;
        clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
        int ms=tp.tv_sec + tp.tv_nsec/1000000;                      // get milliseconds
	return ms;
}
