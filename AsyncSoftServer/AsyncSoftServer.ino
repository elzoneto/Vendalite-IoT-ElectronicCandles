#include <ESPAsyncWebServer.h>
#include <WiFi.h>

// Turn ESP to Local Server
// IP Address: 192.168.4.1
const char *soft_ap_ssid = "ESPLocal";
const char *soft_ap_password = "password";

AsyncWebServer server(80); // Create AsyncWebServer object on port 80

const char* PARAM_MESSAGE = "message";
const char* PARAM_RESTART = "restart";
//const char* PARAM_SSIDNAME = "SSIDname";
//const char* PARAM_SSIDPASS = "SSIDpass";

// If Page not found
void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

//HTML
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<html>
    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
        <style>
            * { margin:0; }
            body {
            background:url('https://i.imgur.com/gaqdL8o.jpg') repeat #ccc;
            font-family: 'Open Sans', sans-serif;
            font-weight:400;
            }
            a {
            text-decoration:none;
            color:white;
            }
            .b-tab {
              display: none;
            }
            .b-tab.active {
              display: block;
            }
            .container, nav#mainNav {
            width:768px;
            margin:0 auto;
            }
            div.navBar, input[type="submit"] {
            background::#50a5ed;
            background:linear-gradient(#50a5ed, #3280c2);
            }
            #mainNav ul li {
            display:inline;
            }
            #mainNav ul li a {
            padding:15px 0;
            margin-right:50px;
            display:inline-block;
            text-align:center;
            }
            div.container {
            text-align: center;
            background:white;
            margin-top:30px;
            border-radius:5px;
            border:1px solid #ccc;
            padding:10px;
            color:#585858;
            font-weight:400;
            font-size:24px;
            }
            .formElement { padding:15px 5px; }
            input,textarea, select {
            font-family: 'Open Sans', sans-serif;
            background:#f4f4f4;
            color:#000;
            padding:15px;
            border-radius:5px;
            border:1px solid #ccc;
            width:50%;
            }
            input[type="submit"] {
            cursor:pointer;
            border-radius:40px;
            width:20%;
            }
            @media screen and (max-width: 768px){
            nav#mainNav, div.container {
            width:95%;
            }
            }
            @media screen and (max-width: 375px){
            nav#mainNav, div.container {
            width:90%;
            }
            button {
            cursor:pointer;
            border-radius:40px;
            width:25%;
            }
            }
            @media screen and (max-width: 375px){
            nav#mainNav, div.container {
            width:90%;
            }
            input[type="submit"] {
            width:30%;
            }
            }
        </style>
    </head>
    <body>
        <div class="navBar">
            <nav id="mainNav">
                <ul>
                    <li><a href="#" data-tab="Home" class="b-nav-tab active">Home</a></li>
                    <li><a href="#" data-tab="Settings" class="b-nav-tab">Settings</a></li>
                </ul>
            </nav>
        </div>

        <div class="container">
            <form id="Home" class="b-tab active">
                <h1> Device Set Up Menu </h1>
                <br/>
                <div class="formElement">
                    <input type="text" placeholder="Device ID" id="appName">
                </div>
                <div class="formElement">
                    <input type="text" placeholder="WiFi Name" id="appName">
                </div>
                <div class="formElement">
                    <input type="text" placeholder="WiFi Password" id="appName">
                </div>
                <div class="formElement">
                    <input type="submit" value="Submit" name="submit">
                    <input type="submit" value="Clear" name="submit">
                </div>
            </form>

            <form id="Settings" class="b-tab">
              <h1> Device Settings Menu </h1>
              <br/>
              <label class="label">Reset to Default</label>
              <div class="formElement">
                  <input type="submit" value="Restart" name="submit" onclick="resetFunction()">
              </div>
              <p id="resetResult"></p>
              <p id="btnResult"></p>
            </form>
        </div>
    </body>
<script>
  function Tabs() {
    var bindAll = function() {
      var menuElements = document.querySelectorAll('[data-tab]');
      for(var i = 0; i < menuElements.length ; i++) {
        menuElements[i].addEventListener('click', change, false);
      }
    }
    var clear = function() {
      var menuElements = document.querySelectorAll('[data-tab]');
      for(var i = 0; i < menuElements.length ; i++) {
        menuElements[i].classList.remove('active');
        var id = menuElements[i].getAttribute('data-tab');
        document.getElementById(id).classList.remove('active');
      }
    }
    var change = function(e) {
      clear();
      e.target.classList.add('active');
      var id = e.currentTarget.getAttribute('data-tab');
      document.getElementById(id).classList.add('active');
    }
    bindAll();
  }
  var connectTabs = new Tabs();

  function resetFunction() {
    var url = window.location.protocol + "//" + window.location.host;
    if (history.pushState) {
      var newurl = window.location.protocol + "//" + window.location.host + window.location.pathname + 'command?restart';
      window.location.href = newurl;
      //window.history.pushState({path:newurl},'',newurl);
    }
    window.location.href = url;
    alert("restarting in 10 seconds")
  }
</script>
</html>)rawliteral";

void setup() {
  Serial.begin(115200);
  
  // Local ESP to a SoftAP
  WiFi.softAP(soft_ap_ssid, soft_ap_password);
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to <IP>/get?message=<message>
  server.on("/command", HTTP_GET, [] (AsyncWebServerRequest *request) {
      String message;
      if (request->hasParam(PARAM_MESSAGE)) {
          message = String(": " + request->getParam(PARAM_MESSAGE)->value());
          message = String(PARAM_MESSAGE + message);
          request->send(200, "text/plain", message);
      }
      else if (request->hasParam(PARAM_RESTART)) {
          //request->send(200, "text/plain", "restarting in 10 seconds");
          //preferences.end();
          delay(10000);
          ESP.restart();
      }
      else {
          message = "No message sent";
      }
      Serial.print("GET message = <");
      Serial.print(message);
      Serial.println(">");
  });

  server.onNotFound(notFound); // Returning not found page
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:

}
