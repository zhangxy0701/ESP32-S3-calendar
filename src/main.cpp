/*******************************************************************************
 * Start of Arduino_GFX setting
 *
 * Arduino_GFX try to find the settings depends on selected board in Arduino IDE
 * Or you can define the display dev kit not in the board list
 * Defalult pin list for non display dev kit:
 * Arduino Nano, Micro and more: CS:  9, DC:  8, RST:  7, BL:  6, SCK: 13, MOSI: 11, MISO: 12
 * ESP32 various dev board     : CS:  5, DC: 27, RST: 33, BL: 22, SCK: 18, MOSI: 23, MISO: nil
 * ESP32-C3 various dev board  : CS:  7, DC:  2, RST:  1, BL:  3, SCK:  4, MOSI:  6, MISO: nil
 * ESP32-S2 various dev board  : CS: 34, DC: 38, RST: 33, BL: 21, SCK: 36, MOSI: 35, MISO: nil
 * ESP32-S3 various dev board  : CS: 40, DC: 41, RST: 42, BL: 48, SCK: 36, MOSI: 35, MISO: nil
 * ESP8266 various dev board   : CS: 15, DC:  4, RST:  2, BL:  5, SCK: 14, MOSI: 13, MISO: 12
 * Raspberry Pi Pico dev board : CS: 17, DC: 27, RST: 26, BL: 28, SCK: 18, MOSI: 19, MISO: 16
 * RTL8720 BW16 old patch core : CS: 18, DC: 17, RST:  2, BL: 23, SCK: 19, MOSI: 21, MISO: 20
 * RTL8720_BW16 Official core  : CS:  9, DC:  8, RST:  6, BL:  3, SCK: 10, MOSI: 12, MISO: 11
 * RTL8722 dev board           : CS: 18, DC: 17, RST: 22, BL: 23, SCK: 13, MOSI: 11, MISO: 12
 * RTL8722_mini dev board      : CS: 12, DC: 14, RST: 15, BL: 13, SCK: 11, MOSI:  9, MISO: 10
 * Seeeduino XIAO dev board    : CS:  3, DC:  2, RST:  1, BL:  0, SCK:  8, MOSI: 10, MISO:  9
 * Teensy 4.1 dev board        : CS: 39, DC: 41, RST: 40, BL: 22, SCK: 13, MOSI: 11, MISO: 12
 ******************************************************************************/

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "../lib/lvgl/lvgl.h"
#include <demos/lv_demos.h>

#include "../include/ui_h/ui.h"
#include "../include/ui_h/ui_helpers.h"
#include <WiFi.h>
#include <NTP.h>
 #include <lvgl.h>
#include <esp32-hal-log.h>
#include <esp_log.h>

#include <HTTPClient.h>

#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_Sensor.h>
#include  "../lib/lvgl/src/extra/widgets/calendar/lv_calendar.h"
#include  "../lib/lvgl/src/font/lv_font_fmt_txt.h"
#include  "../lib/lvgl/src/extra/widgets/calendar/lv_calendar_header_dropdown.h"
#include <OneButton.h>







#define DHTPIN 47    // Digital pin connected to the DHT sensor 
#define DHTTYPE DHT11      // DHT 11


DHT dht(DHTPIN, DHTTYPE);

OneButton button21(21, true);

int updateTimes = 60;    // 实时天气更新时间，单位:分钟
int uptateOthers = 3600; // 未来天气和黄历的更新时间，单位:分钟

String str_time_hour();
String str_time_minute();
String str_time_second();

bool attachDuringLongPressNumber;

char *num_week(uint8_t dayofweek,int Mode);//计算星期

void GetCitycode();

void lv_calendar_set_day_names(lv_obj_t * obj, const char ** day_names);
void lv_calendar_set_showed_date(lv_obj_t * obj, uint32_t year, uint32_t month);
void lv_calendar_set_today_date(lv_obj_t * obj, uint32_t year, uint32_t month, uint32_t day);
void lv_calendar_set_highlighted_dates(lv_obj_t * obj, lv_calendar_date_t highlighted[], uint16_t date_num);

void attachClick21();


#define GFX_BL DF_GFX_BL // default backlight pin, you may replace DF_GFX_BL to actual backlight pin

/* More dev device declaration: https://github.com/moononournation/Arduino_GFX/wiki/Dev-Device-Declaration */
#if defined(DISPLAY_DEV_KIT)
Arduino_GFX *gfx = create_default_Arduino_GFX();
#else /* !defined(DISPLAY_DEV_KIT) */

Arduino_DataBus *bus = new Arduino_ESP32LCD16(
   16, 17, 18 /* WR */, 19 /* RD */,
    0 /* D0 */, 1 /* D1 */, 2 /* D2 */, 3 /* D3 */, 4 /* D4 */, 5 /* D5 */, 6 /* D6 */, 7 /* D7 */,
    8 /* D8 */, 9 /* D9 */, 10 /* D10 */, 11 /* D11 */, 12 /* D12 */, 13 /* D13 */, 14 /* D14 */, 15 /* D15 */);


