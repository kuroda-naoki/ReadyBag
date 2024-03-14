#pragma once

#include <Arduino.h>

// RFIDリーダーのシリアル通信設定
#define RFID_UART_BAUDRATE 115200
#define RFID_UART_RX       2
#define RFID_UART_TX       1

// RFIDリーダーの動作コマンド
#define RFID_READER_START_COMMAND "START"

class RFIDUart {
public:
    RFIDUart();
    void init();
    void startRFIDReader();
    String getExistTagId();
    void update();
    void send(const char* data);
    String receive();
    bool isAvailable();
    void clear();
    void end();
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