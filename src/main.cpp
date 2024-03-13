/*
ダイヤルで表示画像を切り替えるサンプルプログラム
プロジェクト環境をenv:m5stack-dial-m5unifiedに設定して書き込んでください
*/

#include "main.hpp"

#include <M5Dial.h>
#include <M5Unified.h>

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