Arduino_GFX *gfx = new Arduino_NT35510(
  bus, 20/* RST */, 1 /* rotation */);
  

#endif /* !defined(DISPLAY_DEV_KIT) */
/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/


/* Change to your screen resolution */
static uint32_t screenWidth;
static uint32_t screenHeight;
static uint32_t bufSize;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_draw_buf;
static lv_disp_drv_t disp_drv;

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
#ifndef DIRECT_MODE
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif
#endif // #ifndef DIRECT_MODE

  lv_disp_flush_ready(disp);
}

//lv_obj_t *label_other;

//int k;

void setup(void)
{
  Serial.begin(115200);

 dht.begin();


  //Serial.setDebugOutput(true);
  //while(!Serial);
  Serial.println("Arduino_GFX Hello World example");


 // 开始连接WIFI

    Serial.println();
    Serial.println();
    Serial.print("WIFI Mode: ");
    Serial.println(WiFi.getMode());  // 显示当前WIFI的模式
    Serial.print("Connecting to ");
    Serial.println(wifi_ssid);


#ifdef GFX_EXTRA_PRE_INIT
  GFX_EXTRA_PRE_INIT();
#endif

  // Init Display
  if (!gfx->begin())
  {
    Serial.println("gfx->begin() failed!");
  }
  lv_init();
 screenWidth = gfx->width();
  screenHeight = gfx->height();
disp_draw_buf = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * screenWidth * 32, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, screenWidth * 32);

/* Initialize the display */
    lv_disp_drv_init(&disp_drv);
    /* Change the following line to your display resolution */
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

/* Create simple label */
    //lv_obj_t *label = lv_label_create(lv_scr_act());
    //lv_label_set_text(label, "Hello Arduino! (V" GFX_STR(LVGL_VERSION_MAJOR) "." GFX_STR(LVGL_VERSION_MINOR) "." GFX_STR(LVGL_VERSION_PATCH) ")");
    //lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);




    pinMode(21, INPUT);



 

lv_obj_t *img1=lv_img_create(lv_scr_act());

lv_img_set_src(img1,&ui_img_images_long_png);

ui_init();
//k=0;
uint8_t wifi_bar_value=0;

WiFi.mode(WIFI_MODE_STA);
WiFi.begin(wifi_ssid,wifi_password);

while(WiFi.status()!=WL_CONNECTED)
{
 wifi_bar_value++;
 lv_bar_set_value(ui_pag1_Bar1_loading,  wifi_bar_value, LV_ANIM_OFF);
 //lv_label_set_text(ui_page2_Label1_time, hour());
 
 lv_timer_handler();
delay(100);
if (wifi_bar_value >= 100)
{

  lv_label_set_text_fmt(ui_page1_Label1_loadingtext, "%s连接超时正在，请检查WiFi信息是否有误", wifi_ssid);
  while (1)
  {
    lv_timer_handler();
  }
}

}
/*wifi连接成功*/
ESP_LOGD("WiFi","连接成功");
ESP_LOGD("WiFi","WiFi信号强度：%的，WiFi_RSSI()");

Serial.println("");
    Serial.println("WiFi connected."); //  WIFI 已经连接
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());  // 显示连接WIFI后的IP地址
    Serial.println(WiFi.RSSI());  // 显示连接WIFI后的IP地址
    Serial.print("WIFI Mode: ");
    Serial.println(WiFi.getMode());  // // 显示当前WIFI的模式

Udp.begin(localPort);
 setSyncProvider(getNtpTime);
  setSyncInterval(300);
GetCitycode();

  button21.attachClick(attachClick21);



}

//label_other=lv_label_create(lv_scr_act());

//lv_label_set_long_mode(label_other,LV_LABEL_LONG_SCROLL);
//lv_obj_set_size(label_other,300,200);
//lv_label_set_text(label_other,"adddddddddddfdcxcdfdfdfdccc");



unsigned long update_CurrentWeather_last;

unsigned long updateOthers_last;
unsigned long updateTimePostion_last;

int dht11_Temperature,dht11_Humidiy;

static lv_calendar_date_t highlighted_days[3];



