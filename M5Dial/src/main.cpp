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

#include <Adafruit_NeoPixel.h>
#include <M5Dial.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "RFIDTagJson.hpp"
#include "RFIDUart.hpp"
#include "config.h"
#include "pathImageFile.h"

#define LED_PIN    13  // INが接続されているピンを指定
#define NUM_PIXELS 15  // LEDの数を指定
Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN,
                         NEO_GRB + NEO_KHZ800);  // 800kHzでNeoPixelを駆動

#define HEIGHT_INTERVAL        40
#define READING_INTERVAL       30
#define READING_CLEAR_INTERVAL 100

RFIDTagJson tagJson;
RFIDUart rfidUart;

TaskHandle_t ledTaskHandle = NULL;       // LED点灯タスクハンドル
TaskHandle_t tagExistTaskHandle = NULL;  // かばん内監視タスクハンドル

// 登録するもののカテゴリ
String category[] = {"サイフ", "カギ",         "スマホ", "パソコン",
                     "ノート", "カードケース", "その他"};
int categoryLength = sizeof(category) / sizeof(category[0]);
int categoryIndex = 0;

int tagListIndex = 0;

// 忘れ物検知の変数
int jsonElementCount = 0;      // JSONファイルの要素数
int jsonElementExists = 0;     // かばん内に存在しているかどうか
int jsonNotElementExists = 0;  // かばん内に存在していないかどうか

// ダイヤルポジション変数
long oldPosition = -999;
long newPosition = 0;

// かばんの中にものが存在するかどうか
bool isExistTag = false;
// LEDのON/OFFフラグ
bool isLedOn = true;
// ブザーのON/OFFフラグ
bool isBuzzerOn = true;

// メニュー画面の画像配置
String existTagImage[3] = {PATH_MENU_GREEN_BELL, PATH_MENU_GREEN_SETTING,
                           PATH_MENU_GREEN_ADD};
String notExistTagImage[3] = {PATH_MENU_RED_BELL, PATH_MENU_RED_SETTING,
                              PATH_MENU_RED_ADD};
int tagImageLength = sizeof(existTagImage) / sizeof(existTagImage[0]);
int tagImageIndex = 0;

// 設定画面の画像配置
String offOffImage[3] = {PATH_SETTING_TOP_OFF_OFF, PATH_SETTING_RIGHT_OFF_OFF,
                         PATH_SETTING_LEFT_OFF_OFF};
String onOffImage[3] = {PATH_SETTING_TOP_ON_OFF, PATH_SETTING_RIGHT_ON_OFF,
                        PATH_SETTING_LEFT_ON_OFF};
String offOnImage[3] = {PATH_SETTING_TOP_OFF_ON, PATH_SETTING_RIGHT_OFF_ON,
                        PATH_SETTING_LEFT_OFF_ON};
String onOnImage[3] = {PATH_SETTING_TOP_ON_ON, PATH_SETTING_RIGHT_ON_ON,
                       PATH_SETTING_LEFT_ON_ON};
int settingImageLength = sizeof(offOffImage) / sizeof(offOffImage[0]);
int settingImageIndex = 0;

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
// ひとつ前の選択肢
Loops oldLoops = SETTING;

// 各画面のループ関数
void loop_menu();
void loop_setting();
void loop_addTag();
void loop_tagList();

// 各関数
void changeOnOffImage();
void showList(int index);
void showTagList(String tagList[], int tagListLength, int index);
int changeImageIndex(int index, int length, int direction);

// 忘れ物時にLEDを点灯させるタスク
void ledTask(void *parameter) {
    while (true) {
        if (isLedOn) {
            pixels.clear();
            if (!isExistTag) {
                for (int j = 0; j < NUM_PIXELS; j++) {
                    if (isExistTag) {
                        break;
                    }
                    pixels.setPixelColor(
                        j, pixels.Color(255, 0, 0));  // LEDの色を設定
                    pixels.show();                    // LEDに色を反映
                    delay(50 - j);                    // 500ms待機
                }
            } else {
                pixels.show();
            }
            for (int i = 0; i < 100; i++) {
                // if (isBuzzerOn) {
                //     // 8000Hzのトーンを20ミリ秒間鳴らします
                //     M5Dial.Speaker.tone(8000, 50);
                // }
                if (isExistTag) {
                    break;
                }
                delay(10);
            }
        } else {
            pixels.clear();
            pixels.show();
            delay(1000);
        }
    }
}

