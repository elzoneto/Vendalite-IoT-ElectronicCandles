/**
   httpUpdateSecure.ino

    Created on: 16.10.2018 as an adaptation of the ESP8266 version of httpUpdate.ino

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

WiFiMulti WiFiMulti;

// Set time via NTP, as required for x.509 validation
void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");  // UTC

  Serial.print(F("Waiting for NTP time sync: "));
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    yield();
    delay(500);
    Serial.print(F("."));
    now = time(nullptr);
  }

  Serial.println(F(""));
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
}

/**
 * This is lets-encrypt-x3-cross-signed.pem
 */
//const char* rootCACertificate = \
//"-----BEGIN CERTIFICATE-----\n" \
//"MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n" \
//"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
//"DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n" \
//"SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n" \
//"GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n" \
//"AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n" \
//"q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n" \
//"SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n" \
//"Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n" \
//"a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n" \
//"/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n" \
//"AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n" \
//"CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n" \
//"bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n" \
//"c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n" \
//"VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n" \
//"ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n" \
//"MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n" \
//"Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n" \
//"AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n" \
//"uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n" \
//"wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n" \
//"X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n" \
//"PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n" \
//"KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n" \
//"-----END CERTIFICATE-----\n";

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Niagara Research_2.4GHz", "Thenrnetwork24!");

  httpUpdate.rebootOnUpdate(true); // don't reboot on update

  Serial.print("my MAC address: ");
  Serial.println(WiFi.macAddress());
  
}

void loop() {

  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(2500);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(2500);                       // wait for a second
  
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    setClock();

    //WiFiClientSecure *client = new WiFiClientSecure;
    WiFiClientSecure client;
    //client.setCACert(rootCACertificate);

    Serial.println("This is old version of software");

    const size_t bufferSize = JSON_ARRAY_SIZE(5) + 5*JSON_OBJECT_SIZE(3) + 5*JSON_OBJECT_SIZE(6) + 461;
  DynamicJsonDocument doc(bufferSize);

    // Reading data over SSL may be slow, use an adequate timeout
    client.setTimeout(12000 / 1000); // timeout argument is defined in seconds for setTimeout

    // The line below is optional. It can be used to blink the LED on the board during flashing
    // The LED will be on during download of one buffer of data from the network. The LED will
    // be off during writing that buffer to flash
    // On a good connection the LED should flash regularly. On a bad connection the LED will be
    // on much longer than it will be off. Other pins than LED_BUILTIN may be used. The second
    // value is used to put the LED on. If the LED is on with HIGH, that value should be passed
    // httpUpdate.setLedPin(LED_BUILTIN, HIGH);
//    if(client){
//      {
//        HTTPClient https;
//  
//      Serial.print("[HTTPS] begin...\n");
//      String server = "https://vendalitemvcwebapiv4.azurewebsites.net/api/devicesapi/2";
//      if (https.begin(*client, server)){
//        Serial.print("[HTTPS] GET...\n");
//        // start connection and send HTTP header
//        int httpCode = https.GET();
//        if (httpCode > 0){
//          Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
//          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY){
//            String payload = https.getString();
//            deserializeJson(doc, payload);
//            String ForceUpdate = doc["ForceUpdate"]
//            Serial.println(ForceUpdate);
//            delay(1000);
//          }
//        }
//      }
//      }
//    }

    t_httpUpdate_return ret = httpUpdate.update(client, "https://vendalitemvcwebapiv4.azurewebsites.net/api/devicesapi/DownloadSoftware/3");
    // Or:
    //t_httpUpdate_return ret = httpUpdate.update(client, "server", 443, "BasicHttpsClientControlByPWM.ino.feather_esp32.bin");
    Serial.println("This is old version of software");
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
    }
  }
}
