#include <Arduino.h>
#include <Wire.h>

#include <WiFi.h>
#include <TimeLib.h>
#include <WiFiUdp.h>


const char* ssid     = "WLAN-VTXKHJ";
const char* password = "HOU08BAKLK32a";

static const char ntpServerName[] = "us.pool.ntp.org";
const int timeZone = 2;     // Central European Time

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets


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


/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

time_t getNtpTime(){
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  WiFi.hostByName(ntpServerName, ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  return 0;
}

String get_formatted_time(){
  String str = "";
  if(hour() < 10) str += "0";
  str += String(hour()) + ":";
  if(minute() < 10) str += "0";
  str += String(minute()) + ":";
  if(second() < 10) str += "0";
  str += String(second());
  return str;
}

String get_formatted_date(){
  String str = "";
  if(day() < 10) str += "0";
  str += String(day()) + ".";
  if(month() < 10) str += "0";
  str += String(month()) + ".";
  str += String(year()-2000);
  return str;
}



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
partial_box main_text_1 = {5, main_circle.top + main_circle.height + 5, display.width() - 10, 25};
partial_box main_text_2 = {5, main_text_1.top + main_text_1.height, display.width() - 10, 25};


float get_battery_voltage(){
  digitalWrite(ADC_ENABLE_PIN, LOW);
  delay(2);
  int sum = 0;
  for(int i=0; i<1000; i++){
    sum = sum + analogRead(ADC_PIN);
  }
  int result = sum/1000;
  return float(result) * (1.437);
}


void setup(){

  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);
  pinMode(BUTTON3_PIN, INPUT);
  pinMode(ADC_ENABLE_PIN, OUTPUT);
  pinMode(ADC_PIN, INPUT);
  analogReadResolution(12);

  digitalWrite(ADC_ENABLE_PIN, HIGH);

  SPI.end(); // release standard SPI pins, e.g. SCK(18), MISO(19), MOSI(23), SS(5)
  SPI.begin(SPI_EPD_CLK, SPI_EPD_MISO, SPI_EPD_MOSI, SPI_EPD_CS); // map and init SPI pins SCK(13), MISO(12), MOSI(14), SS(15)
  display.init(0UL, true);
  display.setRotation(4);
  display.setFont(font_xl.font);
  display.setTextColor(GxEPD_BLACK);
  // display.hibernate();

  // main_circle.clear(false);

  const char hello_str[] = "Hello!";
  const char connecting_str[] = "Wifi...";
  main_text_1.print_text(hello_str, font_xl);
  delay(500);
  main_text_2.print_text(connecting_str, font_xl);


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED && millis() < 20000) {
    delay(500);
    printf(".\r\n");
  }
  printf(".\r\n");

  if(WiFi.status() == WL_CONNECTED){
    Udp.begin(localPort);

    setSyncProvider(getNtpTime);
    setSyncInterval(300);

    now();

    main_text_1.print_text(get_formatted_date(), font_xl);
    main_text_2.print_text(get_formatted_time(), font_xl);
  }
  else{
    printf("unable to connect\r\n");
    main_text_1.print_text("unable to connect", font_xl);
  }

  delay(1000);

  // display.fillScreen(GxEPD_WHITE);
  // display.display(false);

  main_circle.print_tap_circle();

  esp_sleep_enable_timer_wakeup(5000000);
  esp_deep_sleep_start();
}

time_t prevDisplay = 0;


void loop() {
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();
      printf("%s \t %s \r\n", get_formatted_date(), get_formatted_time());
    }
  }
}