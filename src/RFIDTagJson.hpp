#pragma once

#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>

// 保存するJSONファイル名
#define JSON_FILE "/tags.json"

class RFIDTagJson {
public:
    RFIDTagJson();
    void init();
    bool isTagIdExists(const char* tagID);
    String getNameFromTagId(const char* tagID);
    bool addTagFromJson(const char* name, const char* tagID);
    bool deleteTagFromJson(const char* tagID);
    int getJsonElementCount();
    String getNameAtIndex(int index);
};

// ----------------------------------------------------------------------------------------------------------------------------------
// ==================================================================================================================================
// 処理の実装部分
// ==================================================================================================================================
// ----------------------------------------------------------------------------------------------------------------------------------

RFIDTagJson::RFIDTagJson() {
}

// SPIFFS初期化関数
void RFIDTagJson::init() {
    SPIFFS.begin();
}

// タグIDが存在するか確認する関数
bool RFIDTagJson::isTagIdExists(const char* tagID) {
    // jsonファイルが存在しない場合、タグIDは存在しないと判断します。
    if (!SPIFFS.exists(JSON_FILE)) {
        return false;
    }

    // jsonファイルを開きます。
    File file = SPIFFS.open(JSON_FILE, FILE_READ);
    // ファイル読み取り失敗時
    if (!file) {
        return false;
    }

    // ファイルの内容をJSONオブジェクトにデシリアライズします。
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        file.close();
        return false;
    }
    file.close();

    // JSONオブジェクト内を検索し、タグIDが存在するかどうかを確認します。
    for (JsonPair kv : doc.as<JsonObject>()) {
        if (strcmp(kv.value().as<const char*>(), tagID) == 0) {
            // 指定されたタグIDが見つかった場合
            return true;
        }
    }

    // 指定されたタグIDが見つからなかった場合
    return false;
}

// タグIDから名前を取得する関数
String RFIDTagJson::getNameFromTagId(const char* tagID) {
    // jsonファイルが存在しない場合、名前は見つかりません。
    if (!SPIFFS.exists(JSON_FILE)) {
        return "";
    }

    // jsonファイルを開きます。
    File file = SPIFFS.open(JSON_FILE, FILE_READ);
    // ファイル読み取り失敗時
    if (!file) {
        return "";
    }

    // ファイルの内容をJSONオブジェクトにデシリアライズします。
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, file);
    // ファイル変換失敗時
    if (error) {
        file.close();
        return "";
    }
    file.close();

    // JSONオブジェクト内を検索し、タグIDに対応する名前を見つけます。
    for (JsonPair kv : doc.as<JsonObject>()) {
        if (strcmp(kv.value().as<const char*>(), tagID) == 0) {
            // 指定されたタグIDに対応する名前が見つかった場合、名前を返します。
            return String(kv.key().c_str());
        }
    }

    // 指定されたタグIDに対応する名前が見つからなかった場合、空文字列を返します。
    return "";
}

// タグ情報を追加する関数
bool RFIDTagJson::addTagFromJson(const char* name, const char* tagID) {
    // Jsonファイルが存在しない場合は何もしない
    if (!SPIFFS.exists(JSON_FILE)) {
        // return false;
    }

    File file = SPIFFS.open(JSON_FILE, FILE_READ);
    // ファイル読み取り失敗時
    if (!file) {
        // return false;
    }

    StaticJsonDocument<1024> doc;  // JSONドキュメントを作成します。
    DeserializationError error = deserializeJson(doc, file);
    // ファイル変換失敗時
    if (error) {
        file.close();
        // return false;
    }
    file.close();  // ファイル読み込み完了後は閉じる

    // jsonファイルが存在する場合、その内容を読み込みます。
    if (SPIFFS.exists(JSON_FILE)) {
        File file = SPIFFS.open(JSON_FILE, FILE_READ);
        // ファイル読み取り失敗時
        if (!file) {
            return false;
        }
        error = deserializeJson(doc, file);
        // ファイル変換失敗時
        if (error) {
        }
        file.close();
    }

    // 新しいタグ情報を追加します。
    doc[name] = tagID;

    // 変更をファイルに書き戻す
    file = SPIFFS.open(JSON_FILE, FILE_WRITE);
    // ファイル書き込み失敗時
    if (!file) {
        return false;
    }

    // ファイル変換失敗時
    if (serializeJson(doc, file) == 0) {
        file.close();
        return false;
    }
    file.close();
    return true;
}

// タグ情報を削除する関数
bool RFIDTagJson::deleteTagFromJson(const char* tagID) {
    // Jsonファイルが存在しない場合は何もしない
    if (!SPIFFS.exists(JSON_FILE)) {
        return false;
    }

    File file = SPIFFS.open(JSON_FILE, FILE_READ);
    // ファイル読み取り失敗時
    if (!file) {
        return false;
    }

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, file);
    // ファイル変換失敗時
    if (error) {
        file.close();
        return false;
    }
    file.close();  // ファイル読み込み完了後は閉じる

    bool found = false;
    // JSONオブジェクトを反復処理し、指定されたタグIDを探す
    for (JsonPair kv : doc.as<JsonObject>()) {
        if (strcmp(kv.value().as<const char*>(), tagID) == 0) {
            // エントリを削除
            doc.remove(kv.key());
            found = true;
            break;  // マッチする最初のエントリを削除したらループを抜ける
        }
    }

    // タグIDが見つからなかった場合はfalseを返す
    if (!found) {
        return false;
    }

    // 変更をファイルに書き戻す
    file = SPIFFS.open(JSON_FILE, FILE_WRITE);
    // ファイル書き込み失敗時
    if (!file) {
        return false;
    }

    // ファイル変換失敗時
    if (serializeJson(doc, file) == 0) {
        file.close();
        return false;
    }

    file.close();  // ファイル操作完了後は閉じる
    return true;   // 成功した場合はtrueを返す
}

// JSONファイル内の要素数を取得する関数
int RFIDTagJson::getJsonElementCount() {
    if (!SPIFFS.exists(JSON_FILE)) {
        return -1;
    }

    File file = SPIFFS.open(JSON_FILE, FILE_READ);
    if (!file) {
        return -1;
    }

    StaticJsonDocument<1024> doc;  // Adjust size according to your needs
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        file.close();
        return -1;
    }
    file.close();

    // Use size() method to get the number of elements in the JSON object
    int elementCount = doc.size();

    return elementCount;
}

// JSONファイル内の指定されたインデックスの名前を取得する関数
String RFIDTagJson::getNameAtIndex(int index) {
    if (!SPIFFS.exists(JSON_FILE)) {
        return "";
    }

    File file = SPIFFS.open(JSON_FILE, FILE_READ);
    if (!file) {
        return "";
    }

    StaticJsonDocument<1024> doc;  // Adjust size according to your needs
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        file.close();
        return "";
    }
    file.close();

    int currentIndex = 0;
    for (JsonPair kv : doc.as<JsonObject>()) {
        if (currentIndex == index) {
            return String(
                kv.key().c_str());  // Return the name at the specified index
        }
        currentIndex++;
    }

    return "";  // Return an empty string if the index is out of range
}