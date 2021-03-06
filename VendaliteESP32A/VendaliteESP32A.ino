/**
 * File:  VendaliteESP32A
 * WAMIC Niagara College
 * Created on: 22.09.2021
 **/

/* All libraries necessary */
#include <Arduino.h>

#include <Preferences.h>

#include <WiFi.h>

#include <ESPAsyncWebServer.h>

#include <Arduino.h>

#include <ArduinoJson.h>

#include <HTTPClient.h>

#include <HTTPUpdate.h>

Preferences preferences;
AsyncWebServer server(80);

/* Setting up STA & AP WiFi network */
const char * wifi_network_ssid = "VliteSTA";
const char * wifi_network_password = "Gabriel!221186"; //Gabriel!221186
const char * soft_ap_ssid = "VliteAP";
const char * soft_ap_password = "Gabriel!001466"; //Gabriel!001466

/*  
 *  Creating and defining variables 
 *  for latter uses.
 */
#define MAXSTAATTEMPT 40
#define MAXCONFIGFORM 4096
#define CMACIPMAXLEN 64

#define PARAM_RESTART "restart"
#define PARAM_SSIDNAME "SSIDname"
#define PARAM_SSIDPASS "SSIDpass"
#define PARAM_DEVICEID "DeviceID"
#define PARAM_POSTTYPE "posttype"

char cSSIDname[64] = "SSIDname";
char cSSIDpass[64] = "SSIDpass";
char cDeviceID[64] = "DeviceID";
char lastAction[192] = "initialized";

char cMacAddr[CMACIPMAXLEN];
char cIPAddr[CMACIPMAXLEN];

String sSSIDname = "SSIDname";
String sSSIDpass = "SSIDpass";
String sDeviceID = "DeviceID";
String sPosttype;
char configForm[MAXCONFIGFORM]; // check that this is not exceeded

//LED PIN
int ledState = LOW;
const int ledPin0 = 21; //21
const int ledPin1 = 17; //17

/*
 * OnWiFi event class
 * prints STA and AP status on console
 */
void OnWiFiEvent(WiFiEvent_t event) {
  switch (event) {
  case SYSTEM_EVENT_STA_GOT_IP:
    Serial.println("\nSTA Got WiFi IP");
    break;
    
  case SYSTEM_EVENT_STA_CONNECTED:
    Serial.println("\nSTA connected to WiFi Network");
    break;
    
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.println("\nSTA disconnected from WiFi Network");
    
    //BLINK LED FEEDBACK: Wi-Fi disconnected BLINKS
    for(int i = 0; i < 15; i++)
    {
      digitalWrite(ledPin1, HIGH);
      delay(500);
      digitalWrite(ledPin1, LOW);
      delay(200);
    }

    break;
    
  case SYSTEM_EVENT_AP_START: //Start AP
    Serial.println("\nsoft AP started");
    break;
    
  case SYSTEM_EVENT_AP_STACONNECTED: 
    Serial.println("\nsoft AP station connected");
    break;
    
  case SYSTEM_EVENT_AP_STADISCONNECTED:
    Serial.println("\nsoft AP station disconnected");
    break;
    
  default:
    break;
  }
} // end OnWiFiEvent

/*
 * notFound Class 
 * sends a 404 error back to user on browser
 */
void notFound(AsyncWebServerRequest * request) {
  request -> send(404, "text/plain", "Page not found");
}

/* HTML form */
const char * configForm1 = \
"<html>"\

