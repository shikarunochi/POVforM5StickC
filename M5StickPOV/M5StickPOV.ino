#include <M5StickC.h>
#include <Adafruit_NeoPixel.h>
#include "helloM5_120.h"
#define PIN 32
#define NUMPIXELS 8
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//回転検出用
#define ANALOG_PIN        33
int resistorValueSum = 0; //フォトレジスタ読み込み値の累計
int resistorReadCount = 0; //フォトレジスタ読み込んだ回数
int detectOriginCount = 0; //フォトレジスタが明るくなった（基準位置）を検出した回数
int checkOrignFlag = false; //フォトレジスタが明るくなったら次に暗くなるまでTRUE
unsigned long lastOriginTime; //前回フォトレジスタが明るくなった（基準位置）を検出したときの時間
#define THRESHOLD_VALUE 100

boolean showFlag = false;

#define ROTATION_MS 15
unsigned long delayUs = ROTATION_MS * 1000 / DIVCOUNT;

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
  M5.begin();
  pinMode(M5_BUTTON_HOME, INPUT);
  pinMode(M5_BUTTON_RST, INPUT);
  pixels.begin();
  delay(1000);
  pixels.setBrightness(30);
  resetPixel();
  showFlag = false;

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setCursor(7, 0, 2);
  M5.Lcd.println("Ready!");


}
void loop() {
  while (true) {
    //１回転チェック処理

    int readResistorValue = analogRead(ANALOG_PIN);
    if (resistorValueSum > 0 && (resistorValueSum / resistorReadCount) * THRESHOLD_VALUE < readResistorValue) { //平均値のTHRESHOLD_VALUE倍の光を検出したら、LEDの位置に来たと判定
      //OFF->ONになった。
      if (checkOrignFlag == false) {
        checkOrignFlag = true;
        unsigned long nowTime = micros();
        if (nowTime - lastOriginTime > 1000 * 1000) {
          detectOriginCount = 0;//回転止まったとみなす。
        } else {
          detectOriginCount = detectOriginCount + 1;
        }
        //3周続いたらチェック開始
        if (detectOriginCount > 3) {
          //１周にかかった時間はnowTime - lastOriginTime
          delayUs = (nowTime - lastOriginTime) / (DIVCOUNT + 1); //今回の分を足して割る
          if (detectOriginCount == 5) {
            M5.Lcd.print("Diff:");
            M5.Lcd.println(nowTime - lastOriginTime);
            M5.Lcd.print("adjust!:");
            M5.Lcd.println(delayUs);
          }

          //この処理に来たら、実際の表示を開始する。
          if (showFlag == false) {
            showFlag = true;
            M5.Lcd.fillScreen(BLACK);
          }

        }
        lastOriginTime = nowTime;

        //M5.Lcd.print("adjust!:");
        //M5.Lcd.println(delayUs);

      } else {

      }
    } else {
      if(readResistorValue <= 0){ //読み込み値0になる場合もあるので最低値1にしておく。
        readResistorValue = 1;
      }
      resistorValueSum = resistorValueSum + readResistorValue;
      resistorReadCount++;
      checkOrignFlag = false;
    }

    if (showFlag == true) {
      //ここで表示すべきデータは、(原点の時間から経過した時間) / (１つのデータを表示する時間)
      unsigned long nowTime = micros();
      if (nowTime - lastOriginTime > delayUs * DIVCOUNT * 2) { //2周の時間、LED検出がなければLEDをOFF
        resetPixel();
        showFlag = false;
      } else {
        int dispIndex = ((nowTime - lastOriginTime) / delayUs ) % DIVCOUNT;
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, picDate[dispIndex][i + 2]); //下2つはLED無い場所なので表示しません
        }
        pixels.show();
      }
    }

    //オーバフロー防止
    if (resistorReadCount > 2000000) {
      resistorValueSum = resistorValueSum / resistorReadCount;
      resistorReadCount = 1;
    }

    //処理100回に1回、ボタンチェック
    if (resistorReadCount % 100 == 0) {
      if (digitalRead(M5_BUTTON_HOME) == LOW) {
        resetPixel();
        showFlag = false;
      }
    }
  }
}
