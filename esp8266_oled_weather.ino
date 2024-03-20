#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "SSD1306Wire.h"  // OLED屏幕驱动
#include "OLEDDisplayUi.h"
#include <time.h>  // 时间库
#include <sys/time.h>
#include <coredecls.h>
#include "images.h"  // 个人库
#include "Weather.h"

// 和风天气部分
String UserKey = "66bd71a972df49479d52256685074dc1";  // 私钥
String Location = "101270103";                        // 城市代码
String Unit = "m";                                    // 公制-m/英制-i
String Lang = "en";                                   // 语言 英文-en/中文-zh

// 实例化
Weather weather;
WeatherNowData NowData;
WeatherForecastData ForecastData[3];
time_t Now;

SSD1306Wire display(0x3c, D2, D1);  // SDA, SCL
OLEDDisplayUi ui(&display);

// 时间相关
const String WDAY_NAMES[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };  //星期

// OLED相关
int screen_W = 128;
int screen_H = 64;
int CenterX = screen_W / 2;
int CenterY = ((screen_H - 16) / 2);  // top yellow part is 16 px height

bool firstUp = true;
bool readyWeatherNowUpdate = false;             // 实况天气更新状态
bool readyWeatherForecastUpdate = false;        // 天气预报更新状态
long LastWeatherNowUpdate = 0;                  // 上次实况天气更新时间
long LastWeatherForecastUpdate = 0;             // 上次天气预报更新时间
const int WeatherNowUpdateSpan = 10 * 60;       // 实况天气更新时间间隔
const int WeatherForecastUpdateSpan = 30 * 60;  // 天气预报更新时间间隔
// WiFi信息
const char *ssid = "shuangsheng";
const char *password = "chenyuanxi";

// 提前声明函数
void drawProgress(OLEDDisplay *display, int percentage, String label);
void firstUpdate(OLEDDisplay *display);
void drawTimeFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void drawWeatherNow(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void drawWeatherForecast(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void drawWeatherForecastDetails(OLEDDisplay *display, int x, int y, int index);
void drawOverlay(OLEDDisplay *display, OLEDDisplayUiState *state);

// 连接WiFi
void connectWifi() {
  display.clear();
  display.drawXbm(0, 3, 128, 64, WiFi_Logo);
  display.display();
  WiFi.begin(ssid, password);

  Serial.println("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}

// 绘制进度条
void drawProgress(OLEDDisplay *display, int percentage, String label) {
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 16, label);
  display->drawProgressBar(2, 30, 124, 10, percentage);
  display->display();
}

// 第一次更新
void firstUpdate(OLEDDisplay *display) {
  drawProgress(display, 0, "Updating Clock...");
  configTime(8 * 3600, 0, "pool.ntp.org", "ntp1.aliyun.com");
  // 等待时间同步成功
  while (!time(nullptr)) {
    delay(1000);  // 每隔1秒检查一次
  }

  drawProgress(display, 33, "Updating Weather...");
  weather.config(UserKey, Location, Unit, Lang);  // 配置请求信息
  weather.UpDateWeatherNow(&NowData);
  weather.UpDateAirQuality(&NowData);

  drawProgress(display, 66, "Updating Forecast...");
  weather.config(UserKey, Location, Unit, Lang);  // 配置请求信息
  weather.UpDateWeatherForecast(ForecastData);

  drawProgress(display, 100, "Updating Done...");
  delay(1000);
}

// 时钟——1
void drawTimeFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y) {
  Now = time(nullptr);
  struct tm *timeInfo;
  timeInfo = localtime(&Now);
  char buff[6];
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);

  sprintf_P(buff, PSTR("%02d:%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
  display->drawString(CenterX + x, CenterY + y, String(buff));
}

// 实时天气及空气质量——2
void drawWeatherNow(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(62 + x, 42 + y, NowData.FeelLike + "°C | " + NowData.Humidity + "%");

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);
  display->drawString(92 + x, 18 + y, NowData.Temp + "°C");

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64 + x, 54 + y, "aqi: " + NowData.Aqi + " | " + "rain: " + NowData.Precip + " mm");

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(Meteocons_Regular_36);
  display->drawString(32 + x, 16 + y, NowData.Icon);
}

// 天气预报——3
void drawWeatherForecast(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y) {
  drawWeatherForecastDetails(display, x - 44, y, 0);
  drawWeatherForecastDetails(display, x, y, 1);
  drawWeatherForecastDetails(display, x + 44, y, 2);
}

// 天气预报细节
void drawWeatherForecastDetails(OLEDDisplay *display, int x, int y, int index) {
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64 + x, 54 + y, ForecastData[index].TempMin + " ~ " + ForecastData[index].TempMax);
  display->drawString(64 + x, 17 + y, ForecastData[index].FxDate.substring(5, 10));

  display->setFont(Meteocons_Regular_21);
  display->drawString(64 + x, 30 + y, ForecastData[index].IconDay);
}

