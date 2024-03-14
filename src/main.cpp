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
#include "pathImageFile.h"

RFIDTagJson tagJson;
RFIDUart rfidUart;

// ダイヤルポジション変数
long oldPosition = -999;
long newPosition = 0;

// かばんの中にものが存在するかどうか
bool isExistTag = false;

String existTagImage[3] = {PATH_MENU_GREEN_BELL, PATH_MENU_GREEN_SETTING,
                           PATH_MENU_GREEN_ADD};
int existTagImageLength = sizeof(existTagImage) / sizeof(existTagImage[0]);
int existTagImageIndex = 0;

String notExistTagImage[3] = {PATH_MENU_RED_BELL, PATH_MENU_RED_SETTING,
                              PATH_MENU_RED_ADD};
int notExistTagImageLength =
    sizeof(notExistTagImage) / sizeof(notExistTagImage[0]);
int notExistTagImageIndex = 0;

// メニュー画面の選択肢
enum Loops
{
    MENU,
    SETTING,
    ADDTAG,
    TAGLIST
};

// 現在の選択肢
Loops currentLoops = MENU;

// 各画面のループ関数
void loop_menu(String tagImage[], int tagImageLength, int tagImageIndex);
void loop_setting();
void loop_addTag();
void loop_tagList();

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

    // 状態によりループ関数を切り替える
    switch (currentLoops) {
        case MENU:
            if (isExistTag) {
                loop_menu(existTagImage, existTagImageLength,
                          existTagImageIndex);
            } else {
                loop_menu(notExistTagImage, notExistTagImageLength,
                          notExistTagImageIndex);
            }
            break;
        case SETTING:
            loop_setting();
            break;
        case ADDTAG:
            loop_addTag();
            break;
        case TAGLIST:
            loop_tagList();
            break;
    }

    // // ダイヤルがひねられたときの処理
    // if (newPosition != oldPosition) {
    //     oldPosition = newPosition;
    //     if (newPosition < 0) {
    //         newPosition *= -1;
    //     }
    //     uint8_t num = newPosition % 2;
    //     M5Dial.Speaker.tone(8000, 20);
    //     // これを入れると画面が切り替わるたびに黒飛びになる
    //     // M5Dial.Display.clear();
    //     if (num == 0) {
    //         M5.Lcd.drawJpgFile(SPIFFS, "/Menu_red.jpg", 4, 4);
    //     } else if (num == 1) {
    //         M5.Lcd.drawJpgFile(SPIFFS, "/Menu_green.jpg", 4, 4);
    //     }
    // }

    // 遅延を入れないとダイヤルの挙動がおかしくなる
    delay(1);
}

// メニュー画面のループ関数
void loop_menu(String tagImage[], int tagImageLength, int tagImageIndex) {
    newPosition = M5Dial.Encoder.read();

    // ダイヤルがひねられたときの処理
    if (newPosition != oldPosition) {
        M5Dial.Speaker.tone(8000, 20);
        tagImageIndex = changeImageIndex(tagImageIndex, tagImageLength,
                                         newPosition - oldPosition);
        oldPosition = newPosition;
        M5.Lcd.drawJpgFile(SPIFFS, tagImage[tagImageIndex], 0, 0);
        existTagImageIndex = tagImageIndex;
        notExistTagImageIndex = tagImageIndex;
    }

    // ボタンが押されたときの処理
    if (M5Dial.BtnA.wasPressed()) {
        switch (tagImageIndex) {
            case 0:
                currentLoops = TAGLIST;
                break;
            case 1:
                currentLoops = SETTING;
                break;
            case 2:
                currentLoops = ADDTAG;
                break;
        }
    }
}

// 画像のインデックスを変更する関数
int changeImageIndex(int index, int length, int direction) {
    // ダイヤルを時計回りに回したとき
    if (direction > 0) {
        index++;
        if (index >= length) {
            index = 0;
        }
    }
    // ダイヤルを反時計回りに回したとき
    else if (direction < 0) {
        index--;
        if (index < 0) {
            index = length - 1;
        }
    }
    return index;
}