"<style>"\ 
"* { margin:0; }"\
"body {background:url('https://i.imgur.com/gaqdL8o.jpg') repeat #ccc; font-family: 'Open Sans', sans-serif; font-weight:400; }"\
".container{ width:768px; margin:0 auto; }"\
".container {text-align: center;background:white;margin-top:30px;border-radius:5px;border:1px solid #ccc;padding:10px;color:#585858;font-weight:400;font-size:24px;}"\
".formElement { padding:15px 5px; }"\
"button,input,textarea, select { font-family: 'Open Sans', sans-serif; background:#f4f4f4;color:#000;padding:15px;border-radius:5px;border:1px solid #ccc;width:50%;}"\
"button[type=\"button\"], button[type=\"reset\"] {cursor:pointer;border-radius:40px;width:20%;}"\
"button[type=\"button\"]:hover {background:#055C9D;}"\
"button[type=\"reset\"]:hover {background:#808080;}"\
".tablink {background-color: #555;color: white;float: center;border: none;utline: none;cursor: pointer;padding: 14px 16px;font-size: 17px;width: 25%;}"\
"@media screen and (max-width: 768px){.container {width:95%;}}"\
"@media screen and (max-width: 375px){.container {width:90%;}"\
"button[type=\"button\"], button[type=\"reset\"]{cursor:pointer;border-radius:40px;width:25%;}}"\
"@media screen and (max-width: 375px){.container {width:90%;}"\
"button[type=\"button\"], input[type=\"reset\"]{width:30%;}}"\
"</style>"\

"<head>"\
"<title>Vendalite Status and Configuration</title>"\
"<body>"\
"<div class=\"container\">"

"<div id=\"Status\" class=\"tabcontent\">"\
"<br>"\
"<h2>Device status</h2><br>"\
"<br>"\
"&nbsp;&nbsp;Last Action: "; // insert lastAction here

const char * configForm2 = \
  "<br>&nbsp;&nbsp;STA MAC Addr: "; // insert STA MAC here

const char * configForm3 = \
  "<br>&nbsp;&nbsp;STA IP Addr: &nbsp;"; // insert STA IP here

const char * configForm4 = \
  "<br>&nbsp;&nbsp;STA SSID name: "; // insert current STA SSID name text here

const char * configForm5 = \
  "<br>"\
"&nbsp;&nbsp;STA SSID pass: "; // insert current STA SSID pass text here

const char * configForm6 = \
  "<br>"\
"&nbsp;&nbsp;DEVICE ID: " ; // insert current DEVICE ID text here

const char * configForm7 = \
"<br><br>"\
"<form action=\"/gabriel\" method=\"post\">"\
"<input type=\"hidden\" name=\"posttype\" value=\"refresh\">"\
"&nbsp;&nbsp;<button type=\"submit\" value=\"Refresh\">Refresh</button>"\
"</form><br>"\
"<hr>"\
"</div>"\

"<div id=\"Configuration\" class=\"tabcontent\">"\
"<br>"\
"<h2>Configuration</h2><br>"\
"<br>"\
"<form action=\"/gabriel\" method=\"post\">"\
"<input type=\"hidden\" name=\"posttype\" value=\"update\">"\
"<input type=\"text\" name=\"SSIDname\" placeholder=\"STA SSID Name\" maxlength=\"38\" size=\"40\" value=\""; // initialize STA SSID name TextBox here

const char * configForm8 = \
"\"><br>"\
"<input type=\"text\" name=\"SSIDpass\" placeholder=\"STA SSID Password\" maxlength=\"38\" size=\"40\" value=\""; // initialize STA SSID pass TextBox here

const char * configForm9 = \
"\"><br>"\
"<input type=\"text\" name=\"DeviceID\" placeholder=\"Device ID\" maxlength=\"38\" size=\"40\" value=\""; // initialize DEVICE ID TextBox here

const char * configForm10 = \
"\"><br><br>"\
"<button type=\"submit\" value=\"Update\">Update</button>"\

"</form><br>"\
"</div>"\
"<hr>"\

"<div id=\"Restart\" class=\"tabcontent\">"\
"<br>"
"<h2>Restart</h2><br>"
"<br>"\
"&nbsp;&nbsp;Write configuration to memory and reboot"\
"<br>"\
"<br>"\
"<form action=\"/gabriel\" method=\"post\">"\
"<input type=\"hidden\" name=\"posttype\" value=\"restart\">"\
"&nbsp;&nbsp;<button type=\"submit\" value=\"Restart\">Restart</button>"\
"</form>"\
"</div>"
"</div>"\
"</body>"\
"</html>";


