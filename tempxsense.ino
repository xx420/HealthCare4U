#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const char *ssid = "KHAN";
const char *password = "12345678";

WebServer server(80);
DHT dht(26, DHT22);
OneWire oneWire(5);  // DS18B20 connected to Pin D4
DallasTemperature sensors(&oneWire);

float dhtTemperature;
float dhtHumidity;
float ds18b20Temperature;

void handleRoot() {
  char msg[1500];

  // Check if DS18B20 temperature is above 27 degrees Celsius
  bool urgentAttention = ds18b20Temperature > 27.0;

  snprintf(msg, 1500,
           "<html>\
  <head>\
    <meta http-equiv='refresh' content='4'/>\
    <meta name='viewport' content='width=device-width, initial-scale=1'>\
    <link rel='stylesheet' href='https://use.fontawesome.com/releases/v5.7.2/css/all.css' integrity='sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr' crossorigin='anonymous'>\
    <title>ESP32 Sensor Readings</title>\
    <style>\
    html { font-family: Arial; display: inline-block; margin: 0px auto; text-align: center;}\
    h2 { font-size: 3.0rem; }\
    p { font-size: 3.0rem; }\
    .units { font-size: 1.2rem; }\
    .dht-labels{ font-size: 1.5rem; vertical-align:middle; padding-bottom: 15px;}\
    </style>\
    <script>\
      function checkTemperature() {\
        if (%s) {\
          alert('Seek Medical Attention Urgently!');\
        }\
      }\
      window.onload = checkTemperature;\
    </script>\
  </head>\
  <body>\
      <h2>ESP32 Sensor Readings!</h2>\
      <p>\
        <i class='fas fa-thermometer-half' style='color:#ca3517;'></i>\
        <span class='dht-labels'>DHT22 Temperature</span>\
        <span>%.2f</span>\
        <sup class='units'>&deg;C</sup>\
      </p>\
      <p>\
        <i class='fas fa-tint' style='color:#00add6;'></i>\
        <span class='dht-labels'>DHT22 Humidity</span>\
        <span>%.2f</span>\
        <sup class='units'>&percnt;</sup>\
      </p>\
      <p>\
        <i class='fas fa-thermometer-half' style='color:#ca3517;'></i>\
        <span class='dht-labels'>DS18B20 Temperature</span>\
        <span>%.2f</span>\
        <sup class='units'>&deg;C</sup>\
      </p>\
  </body>\
</html>",
           urgentAttention ? "true" : "false", dhtTemperature, dhtHumidity, ds18b20Temperature
          );
  server.send(200, "text/html", msg);
}

void setup(void) {
  Serial.begin(115200);
  dht.begin();
  sensors.begin();
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  server.on("/", handleRoot);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  dhtTemperature = readDHTTemperature();
  dhtHumidity = readDHTHumidity();

  sensors.requestTemperatures();
  ds18b20Temperature = sensors.getTempCByIndex(0);

  server.handleClient();
  delay(2000); // Allow the CPU to switch to other tasks
}

float readDHTTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  }
  else {
    Serial.println("DHT Temperature: " + String(t));
    return t;
  }
}

float readDHTHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  }
  else {
    Serial.println("DHT Humidity: " + String(h));
    return h;
  }
}
