

//"Librairies utilisées"
#include <Arduino.h>
#include "DHT.h"
#include <ESP8266WiFi.h>
#include <Ticker.h>

#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

//"Identifiant et mot de passe utilisé pour créer le réseau"
#define WIFI_SSID "Lain"
#define WIFI_PASSWORD "eil123@123"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDmdKHjCLh7Ds3IRTbpJCULI_rIBIxV4II"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://iot-eilco-default-rtdb.europe-west1.firebasedatabase.app"

#define USER_EMAIL "esp1@gmail.com"
#define USER_PASSWORD "esp12023"


//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

// Digital pin connected to the DHT sensor
#define DHTPIN 4

// Uncomment whatever DHT sensor type you're using
#define DHTTYPE DHT11   // DHT 11


// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Variables to hold sensor readings
float temp;
float hum;


WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 1000;        // Interval at which to publish sensor readings

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  wifiReconnectTimer.once(2, connectToWifi);
}



void setup() {
  Serial.begin(115200);
  Serial.println();

  dht.begin();

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  connectToWifi();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

   /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  unsigned long currentMillis = millis();


  if (Firebase.ready() && (millis() - sendDataPrevMillis > 10000 || sendDataPrevMillis == 0)) {
    hum = dht.readHumidity();
    // Read temperature as Celsius (the default)
    temp = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //temp = dht.readTemperature(true);
    sendDataPrevMillis = millis();
    Serial.printf("Température :%.2f\n", temp);
    Serial.printf("Humidité :%.2f\n", hum);
    // Write an Int number on the database path test/int
    if (Firebase.RTDB.setFloat(&fbdo, "live_record/sensor1/hum", hum)) {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    // Write an Float number on the database path test/float
    if (Firebase.RTDB.setFloat(&fbdo, "live_record/sensor1/temp", temp)) {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}