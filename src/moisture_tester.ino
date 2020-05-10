#include <avr/dtostrf.h>
#include "secrets.h"
#include "pitches.h"
#include <WiFi101.h>
#include <WiFiClient.h>
#include <ArduinoLowPower.h>
#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>

// Comment to disable debug serial output
//#define DEBUG

#ifdef DEBUG
#define DPRINT(...) Serial.print(__VA_ARGS__)
#define DPRINTLN(...) Serial.println(__VA_ARGS__)
#define DPRINTF(...) Serial.print(F(__VA_ARGS__))
#define DPRINTLNF(...) Serial.println(F(__VA_ARGS__))
#define SLEEP_INTERVAL 10000

#else
#define DPRINT(...)            //blank line
#define DPRINTLN(...)          //blank line
#define DPRINTF(...)           //blank line
#define DPRINTLNF(...)         //blank line
#define SLEEP_INTERVAL 3600000 // 1 hour
#endif

#define ANALOG_MAX 4096L // maximum read value
#define VOLTAGE 3.3      // operating voltage for board

const char id[] = SECRET_ID;
const char ssid[] = SECRET_SSID;
const char password[] = SECRET_PWD;
const char broker[] = SECRET_BROKER;
const char topic[] = SECRET_TOPIC;
const char *certificate = SECRET_CERTIFICATE;

const int lightPin = A0;
const int temperaturePin = A1;
const int moisturePin = A2;
const int outputPin = 6;
const int buzzerPin = 5;

WiFiClient wifiClient;
BearSSLClient sslClient(wifiClient);
MqttClient mqttClient(sslClient);
bool awake = true;

void setup()
{
// We have no regular serial output.
#ifdef DEBUG
  Serial.begin(9600);
  while (!Serial)
    ;
#endif

  if (!ECCX08.begin())
  {
    DPRINTLNF("No ECCX08 present!");
    while (1)
      ;
  }

  // Max resolution supported by MKR family boards
  analogReadResolution(12);

  ArduinoBearSSL.onGetTime(getTime);
  sslClient.setEccSlot(0, certificate);
  mqttClient.setId(F("kasvikamu-1"));
  // mqttClient.onMessage(onMessageReceived);

  LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, wakeUp, CHANGE);

  pinMode(outputPin, OUTPUT);
  delay(2000);
}

void loop()
{
  if (!awake)
  {
    return;
  }

  digitalWrite(outputPin, HIGH);
  delay(500);
  digitalWrite(outputPin, LOW);
  delay(500);

  beep(NOTE_C6, 8);

  if (WiFi.status() != WL_CONNECTED)
  {
    connectWiFi();
  }

  if (!mqttClient.connected())
  {
    connectMQTT();
  }

  mqttClient.poll();
  readAndPublish();

#ifdef DEBUG
  // Detach / reattach USB device for Windows 10
  // to be able to read serial input
  USBDevice.detach();
#endif

  mqttClient.stop();
  WiFi.end();

  awake = false;
  LowPower.deepSleep(SLEEP_INTERVAL);

#ifdef DEBUG
  USBDevice.attach();
  while (!Serial)
    ;
#endif

  // Some wake up time
  delay(2000);
  DPRINTLN();
}

void readAndPublish()
{
  // Buffers to contain response
  char tbs[80];
  char strTemp[3][6];

  // Get readings as {2}.{2} decimal strings
  dtostrf(getAverageMoisture(), 4, 2, strTemp[0]);
  dtostrf(getTemperature(), 4, 2, strTemp[1]);
  dtostrf(getLight(), 6, 2, strTemp[2]);

  sprintf(tbs, "{\"id\":\"%s\",\"tmst\":%d,\"mstr\":%s,\"tmpr\":%s,\"lght\":%s}",
          id, getTime(), strTemp[0], strTemp[1], strTemp[2]);

  publishMessage(tbs);

  DPRINTF(topic);
  DPRINTF(": ");
  DPRINTLN(tbs);
}

float getAverageMoisture()
{
  // make an average of 10 values to be more accurate
  analogRead(moisturePin); // initial dummy read
  int tempValue = 0;       // variable to temporarily store moisture value
  for (int a = 0; a < 10; a++)
  {
    tempValue += analogRead(moisturePin);
    delay(100);
  }
  // scaling factor of 100 gives results as percents of maximum
  return ((ANALOG_MAX - ((float)tempValue / 10)) / ANALOG_MAX) * 100;
}

float getTemperature()
{
  // dummy read for more accuracy
  int reading = analogRead(temperaturePin);
  reading = analogRead(temperaturePin);

  float voltage = reading * VOLTAGE / ANALOG_MAX;
  // Output temperature as Celsius
  float temperatureC = (voltage - 0.5) * 100; // converting from 10 mV per degree with 500 mV offset
  return temperatureC;
}

float getLight()
{
  // dummy read for more accuracy
  int lightValue = analogRead(lightPin);
  lightValue = analogRead(lightPin);
  // scaling factor of 100 gives result as percents of maximum
  return ((float)lightValue * 100) / ANALOG_MAX;
}

unsigned long getTime()
{
  // get the current time from the WiFi module
  return WiFi.getTime();
}

void connectWiFi()
{
  DPRINTF("Connecting Wifi: ");
  DPRINT(ssid);

  while (WiFi.begin(ssid, password) != WL_CONNECTED)
  {
    DPRINTF(".");
    delay(2000);
  }

  DPRINTLN();
  DPRINTLNF("WiFi connected.");
}

void connectMQTT()
{
  DPRINTF("Connecting MQTT: ");
  DPRINT(broker);

  while (!mqttClient.connect(broker, 8883))
  {
    DPRINTF(".");
    delay(2000);
  }

  DPRINTLN();
  DPRINTLNF("MQTT connected.");

  // mqttClient.subscribe("kasvikamu/incoming");
}

void publishMessage(char *message)
{
  mqttClient.beginMessage(topic);
  mqttClient.print(message);
  mqttClient.endMessage();
}

void wakeUp()
{
  awake = true;
}

/*
 * The Purkki jingle
 */
void playJingle()
{
  const int tempo = 3000; // 1 bar in milliseconds
  beep(NOTE_E5, 16);
  beep(NOTE_G5, 16);
  beep(NOTE_A5, 8);
  beep(NOTE_A5, 16);
  beep(NOTE_G5, 16);
  beep(NOTE_A5, 16);
  delay(tempo / 16);
  beep(NOTE_E5, 8);
  delay(tempo / 4);
  delay(tempo / 8);
  beep(NOTE_E5, 16);
  beep(NOTE_G5, 16);
  beep(NOTE_A5, 8);
  beep(NOTE_A5, 16);
  beep(NOTE_G5, 16);
  beep(NOTE_A5, 16);
  delay(tempo / 16);
  beep(NOTE_C6, 8);
}
void beep(int note, int duration)
{
  const int tempo = 3000;              // 1 bar in milliseconds
  int noteDuration = tempo / duration; // floored, int division
  tone(buzzerPin, note, noteDuration);
  delay(noteDuration * 1.30);
  noTone(buzzerPin);
}
