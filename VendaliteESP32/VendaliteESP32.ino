/* 
  Created on: 26.08.2021
  WAMIC - Vendalite
*/
#include <Arduino.h>

#include <ArduinoJson.h>

#include <WiFi.h>

#include <WiFiMulti.h>

#include <HTTPClient.h>

#include <HTTPUpdate.h>

#include <time.h>

#include <WiFiClientSecure.h>

#include <Wire.h>

#include <Adafruit_PWMServoDriver.h>


/*AWS Vendalite (expires Nov 16, 2021) 
  then R3 (expires Sep 15, 2025) 
  then (ISRG Root X1 certificate expires Jun 4, 2035)*/
//
const char * rootCACertificate = \
  "-----BEGIN CERTIFICATE-----\n"\
"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\n"\
"-----END CERTIFICATE-----\n";

// Set time via NTP, as required for x.509 validation
void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // UTC

  Serial.print(F("Waiting for NTP time sync: "));
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    yield();
    delay(500);
    Serial.print(F("."));
    now = time(nullptr);
  }
}

WiFiMulti WiFiMulti;

//For testing 
int ledState = LOW;
const int ledPin0 = 21;
const int ledPin1 = 17;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  //WiFi connected
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Niagara Research_2.4GHz", "Thenrnetwork24!");
  httpUpdate.rebootOnUpdate(true); // don't reboot on update
  Serial.print("my MAC address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  //CODE IN PROGRESS...
  //file found at server
  Serial.println();
  Serial.println("INSIDE THE LOOP");

  if ((WiFiMulti.run() == WL_CONNECTED)) {
    setClock();

    //Setting digital pins as output:
    pinMode(ledPin0, OUTPUT);
    pinMode(ledPin1, OUTPUT);

    WiFiClientSecure * client = new WiFiClientSecure;
    const size_t bufferSize = JSON_ARRAY_SIZE(5) + 5 * JSON_OBJECT_SIZE(3) + 5 * JSON_OBJECT_SIZE(6) + 461;
    DynamicJsonDocument doc(bufferSize);

    if (client) {
      client -> setCACert(rootCACertificate); {
        HTTPClient https;
        Serial.println("[HTTPS] begin...\n");

        String server = "https://vendalite.com/gabriel/api/esp32devicesapi/bychurch/1";
        https.begin( * client, server);
        https.addHeader("Authorization", "Basic YWRtaW4xQG91dGxvb2suY29tOnBhc3N3b3Jk");

        if (true) {
          // start connection and send HTTP header
          Serial.print("[HTTPS] GET...\n");
          int httpCode = https.GET();

          // httpCode will be negative on error
          if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {

              String payload = https.getString();
              deserializeJson(doc, payload);
              String ForceUpdate = doc[0]["forceUpdate"];
              delay(5000);

              Serial.println("wait 5s...");
              Serial.println();

              Serial.println("FIRMWARE UPDATE PROCESS: STARTING...");
              Serial.println();

              if (ForceUpdate == "true") {
                Serial.println("Start process to update software");
                Serial.println(" ");

                t_httpUpdate_return ret = httpUpdate.update( * client, "https://vendalite.com/gabriel/api/esp32devicesapi/updatedevice/1");

                Serial.println("This is old version of software");
                Serial.println(ret);

                switch (ret) {
                case HTTP_UPDATE_FAILED:
                  Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
                  break;

                case HTTP_UPDATE_NO_UPDATES:
                  Serial.println("HTTP_UPDATE_NO_UPDATES: ");
                  break;

                case HTTP_UPDATE_OK:
                  Serial.println("HTTP_UPDATE_OK");
                  break;
                }
                Serial.println("Update Done!");

              } else {
                Serial.println("Device Force update is not checked. Please, check the device details page.");
              }

              Serial.println();
              Serial.println("CANDLE COUNTER: STARTING...");
              Serial.println();

              //String payload = https.getString();
              Serial.println(payload);

              deserializeJson(doc, payload);
              String ledcheck = doc[0]["countCandleLight"];
              Serial.println(ledcheck);

              int timeToLoop = ledcheck.toInt();
              for (int i = 0; i < timeToLoop; i++) {
                Serial.println(i);
                ledState = HIGH;
                digitalWrite(ledPin0, ledState);
                digitalWrite(ledPin1, ledState);
                delay(100);
                ledState = LOW;
                digitalWrite(ledPin0, ledState);
                digitalWrite(ledPin1, ledState);
                delay(1000);
              }
              String newurl = "https://vendalite.com/gabriel/api/ESP32DevicesApi/ForESP32Device/1?counter=" + String(ledcheck);
              if (https.begin( * client, newurl)) {
                int httpCode = https.PUT(String(ledcheck));
                Serial.print(httpCode);
              }
            }
          } else {
            Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
          }
        }
      }
      delete client;
    } else {
      Serial.println("Unable to create client");
    }
  }

  Serial.println();
  Serial.println("Waiting 60s before the next round...");
  delay(60000);
}
