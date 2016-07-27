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
  
  String webpage = "<html>\n\
<body>\n\
<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js\"></script>\n\
<script type=\"text/javascript\">\n\
var interval;\n\
var duration = 100;\n\
var touch = false;\n\
var Kp = "  + String(Kp)+ ";\n\
var Ki = "  + String(Ki)+ ";\n\
var Kd = "  + String(Kd)+ ";\n\
\n\
window.onload = function() {\n\
  document.getElementById('Kp_value').value = Kp;\n\
  document.getElementById('Kp_slider').value = Kp;\n\
  document.getElementById('Ki_value').value = Ki;\n\
  document.getElementById('Ki_slider').value = Ki;\n\
  document.getElementById('Kd_value').value = Kd;\n\
  document.getElementById('Kd_slider').value = Kd;\n\
};\n\
\n\
\n\
function forward(){\n\
  document.getElementById(\"command\").innerHTML = \"Forward\";\n\
  $.post(\"http://esp001/com\",{com:'f'});\n\
}\n\
function backward(){\n\
  document.getElementById(\"command\").innerHTML = \"Backward\";\n\
  $.post(\"http://esp001/com\",{com:'b'});\n\
}\n\
function left(){\n\
  document.getElementById(\"command\").innerHTML = \"Left\";\n\
  $.post(\"http://esp001/com\",{com:'l'});\n\
}\n\
function right(){\n\
  document.getElementById(\"command\").innerHTML = \"Right\";\n\
  $.post(\"http://esp001/com\",{com:'r'});\n\
}\n\
function stop(){\n\
  document.getElementById(\"command\").innerHTML = \"Stop\";\n\
  $.post(\"http://esp001/com\",{com:'s'});\n\
}\n\
function showKp(val){\n\
  Kp = val;\n\
  document.getElementById('Kp_value').value=Kp;\n\
  document.getElementById('Kp_slider').value=Kp;\n\
}\n\
function showKi(val){\n\
  Ki = val;\n\
  document.getElementById('Ki_value').value=Ki;\n\
  document.getElementById('Ki_slider').value=Ki;\n\
}\n\
function showKd(val){\n\
  Kd = val;\n\
  document.getElementById('Kd_value').value=Kd;\n\
  document.getElementById('Kd_slider').value=Kd;\n\
}\n\
function updatePID(){\n\
  var command = \"p\"+Kp+\"i\"+Ki+\"d\"+Kd;\n\
  $.post(\"http://esp001/com\",{com:command},\n\
  function(data, status){\n\
        updateLocalPID(data);\n\
        });\n\
}\n\
function updateLocalPID(data){\n\
  Kp = parseInt(data.slice(data.search(\"p\")+1,data.search(\"i\")));\n\
  Ki = parseInt(data.slice(data.search(\"i\")+1,data.search(\"d\")));\n\
  Kd = parseInt(data.slice(data.search(\"d\")+1));\n\
  alert(\"PID updated:  Kp: \" + String(Kp) + \"  Ki: \" + String(Ki) + \"  Kd: \" + String(Kd) + \"  !\");\n\
}\n\
</script>\n\
<center><button name='com' value='f' style=\"width: 200px; height:100px;\"  ontouchstart =\"{touch = true; interval = setInterval(forward, duration);}\"\n\
                            ontouchend = \"{touch = false; clearInterval(interval); stop();}\"\n\
                            onmousedown =\"{if(!touch) interval = setInterval(forward, duration);}\"\n\
                            onmouseup = \"{clearInterval(interval); stop();}\"\n\
\n\
        >Forward</button><br><br>\n\
\n\
<button name='com' value='l' style=\"width: 200px; height:100px; margin-right: 30px;\"  ontouchstart =\"{touch = true; interval = setInterval(left, duration);}\"\n\
                                  ontouchend = \"{touch = false; clearInterval(interval); stop();}\"\n\
                                  onmousedown =\"{if(!touch) interval = setInterval(left, duration);}\"\n\
                                  onmouseup = \"{clearInterval(interval); stop();}\"\n\
        >Left</button>\n\
\n\
<button name='com' value='r' style=\"width: 200px; height:100px; margin-left: 30px;\"     ontouchstart =\"{touch = true; interval = setInterval(right, duration);}\"\n\
                                  ontouchend = \"{touch = false; clearInterval(interval); stop();}\"\n\
                                  onmousedown =\"{if(!touch) interval = setInterval(right, duration);}\"\n\
                                  onmouseup = \"{clearInterval(interval); stop();}\"\n\
        >Right</button><br><br>\n\
\n\
<button name='com' value='b' style=\"width: 200px; height:100px;\"            ontouchstart =\"{touch = true; interval = setInterval(backward, duration);}\"\n\
                                  ontouchend = \"{touch = false; clearInterval(interval); stop();}\"\n\
                                  onmousedown =\"{if(!touch) interval = setInterval(backward, duration);}\"\n\
                                  onmouseup = \"{clearInterval(interval); stop();}\"\n\
        >Backward</button><br>\n\
\n\
<p id=\"LastCommand\"> Last command:</p> <p id=\"command\">Stop</p><br>\n\
\n\
Kp  : <input type=\"range\" id=\"Kp_slider\" min=\"0\" max=\"1000\" step=\"1\" onchange=\"showKp(this.value);\">\n\
<input type=\"text\" id=\"Kp_value\" style=\"width:50px\" onchange=\"showKp(this.value);\">\n\
Ki  : <input type=\"range\" id=\"Ki_slider\" min=\"0\" max=\"1000\" step=\"1\" onchange=\"showKi(this.value);\">\n\
<input type=\"text\" id=\"Ki_value\" style=\"width:50px\" onchange=\"showKi(this.value);\">\n\
Kd  : <input type=\"range\" id=\"Kd_slider\" min=\"0\" max=\"1000\" step=\"1\" onchange=\"showKd(this.value);\">\n\
<input type=\"text\" id=\"Kd_value\" style=\"width:50px\" onchange=\"showKd(this.value);\"><br><br>\n\
\n\
<button  style=\"width: 200px; height:100px;\"  onclick =\"updatePID();\">Update</button><br>\n\
\n\
</body>\n\
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
