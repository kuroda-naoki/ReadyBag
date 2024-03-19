#pragma once

// メニュー画面の画像ファイルパス
#define PATH_MENU_GREEN_ADD     "/home_green_1.jpg"
#define PATH_MENU_GREEN_BELL    "/home_green_2.jpg"
#define PATH_MENU_GREEN_SETTING "/home_green_3.jpg"
#define PATH_MENU_RED_ADD       "/home_red_1.jpg"
#define PATH_MENU_RED_BELL      "/home_red_2.jpg"
#define PATH_MENU_RED_SETTING   "/home_red_3.jpg"

// 設定画面の画像ファイルパス
// 選択中の項目、LEDのON/OFF、ブザーのON/OFF
#define PATH_SETTING_LEFT_OFF_OFF  "/setting_left_off_off.jpg"
#define PATH_SETTING_LEFT_ON_OFF   "/setting_left_on_off.jpg"
#define PATH_SETTING_LEFT_OFF_ON   "/setting_left_off_on.jpg"
#define PATH_SETTING_LEFT_ON_ON    "/setting_left_on_on.jpg"
#define PATH_SETTING_TOP_OFF_OFF   "/setting_top_off_off.jpg"
#define PATH_SETTING_TOP_ON_OFF    "/setting_top_on_off.jpg"
#define PATH_SETTING_TOP_OFF_ON    "/setting_top_off_on.jpg"
#define PATH_SETTING_TOP_ON_ON     "/setting_top_on_on.jpg"
#define PATH_SETTING_RIGHT_OFF_OFF "/setting_right_off_off.jpg"
#define PATH_SETTING_RIGHT_ON_OFF  "/setting_right_on_off.jpg"
#define PATH_SETTING_RIGHT_OFF_ON  "/setting_right_off_on.jpg"
#define PATH_SETTING_RIGHT_ON_ON   "/setting_right_on_on.jpg"

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