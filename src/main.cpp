#include <Arduino.h>
#include <main.h>

#include <FS.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>

/*
..#######..########........##.########..######..########..######.
.##.....##.##.....##.......##.##.......##....##....##....##....##
.##.....##.##.....##.......##.##.......##..........##....##......
.##.....##.########........##.######...##..........##.....######.
.##.....##.##.....##.##....##.##.......##..........##..........##
.##.....##.##.....##.##....##.##.......##....##....##....##....##
..#######..########...######..########..######.....##.....######.
*/
ESP8266WebServer server(80);            // web server
IPAddress apIP(192, 168, 10, 10);       // Adress AP
WiFiUDP ntpUDP;                         // UDP client for time
IPAddress timeServerIP;                 // server IP adress
Config config;                          // config structure

/*
.########..########...#######...######..########.########..##.....##.########..########..######.
.##.....##.##.....##.##.....##.##....##.##.......##.....##.##.....##.##.....##.##.......##....##
.##.....##.##.....##.##.....##.##.......##.......##.....##.##.....##.##.....##.##.......##......
.########..########..##.....##.##.......######...##.....##.##.....##.########..######....######.
.##........##...##...##.....##.##.......##.......##.....##.##.....##.##...##...##.............##
.##........##....##..##.....##.##....##.##.......##.....##.##.....##.##....##..##.......##....##
.##........##.....##..#######...######..########.########...#######..##.....##.########..######.
*/
void wifiConnect();                     // try connect to Wifi
void fileindex();                       // web index page
void fileaqua();                        // web aqua page
void fileparams();                      // web params page
void filelight();                       // web light page
void style();                           // web style css
void styleaqua();                       // web style aqua css
void stylelight();                      // web style light css
void styleparams();                     // web style params css
void aquapng();                         // web aqua png
void lightpng();                        // web light png
void parampng();                        // web param png
void printTime();                       // print time for debug
void timeUpdateNTP();                   // update time from iNet
void getNTPtime();                      // get time from server
void printFile(const char *filename);   // print file for debug
void saveConfig(const char *filename, Config &config);  // load config from file
void loadConfiguration(const char *filename, Config &config);   // save config to file
void updateTime();                      // update time, work clock!
void sendData();                        // send data to web
void restart();                         // restart controller
void saveContent();                     // save web content
void script();                          // web script js

/*
..######..########.########.##.....##.########.
.##....##.##..........##....##.....##.##.....##
.##.......##..........##....##.....##.##.....##
..######..######......##....##.....##.########.
.......##.##..........##....##.....##.##.......
.##....##.##..........##....##.....##.##.......
..######..########....##.....#######..##.......
*/
void setup() {
    #ifdef DEBUG_ENABLE
        Serial.begin(57600);
    #endif

    //FS
    SPIFFS.begin();
    //FS

    DEBUG("");
    DEBUG("START >>>");

    loadConfiguration(fileConfigName, config);  //loadconfig from file

    DEBUG("");

    wifiConnect();                      // try connect to Wifi

    localMillisAtUpdate = millis();
    localEpoc = (hour * 60 * 60 + minute * 60 + second);

    if(WiFi.status() == WL_CONNECTED) {
        ntpUDP.begin(localPort);            // Run UDP for take a time
        timeUpdateNTP();                    // update Time
    }

    //WEB
    server.begin();
    server.on("/", fileindex);
    server.on("/index", fileindex);
    server.on("/aqua.html", fileaqua);
    server.on("/light.html", filelight);
    server.on("params.html", fileparams);
    server.on("/params.html", fileparams);
    server.on("style.css", style);
    server.on("/style.css", style);
    server.on("style-aqua.css", styleaqua);
    server.on("/style-aqua.css", styleaqua);
    server.on("style-light.css", stylelight);
    server.on("/style-light.css", stylelight);
    server.on("style-params.css", styleparams);
    server.on("/style-params.css", styleparams);
    server.on("aqua.png", aquapng);
    server.on("/aqua.png", aquapng);
    server.on("light.png", lightpng);
    server.on("/light.png", lightpng);
    server.on("params.png", parampng);
    server.on("/params.png", parampng);
    server.on("script.js", script);
    server.on("/script.js", script);


    server.on("/getData", sendData);
    server.on("/saveContent", saveContent);
    server.on("/restart", restart);
    //WEB
}

