#include <Arduino.h>
#include "time.h"
#include <ESP32Time.h>
#include <wifi.cpp>
#include <FS.h>
#include <SPIFFS.h> 
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <./webServer.h>
#define FORMAT_SPIFFS_IF_FAILED true


// Ntp
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;
const int   daylightOffset_sec = 3600;

// Rtc
ESP32Time rtc;

// Relays 
const byte relayPins[12] = {32, 33, 25, 26, 27, 14, 13, 19, 18, 5, 4, 0};

struct Relay {
  String label; // light 1 / light 2 / pump 1   
  byte pinNumber;
  String status; // on / off / auto
  bool overridable;
  String startTime;
  String stopTime;
};

struct Relay relays[12] = {
  { "All lights", 99,         "auto",false, "08:00:00", "20:00:00" },
  { "Pump",    relayPins[9], "auto",false, "15:00:00", "15:00:30" },
  { "Light 1", relayPins[0],  "auto", true, "08:00:00", "20:00:00" },
  { "Light 2", relayPins[1],  "auto", true, "08:00:00", "20:00:00" },
  { "Light 3", relayPins[2],  "auto", true, "08:00:00", "20:00:00" },
  { "Light 4", relayPins[3],  "auto", true, "08:00:00", "20:00:00" },
  { "Light 5", relayPins[4],  "auto", true, "08:00:00", "20:00:00" },
  { "Light 6", relayPins[5],  "auto", true, "08:00:00", "20:00:00" },
  { "Light 7", relayPins[6],  "auto", true, "08:00:00", "20:00:00" },
  { "Light 8", relayPins[7],  "auto", true, "08:00:00", "20:00:00" },
  { "Light 9", relayPins[8],  "auto", true, "08:00:00", "20:00:00" },
};
bool applyToAllLights = true;

// Server
AsyncWebServer server(80);

// Functions // TODO move that away
long timeToSeconds(String mytime){
  long hours = mytime.substring(0, 2).toInt()*60*60;
  long min = mytime.substring(3, 5).toInt()*60;
  long sec = mytime.substring(6, 8).toInt();
  return hours+min+sec;
}

void turnAllLight(bool on, int Tdelay){

  if(on){
    digitalWrite(relayPins[0], LOW);delay(Tdelay);
    digitalWrite(relayPins[1], LOW);delay(Tdelay);
    digitalWrite(relayPins[2], LOW);delay(Tdelay);
    digitalWrite(relayPins[3], LOW);delay(Tdelay);
    digitalWrite(relayPins[4], LOW);delay(Tdelay);
    digitalWrite(relayPins[5], LOW);delay(Tdelay);
    digitalWrite(relayPins[6], LOW);delay(Tdelay);
    digitalWrite(relayPins[7], LOW);delay(Tdelay);
    digitalWrite(relayPins[8], LOW);
}else{
    digitalWrite(relayPins[0], HIGH);delay(Tdelay);
    digitalWrite(relayPins[1], HIGH);delay(Tdelay);
    digitalWrite(relayPins[2], HIGH);delay(Tdelay);
    digitalWrite(relayPins[3], HIGH);delay(Tdelay);
    digitalWrite(relayPins[4], HIGH);delay(Tdelay);
    digitalWrite(relayPins[5], HIGH);delay(Tdelay);
    digitalWrite(relayPins[6], HIGH);delay(Tdelay);
    digitalWrite(relayPins[7], HIGH);delay(Tdelay);
    digitalWrite(relayPins[8], HIGH);
  }
}

void writeConfigsToCsv( Relay *relays, bool applyToAllLights){
    File f = SPIFFS.open("/config.csv", FILE_WRITE);
    String string = "";
    for (int i = 0; i < 11; i++) {
      string = relays[i].label+","+relays[i].status+","+relays[i].overridable+","+relays[i].startTime+","+relays[i].stopTime+","+applyToAllLights+",";
      Serial.print("writing line to file : "); Serial.println(string);
      f.println(string);
    }
    f.close();
}