// AWS Vendalite (expires Nov 16, 2021) then R3 (expires Sep 15, 2025) then (ISRG Root X1 certificate expires Jun 4, 2035)
const char * rootCACertificate = \
  "-----BEGIN CERTIFICATE-----\n"\
"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\n"\
"-----END CERTIFICATE-----\n";

/* Getting MAC IP address */
char getCharMacIPAddr(char * charMacAddress, char * charIPAddress) {
  if (WiFi.status() == WL_CONNECTED) {
    String sMyMac = WiFi.macAddress();
    IPAddress mySTAIP = WiFi.localIP();
    String sMyIP = String(mySTAIP[0]) + String(".") + String(mySTAIP[1]) + String(".") + String(mySTAIP[2]) + String(".") + String(mySTAIP[3]);
    sMyMac.toCharArray(charMacAddress, (CMACIPMAXLEN - 2));
    sMyIP.toCharArray(charIPAddress, (CMACIPMAXLEN - 2));
  } else {
    strcpy(charMacAddress, "not connected");
    strcpy(charIPAddress, "not connected");
  }
} // end getCharMacAddress()

/* configurates HTML form on ESP Async Web Server */
void configFormCat(void) {
  /*
   * Note that the following str copy and cat are in order
   * The form is divided in configForm1 - 10
   * Each variable storage part of the HTML form
   */
  strcpy(configForm, configForm1); //Status
  strcat(configForm, lastAction);

  strcat(configForm, configForm2); //label STA MAC
  getCharMacIPAddr(cMacAddr, cIPAddr); //get macIP address
  strcat(configForm, cMacAddr);

  strcat(configForm, configForm3); //label STA IP
  strcat(configForm, cIPAddr);

  strcat(configForm, configForm4); //label STA SSID Name
  strcat(configForm, cSSIDname);

  strcat(configForm, configForm5); //label STA SSID Pass
  strcat(configForm, cSSIDpass);

  strcat(configForm, configForm6); //label Device ID
  strcat(configForm, cDeviceID);

  strcat(configForm, configForm7); //text box SSID Name
  strcat(configForm, cSSIDname);

  strcat(configForm, configForm8); //text box SSID Pass
  strcat(configForm, cSSIDpass);

  strcat(configForm, configForm9); //text box DeviceID
  strcat(configForm, cDeviceID);

  strcat(configForm, configForm10); //restart button
}

/* WiFi connection */
void wifiConnection() {
  Serial.print("Waiting for WiFi to connect...");
  //while ((WiFiMulti.run() != WL_CONNECTED))   {
  int iSTAAttempt = 0;
  while ((WiFi.status() != WL_CONNECTED)) {

     for (int i = 0; i < 5; i++)
      {
        digitalWrite(ledPin1, HIGH);
        delay(200);    
        digitalWrite(ledPin1, LOW);
        delay(100);
      }
    
    delay(1000);
    Serial.print(".");
    
    iSTAAttempt++;
    if (iSTAAttempt > MAXSTAATTEMPT) {
      break; // STA failed to connect continue with soft AP only
    }
  } // end while
  
  if (iSTAAttempt > MAXSTAATTEMPT) {
    Serial.println(" STA connection failed continuing with soft AP only");
    while (true) {
      delay(10000); // essentially stop
    }
  } else {
    Serial.println(" STA connected");

    //LED FEEDBACK
    
    Serial.print("Station:  ");
    Serial.print(WiFi.SSID());
    Serial.print(" ");
    Serial.println(WiFi.localIP());
    Serial.print("STA MAC: ");
    Serial.println(WiFi.macAddress());
  } // end if
}

