#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "NIL (:";
const char* password = "NI131719";

ESP8266WebServer server(80);

const int led = 2;

int Kp,Ki,Kd;


void handleRoot() {
  digitalWrite(led, 1);
  //send on serial when command from button press is available
  Serial.print(server.arg("com"));
  
  //Webpage html code:
  
  String webpage = "<html>"
"<body>"
"<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js\"></script>"
"<script type=\"text/javascript\">"
"var interval;"
"var duration = 100;"
"var touch = false;"
"function forward(){document.getElementById(\"command\").innerHTML = \"Forward\"; $.post(\"http://esp001/#\",{com:'f'});}"
"function backward(){ document.getElementById(\"command\").innerHTML = \"Backward\"; $.post(\"http://esp001/#\",{com:'b'});}"
"function left(){document.getElementById(\"command\").innerHTML = \"Left\"; $.post(\"http://esp001/#\",{com:'l'});}"
"function right(){ document.getElementById(\"command\").innerHTML = \"Right\"; $.post(\"http://esp001/#\",{com:'r'});}"
"function stop(){ document.getElementById(\"command\").innerHTML = \"Stop\"; $.post(\"http://esp001/#\",{com:'s'});}"
"</script>"
"<center><button name='com' value='f' style=\"width: 200px; height:150px;\"   ontouchstart =\"{touch = true; interval = setInterval(forward, duration);}\""
"                            ontouchend = \"{touch = false; clearInterval(interval); stop();}\""
"                            onmousedown =\"{if(!touch) interval = setInterval(forward, duration);}\""
"                            onmouseup = \"{clearInterval(interval); stop();}\""
"        >Forward</button><br><br>"
""
"<button name='com' value='l' style=\"width: 200px; height:150px; margin-right: 30px;\"   ontouchstart =\"{touch = true; interval = setInterval(left, duration);}\""
"                                  ontouchend = \"{touch = false; clearInterval(interval); stop();}\""
"                                  onmousedown =\"{if(!touch) interval = setInterval(left, duration);}\""
"                                  onmouseup = \"{clearInterval(interval); stop();}\""
"        >Left</button>"
""
"        "
"<button name='com' value='r' style=\"width: 200px; height:150px; margin-left: 30px;\"    ontouchstart =\"{touch = true; interval = setInterval(right, duration);}\""
"                                  ontouchend = \"{touch = false; clearInterval(interval); stop();}\""
"                                  onmousedown =\"{if(!touch) interval = setInterval(right, duration);}\""
"                                  onmouseup = \"{clearInterval(interval); stop();}\""
"        >Right</button><br><br>"
"        "
"<button name='com' value='b' style=\"width: 200px; height:150px;\"             ontouchstart =\"{touch = true; interval = setInterval(backward, duration);}\""
"                                  ontouchend = \"{touch = false; clearInterval(interval); stop();}\""
"                                  onmousedown =\"{if(!touch) interval = setInterval(backward, duration);}\""
"                                  onmouseup = \"{clearInterval(interval); stop();}\""
"        >Backward</button><br>"
"        "
"<p id=\"LastCommand\"> Last command: <p id=\"command\"></p>"
""
"</body>"
"</html> ";


  server.send(200, "text/html", webpage);
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
  while(!Serial.available()) {Serial.print("."); delay(500);}   //waiting for data from arduino
  while(Serial.available()){
    switch((char)Serial.read()){
      case 'p':
        Kp = Serial.parseInt();
        break;
      case 'i':
        Ki = Serial.parseInt();
        break;
      case 'd':
        Kd = Serial.parseInt();
        break;
      }
  }
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
