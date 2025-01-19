#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// Wi-Fi credentials
const char *ssid = "MIWIFI_2G_w4zf";
const char *password = "TisHQiFG";

// MQTT server credentials
const char *mqtt_server = "484b32396832453bb03efdd2eae842f2.s2.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char *mqtt_user = "hivemq.webclient.1735489924029";
const char *mqtt_password = ".0pzX74%sGnm&!fPCNB2";
char tempToPublish[10];
const char *mqtt_topic_temperature = "sensors/temperature";
const char *mqtt_topic_relative_humidity = "sensors/relative_humidity";

// DHT11 sensor
#define DHTPIN D3
#define DHTTYPE DHT11
DHT_Unified dht(DHTPIN, DHTTYPE);

WiFiClientSecure espClient; // WiFi client with TLS support
PubSubClient client(espClient);

uint32_t delayMS = 5000;

void connectWiFi()
{
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT()
{
  Serial.print("Connecting to MQTT");
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);

  while (!client.connected())
  {
    Serial.print(".");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password))
    {
      Serial.println("\nMQTT connected");
    }
    else
    {
      Serial.print("\nFailed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void publishDHT11_Temp(int temperature)
{
  char tempString[10]; // Buffer para almacenar la temperatura como texto
  itoa(temperature, tempString, 10); // Convierte el número entero a cadena
  if (client.publish(mqtt_topic_temperature, tempString))
  {
    Serial.print("Temperature published successfully: ");
    Serial.println(tempString);
  }
  else
  {
    Serial.println("Failed to publish temperature");
  }
}

void publishDHT11_Relative_Hum(int relative_hum)
{
  char relative_humString[10]; // Buffer para almacenar la temperatura como texto
  itoa(relative_hum, relative_humString, 10); // Convierte el número entero a cadena
  if (client.publish(mqtt_topic_relative_humidity, relative_humString))
  {
    Serial.print("Relative humidity published successfully: ");
    Serial.println(relative_humString);
  }
  else
  {
    Serial.println("Failed to publish Relative humidity");
  }
}

void readDHT11()
{
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (!isnan(event.temperature))
  {
    int temperature = round(event.temperature);
    Serial.print("Temperature: ");
    Serial.println(temperature);
    publishDHT11_Temp(temperature);
  }
  else
  {
    Serial.println("Error reading temperature");
  }

  dht.humidity().getEvent(&event);
  if (!isnan(event.relative_humidity))
  {
    Serial.print("Humidity: ");
    Serial.print(event.relative_humidity);
    Serial.println("%");
    publishDHT11_Relative_Hum(event.relative_humidity);
  }
  else
  {
    Serial.println("Error reading humidity");
  }
}

void readMoisture()
{
  Serial.print("Pincho de tierra: ");
  Serial.println(analogRead(A0));
}

void setup()
{
  Serial.begin(115200);
  dht.begin();

  connectWiFi();
  connectMQTT();
}

void loop()
{
  if (!client.connected())
  {
    connectMQTT();
  }
  client.loop();

  readDHT11();
  readMoisture();
  delay(delayMS);
}