/* PARAM Request class */
void paramRequests() {
  // Send a GET request to <IP>/configform to display if on AP or STA
  server.on("/hello", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (ON_AP_FILTER(request)) {
      Serial.println("AP GET received hello");
      
      request -> send(200, "text/plain", "AP Hello World");
    } else if (ON_STA_FILTER(request)) {
      Serial.println("STA GET received hello");
      request -> send(200, "text/plain", "STA Hello World");
    }
  });

  // Send a GET request to <IP>/configform to display the form page
  server.on("/gabriel", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (ON_AP_FILTER(request)) {
      strcpy(lastAction, "initialized");
      configFormCat();
      request -> send(200, "text/html", configForm);
    } else if (ON_STA_FILTER(request)) {
      request -> send(200, "text/plain", "STA configuration not supported");
    }
  });

  // Send a POST request to <IP>/post with a form field message set to <message>
  server.on("/gabriel", HTTP_POST, [](AsyncWebServerRequest * request) {
    if (ON_AP_FILTER(request)) {
      Serial.println("POST received");
      String message;
      String postmessage = "";
      if (request -> hasParam(PARAM_POSTTYPE, true)) {
        Serial.print("POST posttype ");
        sPosttype = request -> getParam(PARAM_POSTTYPE, true) -> value();
        Serial.println(sPosttype);

        if (sPosttype == "refresh") {
          strcpy(lastAction, "refreshed"); // status refreshing

        } else if (sPosttype == "update") {
          if (request -> hasParam(PARAM_SSIDNAME, true)) {
            Serial.print("POST SSIDname ");
            sSSIDname = request -> getParam(PARAM_SSIDNAME, true) -> value();
            Serial.println(sSSIDname);
            preferences.putString("SSIDname", sSSIDname);
            int sSSIDnameLen = sSSIDname.length() + 1;
            sSSIDname.toCharArray(cSSIDname, sSSIDnameLen);

            message = String(": " + sSSIDname);
            message = String(PARAM_SSIDNAME + message);

            Serial.print("POST message = <");
            Serial.print(message);
            Serial.println(">");
            postmessage = message;
          }
          if (request -> hasParam(PARAM_SSIDPASS, true)) {
            sSSIDpass = request -> getParam(PARAM_SSIDPASS, true) -> value();
            preferences.putString("SSIDpass", sSSIDpass);
            int sSSIDpassLen = sSSIDpass.length() + 1;
            sSSIDpass.toCharArray(cSSIDpass, sSSIDpassLen);

            message = String(": " + sSSIDpass);
            message = String(PARAM_SSIDPASS + message);

            Serial.print("POST message = <");
            Serial.print(message);
            Serial.println(">");
            postmessage += ", " + message;
          }

          if (request -> hasParam(PARAM_DEVICEID, true)) {
            Serial.print("POST DeviceID ");
            sDeviceID = request -> getParam(PARAM_DEVICEID, true) -> value();
            Serial.println(sDeviceID);
            preferences.putString("DeviceID", sDeviceID);

            message = String(": " + sDeviceID);
            message = String(PARAM_DEVICEID + message);

            Serial.print("POST message = <");
            Serial.print(message);
            Serial.println(">");
            Serial.println("Device ID: " + sDeviceID);
            postmessage = message;
          }
          strcpy(lastAction, "updated parameters");

        } else if (sPosttype == "restart") {
          preferences.end();
          Serial.println(" saved parameters and restarting in 10 seconds");
          strcpy(lastAction, "saved parameters and restarting in 10 seconds");
          configFormCat();
          request -> send(200, "text/html", configForm);

          //BLINK LED FEEDBACK
         
          delay(10000);
          
          ESP.restart();
        } // end sPosttype == ?
      } // end if Posttype

      configFormCat();
      request -> send(200, "text/html", configForm);
    } // end if (ON_AP_FILTER)
  });
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();

  preferences.begin("gabriel", false);

  //Getting SSID name, pass and device ID

  //SSID NAME (Wi-Fi Login)
  sSSIDname = preferences.getString("SSIDname", "");
  Serial.print("SSIDname String: ");
  Serial.print(sSSIDname);
  Serial.println();
  int sSSIDnameLen = sSSIDname.length() + 1;
  sSSIDname.toCharArray(cSSIDname, sSSIDnameLen);
  cSSIDname[sSSIDnameLen] = '\0';

  //SSID PASS (Wi-Fi Password)
  sSSIDpass = preferences.getString("SSIDpass", "");
  Serial.print("SSIDpass String: ");
  Serial.print(sSSIDpass);
  Serial.println();
  int sSSIDpassLen = sSSIDpass.length() + 1;
  sSSIDpass.toCharArray(cSSIDpass, sSSIDpassLen);
  cSSIDpass[sSSIDpassLen] = '\0';

  //STA DEVICE ID
  sDeviceID = preferences.getString("DeviceID", "");
  Serial.print("DeviceID String: ");
  Serial.print(sDeviceID);
  Serial.println();
  int sDeviceIDLen = sDeviceID.length() + 1;
  sDeviceID.toCharArray(cDeviceID, sDeviceIDLen);
  cDeviceID[sDeviceIDLen] = '\0';

  int configFormLen = strlen(configForm1) + strlen(configForm2) + strlen(configForm3) + strlen(configForm4) + strlen(configForm5) +
    strlen(configForm6) + strlen(configForm7) + strlen(configForm7) + strlen(configForm7) + 400;

  // CHECKING CONFIG FORMS
  if (configFormLen == (MAXCONFIGFORM - 80)) {
    Serial.println("configFormLen exceeds storage allocated");
    while (true)
      delay(10000); //essentially stop
  }

  configFormCat(); //Calling HTML Form

  WiFi.onEvent(OnWiFiEvent);
  WiFi.mode(WIFI_MODE_APSTA);
  Serial.println("waiting for soft AP to start");

  //crude wait before softAPConfig really should wait for SYSTEM_EVENT_AP_START
  WiFi.softAP(soft_ap_ssid, soft_ap_password, 1, 1);
  delay(200);

  //use the default 192.168.4.1 to avoid any potential issue
  //Serial.print("soft AP:  ");
  Serial.print(WiFi.softAPSSID());
  Serial.print("AP MAC: ");
  Serial.println(WiFi.softAPIP());
  
  Serial.println();
  //Serial.println(WiFi.softAPmacAddress());

  Serial.print("cSSIDname: ");
  Serial.println(cSSIDname);
  Serial.print("cSSIDpass: ");
  Serial.println(cSSIDpass);

  Serial.println();

  WiFi.begin(cSSIDname, cSSIDpass);

  paramRequests(); //calling param requests

  server.onNotFound(notFound);
  server.begin();

  wifiConnection(); // wait for WiFi connection

} // end setup()