/*
.##........#######...#######..########.
.##.......##.....##.##.....##.##.....##
.##.......##.....##.##.....##.##.....##
.##.......##.....##.##.....##.########.
.##.......##.....##.##.....##.##.......
.##.......##.....##.##.....##.##.......
.########..#######...#######..##.......
*/
void loop() {
// WE SERVER
    server.handleClient();

// WORK WITH TIME
    if(second != lastSecond) {
        lastSecond = second;
        secFr = 0;
    } else {
        secFr++;
    }

// WORK CLOCK
    updateTime();


// CHECK WIFI
    if ((minute % 5 == 0) && (second == 0) && (secFr == 0) && (WiFi.status() != WL_CONNECTED || !WIFI_connected)) {
        WIFI_connected = false;

        if (secFr == 0) DEBUG("============> Check WIFI connect!!!");
        
        wifiConnect();
    }

// Get time every hour from server
    if ((minute == 0) and (second == 0) and (secFr == 0)) {
        if (WIFI_connected) {
           getNTPtime();               // ***** Получение времени из интернета
        }
    }

    if ((not secFr) and (minute == 0))
    {
        printTime();
    }



}

/*
.########.##.....##.##....##..######..########.####..#######..##....##..######.
.##.......##.....##.###...##.##....##....##.....##..##.....##.###...##.##....##
.##.......##.....##.####..##.##..........##.....##..##.....##.####..##.##......
.######...##.....##.##.##.##.##..........##.....##..##.....##.##.##.##..######.
.##.......##.....##.##..####.##..........##.....##..##.....##.##..####.......##
.##.......##.....##.##...###.##....##....##.....##..##.....##.##...###.##....##
.##........#######..##....##..######.....##....####..#######..##....##..######.
*/

/*
.##......##.####.########.####.....######...#######..##....##.##....##.########..######..########
.##..##..##..##..##........##.....##....##.##.....##.###...##.###...##.##.......##....##....##...
.##..##..##..##..##........##.....##.......##.....##.####..##.####..##.##.......##..........##...
.##..##..##..##..######....##.....##.......##.....##.##.##.##.##.##.##.######...##..........##...
.##..##..##..##..##........##.....##.......##.....##.##..####.##..####.##.......##..........##...
.##..##..##..##..##........##.....##....##.##.....##.##...###.##...###.##.......##....##....##...
..###..###..####.##.......####.....######...#######..##....##.##....##.########..######.....##...
*/
void wifiConnect() {
    printTime();
    DEBUG("Connecting WiFi (ssid=" + String(config.ssid) + "  pass=" + String(config.password) + ") ");

#ifdef DEBUG_ENABLE
    if (!firstStart) Serial.println("Trying connecting to Wifi");
#endif

    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.ssid, config.password);

    for (int i = 1; i < 21; i++)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            WIFI_connected = true;
            DEBUG("Wifi connected, IP adress : ");
            DEBUG(WiFi.localIP());
            // if (!firstStart)
            // {
            //     String msg = WiFi.localIP().toString();
            //     DEBUG(" IP: ");
            //     DEBUG(msg);
            // }
            firstStart = 1;
            amountNotStarts = 0;
            WIFI_connected = true;
            return;
        }
        DEBUG(".");
        // if (!firstStart)
        // {
            int j = 0;
            while (j < 500)
            {
                if (j % 10 == 0)
                    delay(5);
                j++;
                delay(2);
            }
        // }
    }
    
    WiFi.disconnect();
    DEBUG(" Not connected!!!");
    amountNotStarts++;
    DEBUG("Amount of the unsuccessful connecting = ");
    DEBUG(amountNotStarts);

    if (amountNotStarts > 6)
        ESP.reset();

    WiFi.disconnect();

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(config.ssidAP, config.passwordAP);
    printTime();    
    DEBUG("Start AP mode!!!");
    DEBUG("Wifi AP IP : ");
    DEBUG(WiFi.softAPIP());

    firstStart = 1;
}

