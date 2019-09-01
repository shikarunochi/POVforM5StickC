#include <M5StickC.h>
#include <Adafruit_NeoPixel.h>
#include "clockNumber.h"
#include <WiFi.h>
#include "time.h"

#define PIN 32
#define NUMPIXELS 8
#define DIVCOUNT 60
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
unsigned long clockSecondCheck;

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

RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;

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
  lastOriginTime = micros();
  clockSecondCheck = millis();

  /*
    //リアルタイムクロックをコンパイル時間に設定する。必要であれば有効化してください。
    //https://macsbug.wordpress.com/2019/06/06/m5stickc-nixie-tube-clock/
    const char *monthName[12] = {
      "Jan", "Feb", "Mar", "Apr", "May", "Jun",
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    // rtc setup start ---------------------------------------------
    String pt = (__DATE__ " " __TIME__); // PC DATE TIME READ
    //000000000011111111112  Read data
    //012345678901234567890
    //Jun  6 2019 07:20:41
    char m1[3]; int  m2; // Month conversion ( Jun to 6 )
    (pt.substring(0,3)).toCharArray(m1,4);
    for (int mx = 0; mx < 12; mx ++) {
      if (strcmp(m1, monthName[mx]) == 0){m2 = mx + 1; break;}
    }
    RTC_DateTypeDef DateStruct;         // Month, Date, Year
    DateStruct.Month   = m2;
    DateStruct.Date    = (pt.substring(4, 6)).toInt();
    DateStruct.Year    = (pt.substring(7,11)).toInt();
    M5.Rtc.SetData(&DateStruct);
    RTC_TimeTypeDef TimeStruct;         // Hours, Minutes, Seconds
    TimeStruct.Hours   = (pt.substring(12,14)).toInt();
    TimeStruct.Minutes = (pt.substring(15,17)).toInt();
    TimeStruct.Seconds = (pt.substring(18,20)).toInt();
    M5.Rtc.SetTime(&TimeStruct);
    // rtc setup end -----------------------------------------------
  */
  /*
  //リアルタイムクロックをNTPサーバから設定する。必要であれば有効にしてください。
  //https://lang-ship.com/blog/?p=563
  const char* ssid       = "your_ssid";
  const char* password   = "your_password";
  const char* ntpServer =  "ntp.jst.mfeed.ad.jp";

  // connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");
 
  // Set ntp time to local
  configTime(9 * 3600, 0, ntpServer);
 
  // Get local time
  struct tm timeInfo;
  if (getLocalTime(&timeInfo)) {
    M5.Lcd.print("NTP : ");
    M5.Lcd.println(ntpServer);
 
    // Set RTC time
    RTC_TimeTypeDef TimeStruct;
    TimeStruct.Hours   = timeInfo.tm_hour;
    TimeStruct.Minutes = timeInfo.tm_min;
    TimeStruct.Seconds = timeInfo.tm_sec;
    M5.Rtc.SetTime(&TimeStruct);
 
    RTC_DateTypeDef DateStruct;
    DateStruct.WeekDay = timeInfo.tm_wday;
    DateStruct.Month = timeInfo.tm_mon + 1;
    DateStruct.Date = timeInfo.tm_mday;
    DateStruct.Year = timeInfo.tm_year + 1900;
    M5.Rtc.SetData(&DateStruct);
  }
 
  //disconnect WiFi
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  */
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
        //３周続いたらチェック開始
        if (detectOriginCount > 3) {
          //１周にかかった時間はnowTime - lastOriginTime
          //つまり、
          delayUs = (nowTime - lastOriginTime) / (DIVCOUNT + 1); //今回の分を足して割る

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
      if (readResistorValue <= 0) { //読み込み値0になる場合もあるので最低値1にしておく。
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
        int dispIndex = ((nowTime - lastOriginTime) / delayUs ) % DIVCOUNT - 10;
        int numIndex = -1;
        int posIndex = 0;
        //現在の場所によって、どの時刻の、どの位置を出すか決まる。
        //0～5 6～11：時
        uint32_t colorFilter = 0xFFFFFF;
        if (dispIndex >= 0 && dispIndex <= 5) {
          numIndex = (int)(RTC_TimeStruct.Hours / 10);
          posIndex = dispIndex;
          colorFilter = 0x00FF00;
        }
        if (dispIndex >= 6 && dispIndex <= 11) {
          numIndex = RTC_TimeStruct.Hours - (int)(RTC_TimeStruct.Hours / 10) * 10;
          posIndex = dispIndex - 6;
          colorFilter = 0x00FF00;
        }
        //12～17：コロン
        if (dispIndex >= 12 && dispIndex <= 17) {
          numIndex = 10;
          posIndex = dispIndex - 12;
        }
        //18～23 24～29 分
        if (dispIndex >= 18 && dispIndex <= 23) {
          numIndex = (int)(RTC_TimeStruct.Minutes / 10);
          posIndex = dispIndex - 18;
          colorFilter = 0x00FF00;
        }
        if (dispIndex >= 24 && dispIndex <= 29) {
          numIndex = RTC_TimeStruct.Minutes - (int)(RTC_TimeStruct.Minutes / 10) * 10;
          posIndex = dispIndex - 24;
          colorFilter = 0x00FF00;
        }
        //30～35：コロン
        if (dispIndex >= 30 && dispIndex <= 35) {
          numIndex = 10;
          posIndex = dispIndex - 30;
        }
        //36～41 42～47 秒
        if (dispIndex >= 36 && dispIndex <= 41) {
          numIndex = (int)(RTC_TimeStruct.Seconds / 10);
          posIndex = dispIndex - 36;
          colorFilter = 0x00FF00;
        }
        if (dispIndex >= 42 && dispIndex <= 47) {
          numIndex = RTC_TimeStruct.Seconds - (int)(RTC_TimeStruct.Seconds / 10) * 10;
          posIndex = dispIndex - 42;
          colorFilter = 0x00FF00;
        }
        //M5.Lcd.print(numIndex);
        //M5.Lcd.print(":");
        //M5.Lcd.println(posIndex);
        if (numIndex != -1) {
          for (int i = 0; i < NUMPIXELS; i++) {
            pixels.setPixelColor(i, picDate[numIndex][posIndex][i + 2] & colorFilter); //下2つはLED無い場所なので表示しません
          }
        } else {
          for (int i = 0; i < NUMPIXELS; i++) {
            pixels.setPixelColor(i, 0); //下2つはLED無い場所なので表示しません
          }
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
      //同じタイミングで時計更新
      M5.Rtc.GetTime(&RTC_TimeStruct);
    }
  }
}
