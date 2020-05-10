# Arduino Plant communicator
Use your Arduino MKR1000 WiFi board to measures temperature, light and moisture, and sends results to AWS IoT Core MQTT endpoint.
Based on [this example project](https://create.arduino.cc/projecthub/arduino/plant-communicator-7ea06f).

Requires you to [generate device certificate](https://create.arduino.cc/projecthub/Arduino_Genuino/securely-connecting-an-arduino-mkr-wifi-1010-to-aws-iot-core-a9f365) for your board.

##Schematics
TODO: Add schematics

## Estimated battery life
Calculations are for 1000mAh Li-Po battery
|Send interval (h) | Estimated battery life (days) |
|:--|:--|
| 1 | ~6 |
| 2 | ~10 |
| 3 | ~12 |
| 4 | ~14 |
| 5 | ~15 |
| 6 | ~16 |