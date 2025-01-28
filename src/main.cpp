// Libraries
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

// Sync application switches
const char *mqtt_topic_switches_sync = "core/sync";

// DHT11 sensor
#define DHTPIN D3
#define DHTTYPE DHT11
DHT_Unified dht(DHTPIN, DHTTYPE);
const char *mqtt_topic_temperature = "sensors/temperature";
const char *mqtt_topic_relative_humidity = "sensors/relative_humidity";
const char *mqtt_topic_switch_dht11 = "switch/dht11";
const char *DHT11State = "on";

// Soil moisture sensor
#define MOISTURE_PIN A0
const char *mqtt_topic_soil_moisture = "sensors/soil_moisture";
const char *mqtt_topic_switch_soil_moisture = "switch/soil_moisture";
const char *SoilMoistureState = "on";

// Irrigation Relay sensor
#define RELAY_PIN D5
const char *mqtt_topic_relay_irrigation = "relay/irrigation";

// WiFi client with TLS support
WiFiClientSecure espClient;
PubSubClient client(espClient);

// Global delay
uint32_t delayMS = 5000;

// Function - Callback
void handleMQTTMessage(char *topic, byte *payload, unsigned int length)
{
  payload[length] = '\0';
  String message = String((char *)payload);

  Serial.print("Message received on topic ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(message);

  String relay_irrigation = digitalRead(RELAY_PIN) == LOW ? "on" : "off";

  // Handle application switches sync
  if (String(topic) == mqtt_topic_switches_sync)
  {
    if (message.equalsIgnoreCase("sync"))
    {
      client.publish(mqtt_topic_relay_irrigation, relay_irrigation.c_str());
      client.publish(mqtt_topic_switch_dht11, DHT11State);
      client.publish(mqtt_topic_switch_soil_moisture, SoilMoistureState);
    }
    else
    {
      Serial.println("Invalid command for switches sync");
    }
  }

  // Handle relay irrigation ON/OFF control
  if (String(topic) == mqtt_topic_relay_irrigation)
  {
    if (message.equalsIgnoreCase("on"))
    {
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("Relay turned ON");
    }
    else if (message.equalsIgnoreCase("off"))
    {
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Relay turned OFF");
    }
    else
    {
      Serial.println("Invalid command for relay");
    }
  }

  // Handle DHT11 ON/OFF control
  if (String(topic) == mqtt_topic_switch_dht11)
  {
    if (message.equalsIgnoreCase("on"))
    {
      DHT11State = "on";
      Serial.println("DHT11 turned ON");
    }
    else if (message.equalsIgnoreCase("off"))
    {
      DHT11State = "off";
      Serial.println("DHT11 turned OFF");
    }
    else
    {
      Serial.println("Invalid command for DHT11");
    }
  }

  // Handle Soil moisture ON/OFF control
  if (String(topic) == mqtt_topic_switch_soil_moisture)
  {
    if (message.equalsIgnoreCase("on"))
    {
      SoilMoistureState = "on";
      Serial.println("Soil moisture turned ON");
    }
    else if (message.equalsIgnoreCase("off"))
    {
      SoilMoistureState = "off";
      Serial.println("Soil moisture turned OFF");
    }
    else
    {
      Serial.println("Invalid command for Soil moisture");
    }
  }
}

// Functions - Subscriptions
void subscriptions()
{
  client.subscribe(mqtt_topic_switches_sync);
  client.subscribe(mqtt_topic_relay_irrigation);
  client.subscribe(mqtt_topic_switch_dht11);
  client.subscribe(mqtt_topic_switch_soil_moisture);
}

// Functions - Wifi connect
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
  client.setCallback(handleMQTTMessage);

  while (!client.connected())
  {
    Serial.print(".");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password))
    {
      Serial.println("\nMQTT connected");
      subscriptions();
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

// Functions - Publish
void publishDHT11_Temp(int temperature)
{
  char tempString[10];
  itoa(temperature, tempString, 10);
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
  char relative_humString[10];
  itoa(relative_hum, relative_humString, 10);
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

void publish_soil_moisture(int humidityPercentage)
{
  char humiditPercentageString[10];
  itoa(humidityPercentage, humiditPercentageString, 10);
  if (client.publish(mqtt_topic_soil_moisture, humiditPercentageString))
  {
    Serial.print("Soil humidity published successfully: ");
    Serial.println(humiditPercentageString);
  }
  else
  {
    Serial.println("Failed to publish temperature");
  }
}

// Functions - Read
void readDHT11()
{
  if (strcmp(DHT11State, "on") == 0)
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
  else
  {
    publishDHT11_Temp(0);
    publishDHT11_Relative_Hum(0);
  }
}

void read_soil_moisture()
{
  if (strcmp(SoilMoistureState, "on") == 0)
  {
    if (!isnan(analogRead(MOISTURE_PIN)))
    {
      int humidityPercentage = map(analogRead(MOISTURE_PIN), 0, 1023, 0, 100);
      Serial.print("Soil moisture value: ");
      Serial.print(analogRead(MOISTURE_PIN));
      Serial.print(" / ");
      Serial.print("Percentage: ");
      Serial.print(humidityPercentage);
      Serial.println("%");
      publish_soil_moisture(humidityPercentage);
    }
    else
    {
      Serial.println("Error reading soil moisture");
    }
  }
  else
  {
    int humidityPercentage = 0;
    publish_soil_moisture(humidityPercentage);
  }
}

void setup()
{
  Serial.begin(115200);
  dht.begin();

  connectWiFi();
  connectMQTT();

  // Pins direction
  pinMode(MOISTURE_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  // Relay initial mode
  digitalWrite(RELAY_PIN, HIGH); // OFF
  digitalWrite(DHTPIN, LOW);     // ON

  // First default sync
  client.publish(mqtt_topic_switch_dht11, DHT11State);
  client.publish(mqtt_topic_switch_soil_moisture, SoilMoistureState);
}

void loop()
{
  if (!client.connected())
  {
    connectMQTT();
  }
  client.loop();

  static unsigned long lastReadTime = 0;
  if (millis() - lastReadTime >= delayMS)
  {
    lastReadTime = millis();
    readDHT11();
    read_soil_moisture();
    Serial.println("-----------------------------------------------------");
  }
}
