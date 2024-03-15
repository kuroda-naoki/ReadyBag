#pragma once

#include <Arduino.h>

// RFIDリーダーのシリアル通信設定
#define RFID_UART_BAUDRATE 115200
#define RFID_UART_RX       2
#define RFID_UART_TX       1

// RFIDリーダーの動作コマンド
#define RFID_READER_START_COMMAND "START"
#define RFID_READER_END_COMMAND   "END"
#define RFID_READER_CLEAR_COMMAND "CLEAR"

class RFIDUart {
public:
    RFIDUart();
    void init();
    void startRFIDReader();
    String getExistTagId();
    void refreshRFIDReader();
    void endRFIDReader();
    void clearExistTagId();
};

// ----------------------------------------------------------------------------------------------------------------------------------
// ==================================================================================================================================
// 処理の実装部分
// ==================================================================================================================================
// ----------------------------------------------------------------------------------------------------------------------------------

RFIDUart::RFIDUart() {
}

// シリアル通信の初期化
void RFIDUart::init() {
    Serial2.begin(RFID_UART_BAUDRATE, SERIAL_8N1, RFID_UART_RX, RFID_UART_TX);
}

// RFIDリーダーの起動
void RFIDUart::startRFIDReader() {
    // RFIDリーダーの起動コマンドを送信
    Serial2.println(RFID_READER_START_COMMAND);
}

// RFIDリーダーから存在するタグIDの取得
String RFIDUart::getExistTagId() {
    String tagId = "";
    while (Serial2.available()) {
        char receivedChar = Serial2.read();
        if (receivedChar == '\n') {
            return tagId;
        } else {
            tagId += receivedChar;
        }
    }
    return tagId;
}

// RFIDリーダーのリフレッシュ
void RFIDUart::refreshRFIDReader() {
    // RFIDリーダーのリフレッシュコマンドを送信
    while (Serial2.available()) {
        Serial2.read();
        delay(1);
    }
}

// RFIDリーダーの終了
void RFIDUart::endRFIDReader() {
    // RFIDリーダーの終了コマンドを送信
    Serial2.println(RFID_READER_END_COMMAND);
}

// RFIDリーダーが保有しているタグIDのクリア
void RFIDUart::clearExistTagId() {
    Serial2.println(RFID_READER_CLEAR_COMMAND);
}