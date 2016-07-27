#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "NIL (:";
const char* password = "NI131719";

ESP8266WebServer server(80);

const int led = 2;

int Kp=100,Ki=0,Kd=0;


void handleRoot() {
  digitalWrite(led, 1);
  //send on serial when command from button press is available
  //Serial.print(server.arg("com"));
  
  //Webpage html code:
  
  String webpage = "<html>\
<body>\
<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js\"></script>\
<script type=\"text/javascript\">\
var interval;\
var duration = 200;\
var touch = false;\
var Kp = "  + String(Kp)+ ";\
var Ki = "  + String(Ki)+ ";\
var Kd = "  + String(Kd)+ ";\
\
window.onload = function() {\
  document.getElementById('Kp_value').value = Kp;\
  document.getElementById('Kp_slider').value = Kp;\
  document.getElementById('Ki_value').value = Ki;\
  document.getElementById('Ki_slider').value = Ki;\
  document.getElementById('Kd_value').value = Kd;\
  document.getElementById('Kd_slider').value = Kd;\
};\
\
\
function forward(){\
  document.getElementById(\"command\").innerHTML = \"Forward\";\
  $.post(\"http://esp001/com\",{com:'f'});\
}\
function backward(){\
  document.getElementById(\"command\").innerHTML = \"Backward\";\
  $.post(\"http://esp001/com\",{com:'b'});\
}\
function left(){\
  document.getElementById(\"command\").innerHTML = \"Left\";\
  $.post(\"http://esp001/com\",{com:'l'});\
}\
function right(){\
  document.getElementById(\"command\").innerHTML = \"Right\";\
  $.post(\"http://esp001/com\",{com:'r'});\
}\
function stop(){\
  document.getElementById(\"command\").innerHTML = \"Stop\";\
  $.post(\"http://esp001/com\",{com:'s'});\
}\
function showKp(val){\
  Kp = val;\
  document.getElementById('Kp_value').value=Kp;\
  document.getElementById('Kp_slider').value=Kp;\
}\
function showKi(val){\
  Ki = val;\
  document.getElementById('Ki_value').value=Ki;\
  document.getElementById('Ki_slider').value=Ki;\
}\
function showKd(val){\
  Kd = val;\
  document.getElementById('Kd_value').value=Kd;\
  document.getElementById('Kd_slider').value=Kd;\
}\
function updatePID(){\
  var command = \"p\"+Kp+\"i\"+Ki+\"d\"+Kd;\
  $.post(\"http://esp001/com\",{com:command}, \
  function(data, status){\
        updateLocalPID(data);\
        });\
}\
function updateLocalPID(data){\
  Kp = parseInt(data.slice(data.search(\"p\")+1,data.search(\"i\")));\
  Ki = parseInt(data.slice(data.search(\"i\")+1,data.search(\"d\")));\
  Kd = parseInt(data.slice(data.search(\"d\")+1));\
  alert(\"PID updated:  Kp: \" + String(Kp) + \"  Ki: \" + String(Ki) + \"  Kd: \" + String(Kd) + \"  !\");\
}\
</script>\
<center><button name='com' value='f' style=\"width: 200px; height:100px;\"  ontouchstart =\"{touch = true; interval = setInterval(forward, duration);}\"\
                            ontouchend = \"{touch = false; clearInterval(interval); stop();}\"\
                            onmousedown =\"{if(!touch) interval = setInterval(forward, duration);}\"\
                            onmouseup = \"{clearInterval(interval); stop();}\"\
\
        >Forward</button><br><br>\
\
<button name='com' value='l' style=\"width: 200px; height:100px; margin-right: 30px;\"  ontouchstart =\"{touch = true; interval = setInterval(left, duration);}\"\
                                  ontouchend = \"{touch = false; clearInterval(interval); stop();}\"\
                                  onmousedown =\"{if(!touch) interval = setInterval(left, duration);}\"\
                                  onmouseup = \"{clearInterval(interval); stop();}\"\
        >Left</button>\
\       
<button name='com' value='r' style=\"width: 200px; height:100px; margin-left: 30px;\"     ontouchstart =\"{touch = true; interval = setInterval(right, duration);}\"\
                                  ontouchend = \"{touch = false; clearInterval(interval); stop();}\"\
                                  onmousedown =\"{if(!touch) interval = setInterval(right, duration);}\"\
                                  onmouseup = \"{clearInterval(interval); stop();}\"\
        >Right</button><br><br>\
\
<button name='com' value='b' style=\"width: 200px; height:100px;\"            ontouchstart =\"{touch = true; interval = setInterval(backward, duration);}\"\
                                  ontouchend = \"{touch = false; clearInterval(interval); stop();}\"\
                                  onmousedown =\"{if(!touch) interval = setInterval(backward, duration);}\"\
                                  onmouseup = \"{clearInterval(interval); stop();}\"\
        >Backward</button><br>\
\
<p id=\"LastCommand\"> Last command:</p> <p id=\"command\">Stop</p><br>\
\
Kp  : <input type=\"range\" id=\"Kp_slider\" min=\"0\" max=\"1000\" step=\"1\" onchange=\"showKp(this.value);\">\
<input type=\"text\" id=\"Kp_value\" style=\"width:50px\" onchange=\"showKp(this.value);\">\
Ki  : <input type=\"range\" id=\"Ki_slider\" min=\"0\" max=\"1000\" step=\"1\" onchange=\"showKi(this.value);\">\
<input type=\"text\" id=\"Ki_value\" style=\"width:50px\" onchange=\"showKi(this.value);\">\
Kd  : <input type=\"range\" id=\"Kd_slider\" min=\"0\" max=\"1000\" step=\"1\" onchange=\"showKd(this.value);\">\
<input type=\"text\" id=\"Kd_value\" style=\"width:50px\" onchange=\"showKd(this.value);\"><br><br>\
\
<button  style=\"width: 200px; height:100px;\"  onclick =\"updatePID();\">Update</button><br>\
\
</body>\
</html>";


  server.send(200, "text/html", webpage);
  digitalWrite(led, 0);
}