void loop() {
  //file found at server
  Serial.println();
  Serial.println("INSIDE THE LOOP");

  if ((WiFi.status() == WL_CONNECTED)) {

    //TURN ON LED while WiFi is connected
    digitalWrite(ledPin1, HIGH);

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

                t_httpUpdate_return ret = httpUpdate.update( * client, "https://vendalite.com/gabriel/api/esp32devicesapi/updatedevice/" + sDeviceID);

                Serial.println("Device ID: " + sDeviceID);

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
                delay(500);
                ledState = LOW;
                digitalWrite(ledPin0, ledState);
                digitalWrite(ledPin1, ledState);
                delay(1000);
              }
              String newurl = "https://vendalite.com/gabriel/api/ESP32DevicesApi/ForESP32Device/" + String(sDeviceID) + "?counter=" + String(ledcheck);
              if (https.begin( * client, newurl)) {
                int httpCode = https.PUT(String(ledcheck));
                Serial.println(newurl);
                Serial.print(httpCode);
              }
            }
          } else {
            Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
          }
        }
      }

      delete client;
      //TURN ON LED while WiFi is connected
      digitalWrite(ledPin1, HIGH);
      
    } else {
      Serial.println("Unable to create client");
    }
  }
  else
  {
    digitalWrite(ledPin1, LOW);  
  }

  Serial.println();
  Serial.println("Waiting 60s before the next round...");
  delay(60000);
}