// 忘れ物時点灯タスクの開始関数
void startLedTask() {
    if (tagExistTaskHandle == NULL) {
        pixels.begin();
        xTaskCreate(ledTask, "ledTask", 5000, NULL, 1, &ledTaskHandle);
    }
}

// 忘れ物時点灯タスクの停止関数
void stopLedTask() {
    if (ledTaskHandle != NULL) {
        vTaskDelete(ledTaskHandle);
        ledTaskHandle = NULL;
    }
}

// 物がかばん内に存在するかどうかを確認するタスク
void tagExistTask(void *parameter) {
    while (true) {
        static int count = 0;
        count++;
        // 取得したタグIDを元に、タグIDが登録されているか確認
        // 登録していた場合
        String tagId = rfidUart.getExistTagId();
        if (tagJson.isTagIdExists(tagId.c_str())) {
            jsonElementCount = tagJson.getJsonElementCount();
            for (int i = 0; i < jsonElementCount; i++) {
                if (tagId == tagJson.getTagIdAtIndex(i)) {
                    if ((jsonElementExists & (1 << i)) == 0) {
                        jsonElementExists += 1 << i;
                    }
                    break;
                }
            }

            if (jsonElementExists == (1 << jsonElementCount) - 1) {
                count = 0;
                jsonElementCount = tagJson.getJsonElementCount();
                jsonElementExists = 0;
                isExistTag = true;
                if (isLedOn)
                    startLedTask();
                // rfidUart.clearExistTagId();
            }
        }
        // 一定間隔でタグIDをクリア
        if (count >= READING_CLEAR_INTERVAL) {
            // 存在しないタグの要素を取得
            jsonNotElementExists =
                jsonElementExists ^ ((1 << jsonElementCount) - 1);
            count = 0;
            jsonElementCount = tagJson.getJsonElementCount();
            jsonElementExists = 0;
            isExistTag = false;
            // rfidUart.clearExistTagId();
        }
        delay(READING_INTERVAL);
    }
}

// かばん内存在確認タスクの開始関数
void startTagExistTask() {
    if (tagExistTaskHandle == NULL) {
        jsonElementCount = tagJson.getJsonElementCount();
        jsonElementExists = 0;
        // rfidUart.clearExistTagId();
        delay(100);
        // rfidUart.startRFIDReader();
        xTaskCreate(tagExistTask, "tagExistTask", 5000, NULL, 1,
                    &tagExistTaskHandle);
    }
}

// かばん内存在確認タスクの停止関数
void stopTagExistTask() {
    if (tagExistTaskHandle != NULL) {
        // rfidUart.endRFIDReader();
        vTaskDelete(tagExistTaskHandle);
        tagExistTaskHandle = NULL;
    }
}

// line通知
void send_line() {
    // HTTPSへアクセス（SSL通信）するためのライブラリ
    WiFiClientSecure client;

    // サーバー証明書の検証を行わずに接続する場合に必要
    client.setInsecure();

    // LineのAPIサーバにSSL接続（ポート443:https）
    if (!client.connect(host, 443)) {
        return;
    }

    String message = "忘れ物があります\n忘れ物:\n";

    for (int i = 0; i < jsonElementCount; i++) {
        if ((jsonNotElementExists & (1 << i)) != 0) {
            message += tagJson.getNameAtIndex(i);
            if (jsonElementCount - 1 > i) {
                message += "\n";
            }
        }
    }

    if (message.endsWith("\n")) {
        message.remove(message.length() - 1);
    }

    // メッセージをURLエンコード
    message.replace("\n", "%0A");

    // パラメーターを設定
    String postData = "message=" + message;

    // HTTPリクエストを送信
    client.println("POST /api/notify HTTP/1.1");
    client.println("Host: notify-api.line.me");
    client.print("Authorization: Bearer ");
    client.println(token);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.println(postData);

    Serial.println("Request sent");

    // 応答を読み取る
    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            Serial.println("Headers received");
            break;
        }
    }
    String line = client.readStringUntil('\n');
}

