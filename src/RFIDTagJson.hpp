#pragma once

#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>

// 保存するJSONファイル名
#define JSON_FILE "/tags.json"

class RFIDTagJson {
public:
    RFIDTagJson();
    bool isTagIdExists(const char* tagID);
    String getNameFromTagId(const char* tagID);
    bool addTagFromJson(const char* name, const char* tagID);
    bool deleteTagFromJson(const char* tagID);
    String[] listTag();
};