// 页眉 时间/星期/年月日
void drawOverlay(OLEDDisplay *display, OLEDDisplayUiState *state) {
  Now = time(nullptr);
  struct tm *timeInfo;
  timeInfo = localtime(&Now);
  char buff[8];
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);

  // 左上角时间
  sprintf_P(buff, PSTR("%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min);
  display->drawString(6, 1, String(buff));
  // 右上角星期
  sprintf_P(buff, PSTR("%s"), WDAY_NAMES[timeInfo->tm_wday].c_str());
  display->drawString(100, 1, String(buff));
  // 中间年月日
  sprintf_P(buff, PSTR("%04d-%02d-%02d"), timeInfo->tm_year + 1900,
            timeInfo->tm_mon + 1, timeInfo->tm_mday);
  display->drawString(37, 6, String(buff));
}

// 添加框架,此数组保留指向所有帧的函数指针,框架是从右向左滑动的单个视图
FrameCallback frames[] = { drawTimeFrame, drawWeatherNow, drawWeatherForecast };
int frameCount = 3;

// 覆盖图静态绘制在框架顶部,页眉
OverlayCallback overlays[] = { drawOverlay };
int overlaysCount = 1;

void setup() {
  Serial.begin(115200);
  ui.init();
  display.clear();
  display.display();

  display.flipScreenVertically();  //屏幕翻转
  display.setContrast(255);        //屏幕亮度

  // webconnect();  // web配网
  connectWifi();

  ui.setTargetFPS(60);                      // 刷新频率
  ui.setActiveSymbol(activeSymbol);         // 设置活动符号
  ui.setInactiveSymbol(inactiveSymbol);     // 设置非活动符号
  ui.setIndicatorPosition(TOP);             // 符号位置 TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorDirection(LEFT_RIGHT);     // 定义第一框架在栏中的位置
  ui.setFrameAnimation(SLIDE_LEFT);         // 屏幕切换方向 SLIDE_LEFT/RIGHT/UP/DOWN
  ui.setFrames(frames, frameCount);         // 设置框架
  ui.setTimePerFrame(5000);                 // 设置切换时间
  ui.setOverlays(overlays, overlaysCount);  // 设置覆盖

  ui.init();                       // UI负责初始化显示
  display.flipScreenVertically();  // 屏幕反转
}

void loop() {
  if (firstUp) {  // 首次更新
    firstUpdate(&display);
    firstUp = false;
  }
  // Serial.println(ui.getUiState()->frameState == FIXED);
  if (millis() - LastWeatherNowUpdate > (1000L * WeatherNowUpdateSpan)) {  // 实况天气刷新
    readyWeatherNowUpdate = true;
    LastWeatherNowUpdate = millis();
    Serial.println();
  }

  if (millis() - LastWeatherForecastUpdate > (1000L * WeatherForecastUpdateSpan)) {  // 天气预报刷新
    readyWeatherForecastUpdate = true;
    LastWeatherForecastUpdate = millis();
  }

  if (readyWeatherNowUpdate && ui.getUiState()->frameState == FIXED) {  // 实况天气更新
    weather.config(UserKey, Location, Unit, Lang);
    weather.UpDateWeatherNow(&NowData);
    readyWeatherNowUpdate = false;
  }

  if (readyWeatherForecastUpdate && ui.getUiState()->frameState == FIXED) {  // 天气预报更新
    weather.config(UserKey, Location, Unit, Lang);
    weather.UpDateWeatherForecast(ForecastData);
    weather.UpDateAirQuality(&NowData);
    readyWeatherForecastUpdate = false;
  }

  int remainingTimeBudget = ui.update();  // 剩余时间预算

  if (remainingTimeBudget > 0) {

    delay(remainingTimeBudget);
  }
}