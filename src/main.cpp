/*
作成日 : 2024/03/13
作成者 : 黒田 直樹

忘れ物を検知するバッグのコントロール側プログラム
デバイス : M5Dial
外部ペリフェラル : NeoPixel(WS2812B)

・機能
    忘れ物の登録、削除、確認を行う
    LEDの点灯、消灯を行う
    ブザーの鳴動を行う
    各機能のON/OFFを行う

*/

#include "main.hpp"

#include <M5Dial.h>
#include <M5Unified.h>

#include "RFIDTagJson.hpp"
#include "RFIDUart.hpp"

RFIDTagJson tagJson;
RFIDUart rfidUart;

// ダイヤルポジション変数
long oldPosition = -999;
long newPosition = 0;

void setup() {
    M5_BEGIN();
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);

    SPIFFS.begin();

    M5_UPDATE();
}

void loop() {
    M5Dial.update();
    M5_UPDATE();

    newPosition = M5Dial.Encoder.read();

    // ダイヤルがひねられたときの処理
    if (newPosition != oldPosition) {
        oldPosition = newPosition;
        if (newPosition < 0) {
            newPosition *= -1;
        }
        uint8_t num = newPosition % 2;
        M5Dial.Speaker.tone(8000, 20);
        // これを入れると画面が切り替わるたびに黒飛びになる
        // M5Dial.Display.clear();
        if (num == 0) {
            M5.Lcd.drawJpgFile(SPIFFS, "/Menu_red.jpg", 4, 4);
        } else if (num == 1) {
            M5.Lcd.drawJpgFile(SPIFFS, "/Menu_green.jpg", 4, 4);
        }
    }

    // 遅延を入れないとダイヤルの挙動がおかしくなる
    delay(1);
}