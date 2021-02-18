#include <Arduino.h>


#define DEBUG_ENABLE        //DEBUG MODE
 #define HOME                // switch to home work

#ifdef DEBUG_ENABLE
#define DEBUG(x) Serial.println(x)
#else
#define DEBUG(x)
#endif

// #ifdef HOME
// const char* ssid = "PUTIN UTELE";
// const char* password = "0674788273";
// #else
// const char* ssid = "SUERTEKSA CNC";
// const char* password = "61347400";
// #endif
// const char* ssidAP      = "Aquarium-AP";         // Назва точки доступу
// const char* passwordAP  = "" ;                   // Пароль точки доступу

boolean firstStart = false;                 // Первый старт
boolean WIFI_connected = false;             // Флаг подкючекния к ВАЙФАЙ
byte amountNotStarts = 0;                   // Счет НЕ подключения

//time
long localEpoc = 0;
long localMillisAtUpdate = 0;
int hour=22, minute=20, second=0, month=4, day=6, dayOfWeek=6, year=2018;
unsigned int localPort = 2390;
bool statusUpdateNtpTime = 0;             // Если не 0, то обновление было удачным
// float timeZone = 2.0;                     // Временная зона для часов
long timeUpdate = 60000;                  // Период обновления времени
uint8_t hourTest[3], minuteTest[3];       // ??
int g_hour, g_minute, g_second, g_month=1, g_day, g_dayOfWeek, g_year;  // ??
// const char* ntpServerName = "ntp3.time.in.ua";   // Сервер обновления времени
const int NTP_PACKET_SIZE = 48;           // Размер пакета от сервера времени
byte packetBuffer[NTP_PACKET_SIZE];       // Буфер пакета времени
// float timeZone;                                     // Временная зона для часов
int summertime = 1;                                     // летнее время
#define LEAP_YEAR(Y) (((1970+Y)>0) && !((1970+Y)%4) && (((1970+Y)%100)||!((1970+Y)%400)))   // Високосні літа
static const uint8_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};        // Кількість днів у місяцях
int secFr, lastSecond, lastMinute;                    // Работа с временем
int j = 0;

// Config structure
struct Config {
#ifdef HOME
    char ssid[50] = "PUTIN UTELE";
    char password[50] = "0674788273";
#else
    char ssid[50] = "SUERTEKSA CNC";
    char password[50] = "61347400";
#endif
    char ssidAP[50] = "Aquarium-AP";
    char passwordAP[50] = "" ;
    float timeZone = 2.0;
    char ntpServerName[50] = "ntp3.time.in.ua";
    int summerTime = 0;
};

const char* fileConfigName = "/config.txt";
// Config structure
