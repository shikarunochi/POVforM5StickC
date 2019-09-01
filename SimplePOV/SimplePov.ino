#include <M5StickC.h>
#include <Adafruit_NeoPixel.h>
#include "helloM5_120.h"
#define PIN 32
#define NUMPIXELS 8
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//実測だと1周45ミリセカンドぐらい。
//試行錯誤の結果、1周（の、待ち時間合計）が15ミリセカンド設定だとうまくいった。
//環境によって異なると思います。
#define ROTATION_MS 15 
unsigned long delayUs = ROTATION_MS * 1000 / DIVCOUNT;

int ledPos = 0;

void resetPixel() {
  pixels.clear();
  delay(10);

  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, 0);
    delay(10);
  }
  delay(10);
  pixels.show();
}

void setup() {
  setCpuFrequencyMhz(240);
  M5.begin();
  pinMode(M5_BUTTON_HOME, INPUT);
  pinMode(M5_BUTTON_RST, INPUT);
  pixels.begin();
  delay(1000);
  pixels.setBrightness(30);
  resetPixel();

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setCursor(7, 0, 2);
  M5.Lcd.println("Ready!");
}
void loop() {
  while (true) { //loop()だとdelayが長すぎるので独自でループします。

    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, picDate[ledPos][i + 2]); //下2つはLED無い場所なので表示なし。
    }
    pixels.show();

    ledPos++;
    if (ledPos >= DIVCOUNT) {
      ledPos = 0;
    }

    delayMicroseconds(delayUs);

    //1回転に1回、ボタンチェック
    if (ledPos == 0) {
      if (digitalRead(M5_BUTTON_HOME) == LOW) { //HOMEボタンで動作停止
        //if(PIND & _BV(M5_BUTTON_HOME)) == LOW){
        resetPixel();
        while (true) {
          delay(1000);
          //画面消してPauseする。
        }
      }

      if (digitalRead(M5_BUTTON_RST) == LOW) { //Resetボタンで回転数微調整
        delayUs = delayUs + 10;
        if (delayUs > 1000) {
          delayUs = 100;
        }
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.setCursor(7, 0, 2);
        M5.Lcd.println(delayUs);
      }
    }
  }
}