void setup() {
    M5_BEGIN();
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);

    rfidUart.init();
    tagJson.init();

    // WiFi接続
    WiFi.begin(ssid, pass);

    // WiFiの接続状態を確認
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
    }

    M5Dial.Display.setTextColor(WHITE);
    M5Dial.Display.setTextDatum(middle_center);
    M5Dial.Display.setTextFont(&fonts::lgfxJapanGothic_36);
    M5Dial.Display.setTextSize(1);

    SPIFFS.begin();

    M5_UPDATE();

    startLedTask();
    delay(100);
    startTagExistTask();
    delay(100);
}

void loop() {
    M5Dial.update();
    // M5_UPDATE();

    // 状態によりループ関数を切り替える
    switch (currentLoops) {
        case MENU:
            if (currentLoops != oldLoops) {
                oldPosition = newPosition;
                if (isExistTag) {
                    M5.Lcd.drawJpgFile(SPIFFS, existTagImage[tagImageIndex], 0,
                                       0);
                } else {
                    M5.Lcd.drawJpgFile(SPIFFS, notExistTagImage[tagImageIndex],
                                       0, 0);
                }
                oldLoops = currentLoops;
            }
            loop_menu();
            break;
        case SETTING:
            if (currentLoops != oldLoops) {
                oldPosition = newPosition;
                changeOnOffImage();
                oldLoops = currentLoops;
            }
            loop_setting();
            break;
        case ADDTAG:
            if (currentLoops != oldLoops) {
                M5Dial.Display.fillScreen(0x4208);
                oldPosition = newPosition;
                showList(categoryIndex);
                oldLoops = currentLoops;
            }
            loop_addTag();
            break;
        case TAGLIST:
            if (currentLoops != oldLoops) {
                M5Dial.Display.fillScreen(0x4208);
                String notExistTags[jsonElementCount];
                for (int i = 0; i < jsonElementCount; i++) {
                    if ((jsonNotElementExists & (1 << i)) != 0) {
                        notExistTags[i] = tagJson.getNameAtIndex(i);
                    }
                }
                showTagList(notExistTags, jsonElementCount, tagListIndex);
                oldLoops = currentLoops;
            }
            loop_tagList();
            break;
    }

    // 遅延を入れないとダイヤルの挙動がおかしくなる
    delay(1);
}