/*
.##......##.########.########.
.##..##..##.##.......##.....##
.##..##..##.##.......##.....##
.##..##..##.######...########.
.##..##..##.##.......##.....##
.##..##..##.##.......##.....##
..###..###..########.########.
*/
void fileindex() {
    File file = SPIFFS.open("/index.html.gz", "r");
    size_t sent = server.streamFile(file, "text/html");
    DEBUG("load index page");
}

void fileaqua() {
    File file = SPIFFS.open("/aqua.html.gz", "r");
    size_t sent = server.streamFile(file, "text/html");
    DEBUG("load aqua page");
}

void filelight() {
    File file = SPIFFS.open("/light.html.gz", "r");
    size_t sent = server.streamFile(file, "text/html");
    DEBUG("load light page");
}

void fileparams() {
    File file = SPIFFS.open("/params.html.gz", "r");
    size_t sent = server.streamFile(file, "text/html");
    DEBUG("load light page");
}

void style() {
    File file = SPIFFS.open("/style.css.gz", "r");
    size_t sent = server.streamFile(file, "text/css");
    DEBUG("load style css");
}

void styleaqua() {
    File file = SPIFFS.open("/style-aqua.css.gz", "r");
    size_t sent = server.streamFile(file, "text/css");
    DEBUG("load aqua style css");
}

void stylelight() {
    File file = SPIFFS.open("/style-light.css.gz", "r");
    size_t sent = server.streamFile(file, "text/css");
    DEBUG("load light style css");
}

void styleparams() {
    File file = SPIFFS.open("/style-params.css.gz", "r");
    size_t sent = server.streamFile(file, "text/css");
    DEBUG("load params style css");
}

void aquapng() {
    File file = SPIFFS.open("/aqua.png", "r");
    size_t sent = server.streamFile(file, "image/png");
    DEBUG("load aqua png");
}
void lightpng() {
    File file = SPIFFS.open("/light.png", "r");
    size_t sent = server.streamFile(file, "image/png");
    DEBUG("load light png");
}
void parampng() {
    File file = SPIFFS.open("/params.png", "r");
    size_t sent = server.streamFile(file, "image/png");
    DEBUG("load param png");
}

void script() {
    File file = SPIFFS.open("/script.js.gz", "r");
    size_t sent = server.streamFile(file, "application/javascript");
}

void sendData() {
    DEBUG("Send data in WEB");

    String json = "{";
    //wifi
    json += "\"ssid\":\"";
    json += config.ssid;
    json += "\",\"password\":\"";
    json += config.password;
    json += "\",\"ssidAP\":\"";
    json += config.ssidAP;
    json += "\",\"passwordAP\":\"";
    json += config.passwordAP;
    //Time
    json += "\",\"timezone\":\"";
    json += config.timeZone;
    json += "\",\"summertime\":\"";
    json += config.summerTime;
    json += "\",\"ntpServerName\":\"";
    json += config.ntpServerName;

    //TODO: Add sending data to web here!

    json += "\"}";

    server.send (200, "text/json", json);
    Serial.println(json);
}

 void saveContent() {
    DEBUG("save web content!");

    //Wifi
    server.arg("ssid").toCharArray(config.ssid, 50) ;
    server.arg("password").toCharArray(config.password, 50) ;
    server.arg("ssidAP").toCharArray(config.ssidAP, 50) ;
    server.arg("passwordAP").toCharArray(config.passwordAP, 50) ;

    //Time
    config.timeZone = server.arg("timezone").toFloat();
    config.summerTime = server.arg("summertime").toInt();
    server.arg("ntpServerName").toCharArray(config.ntpServerName, 50) ;

    saveConfig(fileConfigName, config);
}

void restart() {
    server.send(200, "text/json", "");
    delay(100);
    ESP.restart();
}

