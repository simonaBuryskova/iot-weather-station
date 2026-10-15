#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <HTTPClient.h>
#include <WiFi.h>

const uint64_t uS_TO_S = 1000000;
const int TIME_OFF = 900; // doba "spánku" (sekundy)
const int PIN_SOIL = 32;  // číslo pinu, kde je připojený senzor vlhkosti půdy
const int SOIL_DRY = 2500;
const int SOIL_WET = 1300;
Adafruit_BME280 bme; // objekt bme pro řízení BME280 senzoru, bme je objekt třídy Adafruit..., kterou obsahuje knihovna Adafruit...h
const char* WIFI_SSID = "";
const char* WIFI_PASSW = "";


void setup() {
  Serial.begin(115200);
  delay(100);
  while (!Serial) delay(10); // dokud se linka Serial nezapne, čekej 10 ms a pak to znova zkontroluj

  Serial.println("Meteostation starting");

  // SENZORY
  if (!bme.begin(0x76)) { // zapnutí senzoru, voláme ho na adrese 0x76 (tovární), dokud se spojení nepodaří
    Serial.println("Error: sensor bme280 not found");
    while (1) delay(10);
  }

  Serial.println("Sensor BME280 is ON");
  
  // WIFI
  Serial.printf("Connecting to Wi-Fi: %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID,WIFI_PASSW);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected.");
  Serial.print("IP of meteostation: ");
  Serial.println(WiFi.localIP());

  // DATA
  float temperature = bme.readTemperature();
  float moisture = bme.readHumidity();
  float pressure =  bme.readPressure() / 100.0F; // chceme hPa
  int soilCapacity = analogRead(PIN_SOIL);
  int soilPercent = map(soilCapacity, SOIL_DRY, SOIL_WET, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);

  Serial.printf("Temperature: %.2f °C\n", temperature);
  Serial.printf("Humidity: %.2f %%\n", moisture);
  Serial.printf("Pressure: %.2f hPa\n", pressure);
  Serial.printf("Moisture raw: %d | Moisture percent: %d %%\n", soilCapacity, soilPercent);

  String url = "";
  url += "&field1=" + String(temperature);
  url += "&field2=" + String(moisture);
  url += "&field3=" + String(pressure);
  url += "&field4=" + String(soilPercent);
  Serial.println("Sending data to ThingSpeak.");
  HTTPClient http;
  http.begin(url);
  int answer = http.GET();
  if (answer > 0) {
    Serial.printf("Data saved, code: %d\n", answer);}
    else {
      Serial.printf("error: %d\n", answer);
    }
  http.end();  
  
  delay(2000);

  Serial.printf("All done, time for sleep for %d s.\n", TIME_OFF);
  esp_sleep_enable_timer_wakeup(TIME_OFF * uS_TO_S); // chceme mikrosekundy
  esp_deep_sleep_start();
}

void loop() {}