// メニュー画面のループ関数
void loop_menu() {
    static bool isExistTagOld = isExistTag;
    newPosition = M5Dial.Encoder.read();

    // 忘れ物状態が変化したときの処理
    if (isExistTagOld != isExistTag) {
        M5Dial.Speaker.tone(8000, 20);
        if (isExistTag) {
            M5.Lcd.drawJpgFile(SPIFFS, existTagImage[tagImageIndex], 0, 0);
        } else {
            M5.Lcd.drawJpgFile(SPIFFS, notExistTagImage[tagImageIndex], 0, 0);
            send_line();
        }
        isExistTagOld = isExistTag;
    }

    // ダイヤルがひねられたときの処理
    if (newPosition != oldPosition) {
        M5Dial.Speaker.tone(8000, 20);
        tagImageIndex = changeImageIndex(tagImageIndex, tagImageLength,
                                         newPosition - oldPosition);
        oldPosition = newPosition;
        // 選択肢をもとに画像の切り替え
        if (isExistTag) {
            M5.Lcd.drawJpgFile(SPIFFS, existTagImage[tagImageIndex], 0, 0);
        } else {
            M5.Lcd.drawJpgFile(SPIFFS, notExistTagImage[tagImageIndex], 0, 0);
        }
    }

    // ボタンが押されたときの処理
    if (M5Dial.BtnA.wasPressed()) {
        M5Dial.Speaker.tone(8000, 20);
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

// 設定画面のループ関数
void loop_setting() {
    newPosition = M5Dial.Encoder.read();

    // ダイヤルがひねられたときの処理
    if (newPosition != oldPosition) {
        M5Dial.Speaker.tone(8000, 20);
        settingImageIndex = changeImageIndex(
            settingImageIndex, settingImageLength, newPosition - oldPosition);
        oldPosition = newPosition;
        // 選択肢とブザー、LEDのON/OFFもとに画像の切り替え
        changeOnOffImage();
    }

    // ボタンが押されたときの処理
    if (M5Dial.BtnA.wasPressed()) {
        switch (settingImageIndex) {
            case 0:
                currentLoops = MENU;
                break;
            case 1:
                isBuzzerOn = !isBuzzerOn;
                changeOnOffImage();
                break;
            case 2:
                isLedOn = !isLedOn;
                changeOnOffImage();
                break;
        }
    }
}

// タグ登録画面のループ関数
void loop_addTag() {
    newPosition = M5Dial.Encoder.read();

    // ダイヤルがひねられたときの処理
    if (newPosition != oldPosition) {
        M5Dial.Speaker.tone(8000, 20);
        categoryIndex = changeImageIndex(categoryIndex, categoryLength,
                                         newPosition - oldPosition);
        M5Dial.Display.fillScreen(0x4208);
        showList(categoryIndex);
        oldPosition = newPosition;
    }

    // ボタンが押されたときの処理
    if (M5Dial.BtnA.wasPressed()) {
        int count = 0;
        M5Dial.Display.setTextColor(60388);

        // RFIDリーダーのタグ情報をクリア
        // rfidUart.clearExistTagId();

        delay(100);

        // RFIDリーダーの再起動
        // rfidUart.endRFIDReader();
        delay(100);
        // rfidUart.startRFIDReader();

        while (true) {
            count++;
            M5Dial.Display.fillScreen(0x4208);
            if (count % 3 == 0) {
                M5Dial.Display.drawString("捜索中.", M5Dial.Display.width() / 2,
                                          M5Dial.Display.height() / 2);
            } else if (count % 3 == 1) {
                M5Dial.Display.drawString("捜索中..",
                                          M5Dial.Display.width() / 2,
                                          M5Dial.Display.height() / 2);
            } else if (count % 3 == 2) {
                M5Dial.Display.drawString("捜索中...",
                                          M5Dial.Display.width() / 2,
                                          M5Dial.Display.height() / 2);
            }
            if (count >= 10) {
                M5Dial.Display.fillScreen(0x4208);
                M5Dial.Display.setTextSize(0.6);
                M5Dial.Display.drawString("時間内にタグが",
                                          M5Dial.Display.width() / 2,
                                          M5Dial.Display.height() / 2 - 14);
                M5Dial.Display.drawString("見つかりませんでした",
                                          M5Dial.Display.width() / 2,
                                          M5Dial.Display.height() / 2 + 14);
                M5Dial.Display.setTextSize(1);
                delay(2000);
                break;
            }
            String tagId = rfidUart.getExistTagId();
            // タグIDが取得できた場合
            if (tagId != "") {
                // すでに登録されているタグIDの場合
                if (tagJson.isTagIdExists(tagId.c_str())) {
                    M5Dial.Display.setTextSize(0.6);
                    M5Dial.Display.fillScreen(0x4208);
                    M5Dial.Display.drawString("そのタグは",
                                              M5Dial.Display.width() / 2,
                                              M5Dial.Display.height() / 2 - 20);
                    M5Dial.Display.drawString("すでに登録されています.",
                                              M5Dial.Display.width() / 2,
                                              M5Dial.Display.height() / 2) +
                        20;
                    M5Dial.Display.setTextSize(1);
                    delay(2000);
                    break;
                }
                // 未登録タグIDの場合
                else {
                    // タグIDを登録
                    if (tagJson.addTagFromJson(category[categoryIndex].c_str(),
                                               tagId.c_str())) {
                        M5Dial.Display.setTextSize(0.6);
                        M5Dial.Display.fillScreen(0x4208);
                        M5Dial.Display.drawString(
                            category[categoryIndex], M5Dial.Display.width() / 2,
                            M5Dial.Display.height() / 2 - 20);
                        M5Dial.Display.drawString("を登録しました。",
                                                  M5Dial.Display.width() / 2,
                                                  M5Dial.Display.height() / 2) +
                            20;
                        M5Dial.Display.setTextSize(1);
                        delay(2000);
                        break;
                    }
                    // タグIDの登録に失敗した場合
                    else {
                        M5Dial.Display.setTextSize(0.6);
                        M5Dial.Display.fillScreen(0x4208);
                        M5Dial.Display.drawString("登録に失敗しました.",
                                                  M5Dial.Display.width() / 2,
                                                  M5Dial.Display.height() / 2);
                        M5Dial.Display.setTextSize(1);
                        delay(2000);
                        break;
                    }
                }
                delay(1000);
                // rfidUart.refreshRFIDReader();
            }
            delay(500);
        }
        currentLoops = MENU;
    }
}

// 設定画面の画像を切り替える関数
void changeOnOffImage() {
    if (isLedOn) {
        if (isBuzzerOn) {
            M5.Lcd.drawJpgFile(SPIFFS, onOnImage[settingImageIndex], 0, 0);
        } else {
            M5.Lcd.drawJpgFile(SPIFFS, onOffImage[settingImageIndex], 0, 0);
        }
    } else {
        if (isBuzzerOn) {
            M5.Lcd.drawJpgFile(SPIFFS, offOnImage[settingImageIndex], 0, 0);
        } else {
            M5.Lcd.drawJpgFile(SPIFFS, offOffImage[settingImageIndex], 0, 0);
        }
    }
}

// タグリスト画面のループ関数
void loop_tagList() {
    newPosition = M5Dial.Encoder.read();

    // ダイヤルがひねられたときの処理
    if (newPosition != oldPosition) {
        M5Dial.Speaker.tone(8000, 20);
        int tagListLength = tagJson.getJsonElementCount();
        tagListIndex = changeImageIndex(tagListIndex, tagListLength,
                                        newPosition - oldPosition);
        M5Dial.Display.fillScreen(0x4208);
        String notExistTags[jsonElementCount];
        for (int i = 0; i < jsonElementCount; i++) {
            if ((jsonNotElementExists & (1 << i)) != 0) {
                notExistTags[i] = tagJson.getNameAtIndex(i);
            }
        }
        showTagList(notExistTags, jsonElementCount, tagListIndex);
        oldPosition = newPosition;
    }

    // ボタンが押されたときの処理
    if (M5Dial.BtnA.wasPressed()) {
        currentLoops = MENU;
    }
}

// リストを表示する関数
void showList(int index) {
    for (int i = 0; i < categoryLength; i++) {
        int x = M5Dial.Display.width() / 2;
        int y = ((M5Dial.Display.height() / 2) + HEIGHT_INTERVAL * i) -
                index * HEIGHT_INTERVAL;
        if (i == index) {
            M5Dial.Display.setTextColor(60388);
        } else {
            M5Dial.Display.setTextColor(WHITE);
        }
        M5Dial.Display.drawString(category[i], x, y);
    }
}

// タグリストを表示する関数
void showTagList(String tagList[], int tagListLength, int index) {
    for (int i = 0; i < tagListLength; i++) {
        int x = M5Dial.Display.width() / 2;
        int y = ((M5Dial.Display.height() / 2) + HEIGHT_INTERVAL * i) -
                index * HEIGHT_INTERVAL;
        if (i == index) {
            M5Dial.Display.setTextColor(60388);
        } else {
            M5Dial.Display.setTextColor(WHITE);
        }
        M5Dial.Display.drawString(tagList[i], x, y);
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