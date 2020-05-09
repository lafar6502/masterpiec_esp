/**
 * Copyright (c) 2017-2018 Tara Keeling
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ssd1306.h"
#include "ssd1306_draw.h"
#include "ssd1306_font.h"
#include "ssd1306_default_if.h"
#include "ui_screen.h"

#define USE_I2C_DISPLAY
//#define USE_SPI_DISPLAY

#if defined USE_I2C_DISPLAY
    static const int I2CDisplayAddress = 0x3C;
    static const int I2CDisplayWidth = 128;
    static const int I2CDisplayHeight = 64;
    static const int I2CResetPin = -1;

    struct SSD1306_Device I2CDisplay;
#endif

#if defined USE_SPI_DISPLAY
    static const int SPIDisplayChipSelect = 15;
    static const int SPIDisplayWidth = 128;
    static const int SPIDisplayHeight = 64;
    static const int SPIResetPin = 5;

    struct SSD1306_Device SPIDisplay;
#endif

void SetupDemo( struct SSD1306_Device* DisplayHandle, const struct SSD1306_FontDef* Font );
void SayHello( struct SSD1306_Device* DisplayHandle, const char* HelloText );

bool DefaultBusInit( void ) {
    #if defined USE_I2C_DISPLAY
        assert( SSD1306_I2CMasterInitDefault( ) == true );
        assert( SSD1306_I2CMasterAttachDisplayDefault( &I2CDisplay, I2CDisplayWidth, I2CDisplayHeight, I2CDisplayAddress, I2CResetPin ) == true );
    #endif

    #if defined USE_SPI_DISPLAY
        assert( SSD1306_SPIMasterInitDefault( ) == true );
        assert( SSD1306_SPIMasterAttachDisplayDefault( &SPIDisplay, SPIDisplayWidth, SPIDisplayHeight, SPIDisplayChipSelect, SPIResetPin ) == true );
    #endif

    return true;
}

void SetupDemo( struct SSD1306_Device* DisplayHandle, const struct SSD1306_FontDef* Font ) {
    SSD1306_Clear( DisplayHandle, SSD_COLOR_BLACK );
    SSD1306_SetFont( DisplayHandle, Font );
}

void SayHello( struct SSD1306_Device* DisplayHandle, const char* HelloText ) {
    SSD1306_FontDrawAnchoredString( DisplayHandle, TextAnchor_Center, HelloText, SSD_COLOR_WHITE );
    SSD1306_Update( DisplayHandle );
}



struct FontInf {
    const struct SSD1306_FontDef* pFont;
    uint8_t width;
    uint8_t height; 
};

const struct SSD1306_FontDef* g_font[] = {
    &Font_droid_sans_mono_16x31,
    &Font_liberation_mono_17x30,
    &Font_droid_sans_fallback_24x28,
    &Font_droid_sans_mono_13x24,
    &Font_liberation_mono_13x21,
    &Font_droid_sans_fallback_15x17,
    &Font_liberation_mono_9x15,
    &Font_droid_sans_fallback_11x13,
    &Font_droid_sans_mono_7x13,
};


const struct SSD1306_FontDef* get_font_for(const char* line, uint8_t maxHeight) {
    //widths: 7, 9, 11, 13, 15, 16, 17, 24, 32
    //heights: 13, 15, 17, 21, 24, 28, 30, 31, 32, 64

    int len = strlen(line);
    for(int i=0; i<sizeof(g_font)/sizeof( struct SSD1306_FontDef*); i++) {
        if (g_font[i]->Height <= maxHeight) {
            int maxlen = I2CDisplay.Width / g_font[i]->Width;
            if (len <= maxlen) {
                printf("for %s selected font %d\n", line, i);
                return g_font[i];
            }
        };
    }
    return  &Font_droid_sans_mono_7x13;
}

//returns height of the selected font
int printline(const char* txt, int y, int maxh) {
    
    struct SSD1306_FontDef* f = get_font_for(txt, maxh);
    SSD1306_SetFont(&I2CDisplay, f);
    SSD1306_FontDrawString(&I2CDisplay, 0, y, txt, SSD_COLOR_WHITE);
    return f->Height; 
}


void print4s(const char* t1, const char* t2, const char* t3, const char* t4) {
    int cury = 0;
    int n = 0;
    const char* pp[] = {t1, t2, t3, t4};
    SSD1306_Clear( &I2CDisplay, SSD_COLOR_BLACK );
    for(int i=0; i<4; i++) {
        if (pp[i] != NULL) n++;
    }
    if (n == 0) return;
    int lh = I2CDisplay.Height / n;
    for(int i=0; i<4; i++) {
        if (pp[i] == NULL) continue;
        cury += printline(pp[i], cury, lh);
    }
    SSD1306_Update( &I2CDisplay );
}

void print1s(const char* txt) {
    SSD1306_Clear( &I2CDisplay, SSD_COLOR_BLACK );
    printline(txt, 10, 44);
    SSD1306_Update( &I2CDisplay );
}


void print2s(const char* txt, const char* txt2) 
{
    
    print4s(txt, txt2, NULL, NULL);
}


void initializeDisplay2( void ) {
    printf( "Ready...\n" );

    if (!DefaultBusInit()) {
        printf("Failed to init display");
        return;
    }

    char buf[50];

    vTaskDelay(3000 / portTICK_PERIOD_MS);
    for (int i=0; i<50; i++) {
        print1s("hello");
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        print1s("world haha!!!");
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        print2s("BOO", "hoo hoo hoo hoo hoo");
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        print2s("T=55.3", "T2=52.0");
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        print2s("witamy w naszej", "bajce, slon zagra na");
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        print1s("MM");
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        print4s("A", "oto", "mamy tu takie", "ciekawostki");
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        print4s("siala baba mak", "nie wiedziala jak", "a dziad", "dziadowal");
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        print4s("siala baba mak", "nie wiedziala jak", NULL, "a dziad nie");
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        
        
    }
    
    
}

