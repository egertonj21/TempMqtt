# TempMqtt
Wee bit of code for a ESP32/External Thermometer

Sends a message containing the current recorded temperature via MQTT to home/sensor/temperature every 30 minutes, or when a control message ("get_temp") is received on home/sensor/control