/*
.##.....##.########..########.....###....########.########.########.####.##.....##.########
.##.....##.##.....##.##.....##...##.##......##....##..........##.....##..###...###.##......
.##.....##.##.....##.##.....##..##...##.....##....##..........##.....##..####.####.##......
.##.....##.########..##.....##.##.....##....##....######......##.....##..##.###.##.######..
.##.....##.##........##.....##.#########....##....##..........##.....##..##.....##.##......
.##.....##.##........##.....##.##.....##....##....##..........##.....##..##.....##.##......
..#######..##........########..##.....##....##....########....##....####.##.....##.########
*/
void timeUpdateNTP() {
    if(!WIFI_connected) return;
    printTime();
    statusUpdateNtpTime = 1;
    for(int timeTest = 0; timeTest < 3; timeTest++) {
        getNTPtime();

        DEBUG("Proba #"+String(timeTest+1)+"   "+String(g_hour)+":"+((g_minute<10)?"0":"")+String(g_minute)+":"+((g_second<10)?"0":"")+String(g_second)+":"+String(g_dayOfWeek)+":"+String(g_day)+":"+String(g_month)+":"+String(g_year));

        hourTest[timeTest] = g_hour;
        minuteTest[timeTest] = g_minute;
        if(statusUpdateNtpTime == 0) {
            printTime();
            DEBUG("ERROR TIME!!!\r\n");
                return;
        }
        if(timeTest > 0) {
            if((hourTest[timeTest] != hourTest[timeTest - 1]||minuteTest[timeTest] != minuteTest[timeTest - 1])) {
                statusUpdateNtpTime = 0;
                printTime();
                DEBUG("ERROR TIME!!!\r\n");
                return;
            }
        }
    }

    hour=g_hour;
    minute=g_minute;
    second=g_second;
    day=g_day;
    dayOfWeek=g_dayOfWeek;
    month=g_month;
    year=g_year;

    localMillisAtUpdate = millis();
    localEpoc = (hour * 60 * 60 + minute * 60 + second);

    printTime();

    DEBUG((day < 10 ? "0" : "") + String(day) + "." + (month < 10 ? "0" : "") + String(month) + "." + String(year) + " DW = " + String(dayOfWeek));
    DEBUG("Time update OK.");
}

/*
.########..########..####.##....##.########.########.####.##.....##.########
.##.....##.##.....##..##..###...##....##.......##.....##..###...###.##......
.##.....##.##.....##..##..####..##....##.......##.....##..####.####.##......
.########..########...##..##.##.##....##.......##.....##..##.###.##.######..
.##........##...##....##..##..####....##.......##.....##..##.....##.##......
.##........##....##...##..##...###....##.......##.....##..##.....##.##......
.##........##.....##.####.##....##....##.......##....####.##.....##.########
*/
void printTime() {
    DEBUG((hour < 10 ? "0" : "") + String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute) + ":" + (second < 10 ? "0" : "") + String(second) + "  ");
}

