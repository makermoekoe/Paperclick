#include <Arduino.h>
#include <Wire.h>


// Hardware Definitions
#define SPI_EPD_CLK     6
#define SPI_EPD_MOSI    7
#define SPI_EPD_MISO    -1
#define SPI_EPD_CS      10
#define SPI_EPD_BUSY    20
#define SPI_EPD_RST     21
#define SPI_EPD_DC      0

#define ADC_ENABLE_PIN  1
#define WS2812_PIN      3
#define ADC_PIN         4
#define BUTTON1_PIN     5
#define BUTTON2_PIN     8
#define BUTTON3_PIN     2


#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include "GxEPD2_display_selection_new_style.h"



struct fonts {
  const GFXfont* font;
  int width;
  int height;
};

fonts font_xl = {&FreeSansBold9pt7b, 9, 19};
// fonts font_m = {u8g2_font_profont17_mf, 9, 11};



struct partial_box {
  int left, top, width, height;

  void clear(bool black){
    display.setPartialWindow(left, top, width, height);
    display.firstPage();
    do{
      if(black) display.fillRect(left, top, width, height, GxEPD_BLACK);
      else display.fillRect(left, top, width, height, GxEPD_WHITE);
    }
    while (display.nextPage());
  }

  void print_text(String str, fonts f) {
    display.setPartialWindow(left, top, width, height);
    display.setFont(f.font);
    display.setTextColor(GxEPD_BLACK);

    int16_t tbx, tby; uint16_t tbw, tbh;
    display.getTextBounds(str, left, top, &tbx, &tby, &tbw, &tbh);
    uint16_t x = (left + width/2 - tbw / 2 - 1);
    display.getTextBounds(str, left, top, &tbx, &tby, &tbw, &tbh);
    uint16_t y = top + height/2 + tbh/2 - 1;

    display.firstPage();
    do{
      display.fillRect(left, top, width, height, GxEPD_WHITE);
      display.setCursor(x, y);
      display.print(str);
    }
    while (display.nextPage());
  }

  void print_tap_circle() {
    display.setPartialWindow(left, top, width, height);
    display.setFont(font_xl.font);
    display.setTextColor(GxEPD_BLACK);
    
    int16_t tbx, tby; uint16_t tbw, tbh;
    display.getTextBounds("TAP", left, top, &tbx, &tby, &tbw, &tbh);
    uint16_t x1 = (left + width/2 - tbw / 2 - 1);
    display.getTextBounds("HERE", left, top, &tbx, &tby, &tbw, &tbh);
    uint16_t x2 = (left + width/2 - tbw / 2 - 1);
    display.getTextBounds("TAP", left, top, &tbx, &tby, &tbw, &tbh);
    uint16_t y = top + height/2 + tbh/2 - 1;

    display.firstPage();
    do{
      display.fillRect(left, top, width, height, GxEPD_WHITE);
      display.drawCircle(left+width/2, top+height/2, width/2-1, GxEPD_BLACK);
      display.drawCircle(left+width/2, top+height/2, width/2-2, GxEPD_BLACK);
      display.setCursor(x1, y-10);
      display.print("TAP");
      display.setCursor(x2, y+7);
      display.print("HERE");
    }
    while (display.nextPage());
  }
};

// Initializing the boxes
partial_box main_circle = {10, 10, display.width() - 20, display.width() - 20};
partial_box main_text = {10, main_circle.top + main_circle.height, display.width() - 20, 50};



void setup(){

  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);
  pinMode(BUTTON3_PIN, INPUT);
  pinMode(ADC_ENABLE_PIN, OUTPUT);
  pinMode(ADC_PIN, INPUT);

  SPI.end(); // release standard SPI pins, e.g. SCK(18), MISO(19), MOSI(23), SS(5)
  SPI.begin(SPI_EPD_CLK, SPI_EPD_MISO, SPI_EPD_MOSI, SPI_EPD_CS); // map and init SPI pins SCK(13), MISO(12), MOSI(14), SS(15)
  display.init(0UL, false);
  display.setRotation(4);
  display.setFont(font_xl.font);
  display.setTextColor(GxEPD_BLACK);
  // display.hibernate();

  // main_circle.clear(false);

  const char hello_str[] = "Hello!";
  main_text.print_text(hello_str, font_xl);
  delay(3000);

  display.fillScreen(GxEPD_WHITE);
  display.display();

  main_circle.print_tap_circle();

  esp_sleep_enable_timer_wakeup(5000000);
  esp_deep_sleep_start();
}


void loop() {
  
}


// display.setFont(font_xl.font);
// display.setTextColor(GxEPD_BLACK);
// int16_t tbx, tby; uint16_t tbw, tbh;
// // align with centered HelloWorld
// display.getTextBounds(hello_str, 0, 0, &tbx, &tby, &tbw, &tbh);
// uint16_t x = ((display.width() - tbw) / 2) - tbx;
// // height might be different
// display.getTextBounds(hello_str, 0, 0, &tbx, &tby, &tbw, &tbh);
// uint16_t y = (display.height() * 3 / 4) + tbh / 2; // y is base line!
// // full window mode is the initial mode, set it anyway
// display.setFullWindow();
// // DO NOT display.fillScreen(GxEPD_WHITE); keep current content!
// // display.fillRect(0, wy, display.width(), wh, GxEPD_WHITE); // clear rect instead
// display.fillScreen(GxEPD_WHITE);
// display.setCursor(x, y);
// display.print(hello_str);
// display.display();
// delay(1000);