void handleCOM() {
  digitalWrite(led, 1);
  //send on serial when command from button press is available
  String request =  server.arg("com");
  Serial.println(request);

  //Parse request to save locally
  Kp = request.substring(request.indexOf('p')+1,request.indexOf('i')).toInt();
//  Serial.print("Kp: "); Serial.println(Kp);
  Ki = request.substring(request.indexOf('i')+1,request.indexOf('d')).toInt();
//  Serial.print("Ki: "); Serial.println(Ki);
  Kd = request.substring(request.indexOf('d')+1).toInt();
//  Serial.print("Kd: "); Serial.println(Kd);
  
  //Create response and send:
  String response = "p"+String(Kp)+"i"+String(Ki)+"d"+String(Kd);
//  Serial.println(response);
  
  server.send(200, "text/plain", response);
  digitalWrite(led, 0);
}


void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void){
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  
  //#################################
  //Initialize communication with arduino and get PID values
  Serial.begin(115200);
//  while(!Serial.available()) {Serial.println("."); delay(500);}   //waiting for data from arduino
//  delay(1000);
//  while(Serial.available()){
//    switch((char)Serial.read()){
//      case 'p':
//        Kp = Serial.parseInt();
//        break;
//      case 'i':
//        Ki = Serial.parseInt();
//        break;
//      case 'd':
//        Kd = Serial.parseInt();
//        break;
//      }
//  }
//  Serial.println(Kp);
//  Serial.println(Ki);
//  Serial.println(Kd);
//  Serial.println("PID values initialized!");
  //After getting PID values it possible to continue with this values in webpage as default.
  //#################################

  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/com", handleCOM);
  
  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
}