/*
..######...########.########.##....##.########.########..########.####.##.....##.########
.##....##..##..........##....###...##....##....##.....##....##.....##..###...###.##......
.##........##..........##....####..##....##....##.....##....##.....##..####.####.##......
.##...####.######......##....##.##.##....##....########.....##.....##..##.###.##.######..
.##....##..##..........##....##..####....##....##...........##.....##..##.....##.##......
.##....##..##..........##....##...###....##....##...........##.....##..##.....##.##......
..######...########....##....##....##....##....##...........##....####.##.....##.########
*/
void getNTPtime() {

    WiFi.hostByName(config.ntpServerName, timeServerIP);

    int cb;
    for(int i = 0; i < 3; i++){
        memset(packetBuffer, 0, NTP_PACKET_SIZE);
        packetBuffer[0] = 0b11100011;
        packetBuffer[1] = 0;
        packetBuffer[2] = 6;
        packetBuffer[3] = 0xEC;
        packetBuffer[12] = 49;
        packetBuffer[13] = 0x4E;
        packetBuffer[14] = 49;
        packetBuffer[15] = 52;
        ntpUDP.beginPacket(timeServerIP, 123);                       //NTP порт 123
        ntpUDP.write(packetBuffer, NTP_PACKET_SIZE);
        ntpUDP.endPacket();
        delay(800);                                                  // чекаємо пів секуни
        cb = ntpUDP.parsePacket();
        if(!cb) Serial.println("          no packet yet..." + String (i + 1));
        if(!cb && i == 2) {                                          // якщо час не отримано
        statusUpdateNtpTime = 0;
        return;                                                    // вихіз з getNTPtime()
        }
        if(cb) i = 3;
    }

    if(cb) {                                                      // якщо отримали пакет з серверу
        ntpUDP.read(packetBuffer, NTP_PACKET_SIZE);
        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
        unsigned long secsSince1900 = highWord << 16 | lowWord;
        const unsigned long seventyYears = 2208988800UL;        // Unix час станом на 1 січня 1970. в секундах, то 2208988800:
        unsigned long epoch = secsSince1900 - seventyYears;
        boolean summerTime;

        if(month < 3 || month > 10) summerTime = false;             // не переходимо на літній час в січні, лютому, листопаді і грудню
        if(month > 3 && month < 10) summerTime = true;              // Sommerzeit лічимо в квіні, травні, червені, липні, серпені, вересені
        if(((month == 3) && (hour + 24 * day)) >= (3 + 24 * (31 - (5 * year / 4 + 4) % 7)) || ((month == 10) && (hour + 24 * day)) < (3 + 24 * (31 - (5 * year / 4 + 1) % 7))) summerTime = true;
        epoch += (int)(config.timeZone*3600 + (3600*(summertime   /*isDayLightSaving*/ && summerTime)));

        g_year = 0;
        int days = 0;
        uint32_t time;
        time = epoch/86400;
        g_hour = (epoch % 86400L) / 3600;
        g_minute = (epoch % 3600) / 60;
        g_second = epoch % 60;
        g_dayOfWeek = (((time) + 4) % 7) + 1;
        while((unsigned)(days += (LEAP_YEAR(g_year) ? 366 : 365)) <= time) {    // Счет года
        g_year++;
        }
        days -= LEAP_YEAR(g_year) ? 366 : 365;
        time -= days;
        days = 0;
        g_month = 0;
        uint8_t monthLength = 0;
        for(g_month = 0; g_month < 12; g_month++){                      // Счет месяца
        if(g_month == 1){
            if(LEAP_YEAR(g_year)) monthLength = 29;
            else monthLength = 28;
        }
        else monthLength = monthDays[g_month];
        if(time >= monthLength) time -= monthLength;
        else break;
        }
        g_month++;
        g_day = time + 1;
        g_year += 1970;
        return;
    }
    DEBUG("Nie ma czasu(((");
}

/*
.##........#######.....###....########...........######...#######..##....##.########.####..######..
.##.......##.....##...##.##...##.....##.........##....##.##.....##.###...##.##........##..##....##.
.##.......##.....##..##...##..##.....##.........##.......##.....##.####..##.##........##..##.......
.##.......##.....##.##.....##.##.....##.........##.......##.....##.##.##.##.######....##..##...####
.##.......##.....##.#########.##.....##.........##.......##.....##.##..####.##........##..##....##.
.##.......##.....##.##.....##.##.....##.........##....##.##.....##.##...###.##........##..##....##.
.########..#######..##.....##.########..#######..######...#######..##....##.##.......####..######..
*/
void loadConfiguration(const char *filename, Config &config) {

    File file = SPIFFS.open("/config.txt", "r");


    const size_t capacity = JSON_OBJECT_SIZE(7) + 200;
    DynamicJsonDocument doc(capacity);

    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        DEBUG(F("Failed to read file, using default configuration"));
        DEBUG("Error is :");
        DEBUG(error.c_str());
    }

    //Wifi
#ifdef HOME
    strlcpy(config.ssid, doc["ssid"] | "PUTIN UTELE", sizeof(config.ssid));
    strlcpy(config.password, doc["password"] | "0674788273", sizeof(config.password));
#else
    strlcpy(config.ssid, doc["ssid"] | "SUERTEKSA CNC", sizeof(config.ssid));
    strlcpy(config.password, doc["password"] | "61347400", sizeof(config.password));