void loop()
{

dht11_Temperature=  dht.readTemperature();
dht11_Humidiy=dht.readHumidity();



lv_label_set_text_fmt(ui_page2_Label1_wendu,"%d℃",dht11_Temperature) ;
lv_label_set_text_fmt(ui_page2_Label1_shidu,"%d%%",dht11_Humidiy) ;



// k++;
//ESP_LOGD("NTP","%d-%d-%d %d:%d:%d %d",year(),month(),day(),hour(),minute(),second(),now()-28800);

 //lv_label_set_text_fmt(label_other,"k:%d",k),

 lv_label_set_text_fmt(ui_page2_Label1_time,"%s:%s:%s",str_time_hour(),str_time_minute(),str_time_second());
  lv_label_set_text_fmt(ui_page2_Label1_date,"%d年%d月%d日",year(),month(),day()) ;
   lv_label_set_text_fmt(ui_page2_Label1_week,"星期%s",num_week(weekday(),4));

lv_obj_t* calendar = lv_calendar_create(lv_scr_act());
 
lv_obj_set_size(calendar, 250, 280);
lv_obj_align(calendar, LV_ALIGN_CENTER, 180, 30);  


const char* daysName[7] = { "日", "一", "二",  "三", "四", "五", "六" };
lv_obj_set_style_text_font(calendar, &lv_font_simsun_16_cjk, LV_PART_MAIN);
lv_calendar_set_day_names(calendar, daysName);

lv_calendar_set_showed_date(calendar,year(), month());
lv_calendar_set_today_date(calendar, year(), month(), day());

lv_obj_t * lv_calendar_header_dropdown_create(lv_obj_t * parent);
lv_calendar_header_dropdown_create(calendar);

lv_obj_set_style_border_color(calendar, lv_color_hex(0x0000ff), LV_PART_MAIN);
lv_obj_set_style_border_color(calendar, lv_color_hex(0x0000ff), LV_PART_ITEMS);

lv_calendar_set_highlighted_dates(calendar, highlighted_days, 3);

  // 定时1小时执行的任务
  if (millis() - update_CurrentWeather_last >= (updateTimes * 60 * 1000))
  {
    update_CurrentWeather_last = millis();
  }

  // 定时6小时执行的任务
  if (millis() - updateOthers_last >= (uptateOthers * 60 * 1000))
  {
    updateOthers_last = millis();
  
  }





 lv_timer_handler(); /* let the GUI do its work */
  delay(10);
 button21.tick();

}

void GetCitycode(){

  HTTPClient http;
  http.begin("http://wgeo.weather.com.cn/ip/?_="+String(now()-28800)); //HTTP
http.setUserAgent("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/124.0.0.0 Safari/537.36");

http.addHeader("referer","http://www.weather.com.cn/");


  int httpcode=http.GET();
  Serial.println("httpcode:"+String(httpcode));
  if(httpcode==200)

  {

String httpText=http.getString();
Serial.println(httpText);

  }
  http.end();
}


// 时间：时、分的补零操作
String str_time_hour()
{
  int hour_ = hour();
  if (hour_ <= 9)
  {
    return "0" + String(hour_);
  }
  else
  {
    return String(hour_);
  }
}

String str_time_minute()
{
  int minute_ = minute();
  if (minute_ <= 9)
  {
    return "0" + String(minute_);
  }
  else
  {
    return String(minute_);
  }
}

String str_time_second()
{
  int second_ = second();
  if (second_ <= 9)
  {
    return "0" + String(second_);
  }
  else
  {
    return String(second_);
  }
}


/*
@功能:判断星期并赋值
*/
char week1[10],week2[8],week3[2],week4[4];
char *num_week(uint8_t dayofweek,int Mode){
  switch(dayofweek)
  {
    case 1: 
    strcpy(week1,"Sunday");
    strcpy(week2,"周日");
    strcpy(week3,"Su");
    strcpy(week4,"日"); 
      break;
    case 2: 
    strcpy(week1,"Monday");
    strcpy(week2,"周一");
    strcpy(week3,"Mo");
    strcpy(week4,"一"); 
      break;
    case 3: 
    strcpy(week1,"Tuesday");
    strcpy(week2,"周二");
    strcpy(week3,"Tu");
    strcpy(week4,"二"); 
      break;
    case 4: 
    strcpy(week1,"Wednesday");
    strcpy(week2,"周三"); 
    strcpy(week3,"We");
    strcpy(week4,"三"); 
      break;
    case 5: 
    strcpy(week1,"Thursday");
    strcpy(week2,"周四"); 
    strcpy(week3,"Th");
    strcpy(week4,"四"); 
      break;
    case 6: 
    strcpy(week1,"Friday");
    strcpy(week2,"周五");
    strcpy(week3,"Fr"); 
    strcpy(week4,"五");
      break;
    case 7: 
    strcpy(week1,"Saturday");
    strcpy(week2,"周六"); 
    strcpy(week3,"Sa");
    strcpy(week4,"六");
      break;
    default:
    strcpy(week1,"NO");
    strcpy(week2,"无");
    strcpy(week3,"NO");
    strcpy(week4,"无");
      break; 
  }
  switch(Mode)
  {
    case 1: return week1; break;
    case 2: return week2; break;
    case 3: return week3; break;
    case 4: return week4; break;
  }
}




char pageNumber = 0;
// 按钮 - 单击事件
void attachClick21()
{
  pageNumber++;
  switch (pageNumber)
  {
  case 1:
    lv_disp_load_scr(ui_page1_Screen1_start);
    break;
  case 2:
    lv_disp_load_scr(ui_page2_Screen2_weather);
    break;
  case 3:
    lv_disp_load_scr(ui_page1_Screen1_start);
    pageNumber = 0;
    break;
  default:
    break;
  }
}

