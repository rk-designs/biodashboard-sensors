#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN D3
#define DHTTYPE DHT11

DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

void setup()
{
  Serial.begin(115200);
  dht.begin();
  delayMS = 5000;
}

void sensor_DHT11()
{
  sensors_event_t event;
  dht.temperature().getEvent(&event);

  if (isnan(event.temperature))
  {
    Serial.println(F("Error reading temperature!"));
  }
  else
  {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
  }

  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity))
  {
    Serial.println(F("Error reading humidity!"));
  }
  else
  {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
  }
}
void sensor_moisture()
{
  Serial.println(F("Pincho de tierra: "));
  Serial.print(analogRead(A0));
}

void loop()
{
  delay(delayMS);
  sensor_DHT11();
  sensor_moisture();
}