#endif
    strlcpy(config.ssidAP, doc["ssidAP"] | "AQUA_ROOM_AP", sizeof(config.ssidAP));
    strlcpy(config.passwordAP, doc["passwordAP"] | "1111", sizeof(config.passwordAP));

    //Time
    config.timeZone = doc["timezone"] | 2.0;
    config.summerTime = doc["summertime"] | 0;
    strlcpy(config.ntpServerName, doc["ntpServerName"] | "ntp3.time.in.ua", sizeof(config.ntpServerName));


    // TODO: Add all parameters for loading


#ifdef DEBUG
    DEBUG("****** LOAD FILE ******");
    printFile(fileConfigName);
#endif

    file.close();
}

/*
..######.....###....##.....##.########..........######...#######..##....##.########.####..######..
.##....##...##.##...##.....##.##...............##....##.##.....##.###...##.##........##..##....##.
.##........##...##..##.....##.##...............##.......##.....##.####..##.##........##..##.......
..######..##.....##.##.....##.######...........##.......##.....##.##.##.##.######....##..##...####
.......##.#########..##...##..##...............##.......##.....##.##..####.##........##..##....##.
.##....##.##.....##...##.##...##...............##....##.##.....##.##...###.##........##..##....##.
..######..##.....##....###....########.#######..######...#######..##....##.##.......####..######..
*/
void saveConfig(const char *filename, Config &config) {

    SPIFFS.remove(filename);

   // Open file for writing
    File file = SPIFFS.open(filename, "w");
    if (!file) {
        Serial.println(F("Failed to create file"));
        return;
    }

    const size_t capacity = JSON_OBJECT_SIZE(26) + 720;
    DynamicJsonDocument doc(capacity);

    doc["ssid"] = config.ssid;
    doc["password"] = config.password;
    doc["ssidAP"] = config.ssidAP;
    doc["passwordAP"] = config.passwordAP;
    doc["timezone"] = config.timeZone;
    doc["summertime"] = config.summerTime;
    doc["ntpServerName"] = config.ntpServerName;

    //TODO: Add all params for saving

    if (serializeJson(doc, file) == 0) {
        Serial.println(F("Failed to write to file"));
    }

#ifdef DEBUG
    DEBUG("****** SAVE FILE ******");
    printFile(fileConfigName);
#endif

    file.close();
}

/*
.########..########..####.##....##.########.........########.####.##.......########
.##.....##.##.....##..##..###...##....##............##........##..##.......##......
.##.....##.##.....##..##..####..##....##............##........##..##.......##......
.########..########...##..##.##.##....##............######....##..##.......######..
.##........##...##....##..##..####....##............##........##..##.......##......
.##........##....##...##..##...###....##............##........##..##.......##......
.##........##.....##.####.##....##....##....#######.##.......####.########.########
*/
void printFile(const char *filename) {
  // Open file for reading
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    DEBUG(F("Failed to read file"));
    return;
  }

  // Extract each characters by one by one
  while (file.available()) {
    Serial.print((char)file.read());
  }

  // Close the file
  file.close();
}

/*
.##.....##.########..########.....###....########.########.........########.####.##.....##.########
.##.....##.##.....##.##.....##...##.##......##....##..................##.....##..###...###.##......
.##.....##.##.....##.##.....##..##...##.....##....##..................##.....##..####.####.##......
.##.....##.########..##.....##.##.....##....##....######..............##.....##..##.###.##.######..
.##.....##.##........##.....##.#########....##....##..................##.....##..##.....##.##......
.##.....##.##........##.....##.##.....##....##....##..................##.....##..##.....##.##......
..#######..##........########..##.....##....##....########.#######....##....####.##.....##.########
*/
void updateTime() {
  long curEpoch = localEpoc + ((millis() - localMillisAtUpdate) / 1000);
  long epoch = round(double((curEpoch + 86400L) % 86400L));
  hour = ((epoch % 86400L) / 3600) % 24;

  minute = (epoch % 3600) / 60;
  second = epoch % 60;
}