void setup() {
  Serial.begin(9600);
  delay(100);

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS init failed");
  }

  connectToWifi();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  AdvertiseServices("plantes.local");

  int curCnt = 0; 
  File f = SPIFFS.open("/config.csv", "r");
  if (!f) {
    writeConfigsToCsv(relays,applyToAllLights);
  }else{
    char buffer[512];
    int row = 0 ;
    int field = 0 ; 
    String fieldBuff ="";    
    while (f.available()) {
      int l = f.readBytesUntil('\n', buffer, sizeof(buffer));
      char* d = strtok(buffer, ",");
      while (d != NULL) {   
        if(field == 0 ){ (relays[row]).label = d;}
        if(field == 1 ){ (relays[row]).status = d;}
        if(field == 2 ){ (relays[row]).overridable = d;}
        if(field == 3 ){ (relays[row]).startTime = d;}
        if(field == 4 ){ (relays[row]).stopTime = d;}
        if(field == 5 ){ applyToAllLights = d;}
        field++;
        d = strtok(NULL, ",");
      }

      buffer[l] = 0;
      field = 0 ;
      row++;
    }
  }

  for (byte pinNumber : relayPins) {
    pinMode(pinNumber, OUTPUT);
    digitalWrite(pinNumber, HIGH);
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("/index.html");
  });

  server.on("/set", HTTP_GET, [](AsyncWebServerRequest *request) {
    String page  = request->getParam(0)->value();
    String field = request->getParam(1)->name();
    String value = request->getParam(1)->value();

    if( field.equals("status") ){ (relays[page.toInt()]).status = value; }
    if( field.equals("startTime") ){ (relays[page.toInt()]).startTime = value; }
    if( field.equals("stopTime") ){ (relays[page.toInt()]).stopTime = value; }
    if( field.equals("applyToAllLights") ){ applyToAllLights = value.toInt(); }
    
    writeConfigsToCsv(relays,applyToAllLights);


    
    if( page.equals("1") && field.equals("status") && value.equals("off") ){
      digitalWrite((relays[page.toInt()]).pinNumber, HIGH);
    }
    if( page.equals("1") && field.equals("status") && value.equals("on") ){
      digitalWrite((relays[page.toInt()]).pinNumber, LOW);
    } 
    if(applyToAllLights){
      if( page.equals("0") && field.equals("status") && value.equals("off") ){
        turnAllLight(false,100);
      }
      if( page.equals("0") && field.equals("status") && value.equals("on") ){
         turnAllLight(true,100);
      }

    }
    if(page.toInt() > 1 ){
      if(field.equals("status") && value.equals("off") ){
        digitalWrite((relays[page.toInt()]).pinNumber, HIGH);
      }
      if(  field.equals("status") && value.equals("on") ){
         digitalWrite((relays[page.toInt()]).pinNumber, LOW);
      }
      
    }


    String json = "{\"label\": \""+ relays[page.toInt()].label       + "\","
      + "\"pinNumber\": \""       + relays[page.toInt()].pinNumber   + "\","
      + "\"status\": \""          + relays[page.toInt()].status      + "\","
      + "\"overridable\": \""     + relays[page.toInt()].overridable + "\","
      + "\"startTime\": \""       + relays[page.toInt()].startTime   + "\","
      + "\"stopTime\":"+'"'       + relays[page.toInt()].stopTime    + "\","
      + "\"applyToAllLights\":\"" + applyToAllLights                 + "\""
      + "}";
    request->send(200, "text/html", json );
    });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
    String page  = request->getParam(0)->value();
    String json = "{\"label\": \""+ relays[page.toInt()].label       + "\","
      + "\"pinNumber\": \""       + relays[page.toInt()].pinNumber   + "\","
      + "\"status\": \""          + relays[page.toInt()].status      + "\","
      + "\"overridable\": \""     + relays[page.toInt()].overridable + "\","
      + "\"startTime\": \""       + relays[page.toInt()].startTime   + "\","
      + "\"stopTime\":"+'"'       + relays[page.toInt()].stopTime    + "\","
      + "\"applyToAllLights\":\"" + applyToAllLights                 + "\""
      + "}";
      request->send(200, "text/html", json );
  });

  server.serveStatic("/", SPIFFS, "/");
  server.begin();

  Serial.println("setup done");
}




void loop() {
  if (applyToAllLights) {
    if ( (relays[0]).status.equals("auto") ){
      if( timeToSeconds(rtc.getTime()) >= timeToSeconds(relays[0].startTime)
      && timeToSeconds(rtc.getTime()) <= timeToSeconds(relays[0].stopTime) ){
        turnAllLight(true,1500);
      }else{
        turnAllLight(false,1500);
      }
    }

  }else{
    for (int i = 2; i < 11; i++) {
      if (  (relays[i]).status.equals("auto") ){   // if on auto. 
        if( timeToSeconds(rtc.getTime()) >= timeToSeconds(relays[i].startTime)
        && timeToSeconds(rtc.getTime()) <= timeToSeconds(relays[i].stopTime) ){
            digitalWrite(relays[i].pinNumber, LOW);
        }else{
            digitalWrite(relays[i].pinNumber, HIGH);
        }
      }
    }   
  }

// pump 
  if ( (relays[1]).status.equals("auto") ){
    if( timeToSeconds(rtc.getTime()) >= timeToSeconds(relays[1].startTime)
    && timeToSeconds(rtc.getTime()) <= timeToSeconds(relays[1].startTime) ){
        digitalWrite(relayPins[9], LOW);
    }else{
        digitalWrite(relayPins[9], HIGH);
    }
  }
    





 
}

