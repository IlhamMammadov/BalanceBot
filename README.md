# BalanceBot

BalanceBot platform consists of Arduino Nano + MPU6050 with DMP on + ESP8266-01.

Nano controls 2 DC motors, gets attitude data from IMU and has serial interface for ESP to get and send data to Webpage.

ESP8266-01 Connects lo local WiFi network, starts webserver. Users can interract with Robot through web interface.

Web interface has 4 motion buttons and 3 fields to change PID values on the go.
