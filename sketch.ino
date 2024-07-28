#include <WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>

// Konfigurasi Wi-Fi
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "public.mqtthq.com"; 
const char* username = "";
const char* pass = "";

// Pin untuk tiga LED
int ldrPin = 36;        // Pin untuk sensor LDR
int ledPin1 = 19;       // Pin untuk LED 1
int ledPin2 = 18;       // Pin untuk LED 2
int ledPin3 = 5;       // Pin untuk LED 3
int Relay_Fan = 17;
int Relay_Purifier = 16;
int manualswitch;
#define DHTPIN 12    // GPIO pin where the DHT22 is connected
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
const float GAMMA = 0.7;
const float RL10 = 50;
int maxPWM = 255;       // Nilai maksimum PWM (8-bit)
int minLux = 250;       // Minimum lux yang diinginkan
int maxLux = 300;       // Maksimum lux yang diinginkan

char str_hum_data[10];
char str_temp_data[10];
char str_lux[10];
char str_totallux_lampu [10];

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String messageLamp;
  String messagekontrol;
  String messageFan;
  String MessagePurifier;
  
  if(strstr(topic, "ruangkelas_arief/kendali")){
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      messagekontrol += (char)payload[i];
    }
    if(messagekontrol == "auto"){
      manualswitch = 0;
    }
    else if (messagekontrol == "manual"){
      manualswitch = 1;
    }
  }
  
  if (manualswitch == 1) {
    if (strstr(topic, "ruangkelas_arief/lamp1")) {
        for (int i = 0; i < length; i++) {
          Serial.print((char)payload[i]);
          messageLamp += (char)payload[i];
        }
        if (messageLamp == "On") {
          digitalWrite(ledPin1, HIGH);
        }
        else if (messageLamp == "Off") {
          digitalWrite(ledPin1, LOW);
        }
    }
    if (strstr(topic, "ruangkelas_arief/lamp2")) {
        for (int i = 0; i < length; i++) {
          Serial.print((char)payload[i]);
          messageLamp += (char)payload[i];
        }
        if (messageLamp == "On") {
          digitalWrite(ledPin2, HIGH);
        }
        else if (messageLamp == "Off") {
          digitalWrite(ledPin2, LOW);
        }
    }
    if (strstr(topic, "ruangkelas_arief/lamp3")) {
        for (int i = 0; i < length; i++) {
          Serial.print((char)payload[i]);
          messageLamp += (char)payload[i];
        }
        if (messageLamp == "On") {
          digitalWrite(ledPin3, HIGH);
        }
        else if (messageLamp == "Off") {
          digitalWrite(ledPin3, LOW);
        }
    }
    if (strstr(topic, "ruangkelas_arief/fan")) {
        for (int i = 0; i < length; i++) {
          Serial.print((char)payload[i]);
          messageFan += (char)payload[i];
        }
        if (messageFan == "On") {
          digitalWrite(Relay_Fan, HIGH);
        }
        else if (messageFan == "Off") {
          digitalWrite(Relay_Fan, LOW);
        }
    }
    if (strstr(topic, "ruangkelas_arief/purifier")) {
        for (int i = 0; i < length; i++) {
          Serial.print((char)payload[i]);
          MessagePurifier += (char)payload[i];
        }
        if (MessagePurifier == "On") {
          digitalWrite(Relay_Purifier, HIGH);
        }
        else if (MessagePurifier == "Off") {
          digitalWrite(Relay_Purifier, LOW);
        }
   }    
    Serial.println();
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), username, pass)) {
      Serial.println("connected");
      client.subscribe("ruangkelas_arief/kendali");
      client.subscribe("ruangkelas_arief/lamp1");
      client.subscribe("ruangkelas_arief/lamp2");
      client.subscribe("ruangkelas_arief/lamp3");
      client.subscribe("ruangkelas_arief/fan");
      client.subscribe("ruangkelas_arief/purifier");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(ldrPin, INPUT);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(Relay_Fan, OUTPUT);
  pinMode(Relay_Purifier, OUTPUT);
  
  // Koneksi ke Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();

  if(now - lastMsg > 2000){
    float temp_data = dht.readTemperature();
    dtostrf(temp_data, 4, 2, str_temp_data);
    float hum_data = dht.readHumidity();
    dtostrf(hum_data, 4, 2, str_hum_data);
    client.publish("ruangkelas_arief/sensor/temperature",str_temp_data);
    client.publish("ruangkelas_arief/sensor/humidity", str_hum_data);
    Serial.println("");
    Serial.println("-------------------------------");
    Serial.println("Publish message: ");
    Serial.print("Temperature: ");
    Serial.print(temp_data);
    Serial.print(" °C, Humidity: ");
    Serial.print(hum_data);
    Serial.println(" %");
    Serial.println("-------------------------------");
    

    int ldrValue = analogRead(ldrPin); // Membaca nilai LDR (0-4095)
    float voltage = ldrValue / 4095.0 * 5.0;
    float resistance = 2000 * voltage / (1 - voltage / 5.0);
    float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));
    //int lux = map(ldrValue, 0, 4095, 0, 1000); // Perkiraan nilai lux (sesuaikan sesuai dengan sensor LDR Anda)
    
    
    dtostrf(lux, 4, 2, str_lux);
    client.publish("ruangkelas_arief/sensor/luxruangan",str_lux);

    Serial.print("LDR Value : ");
    Serial.println(ldrValue);
    Serial.print("Lux : ");
    Serial.println(lux);
    
    int total=0;
    int totalBrightness = 0;
    
    if(manualswitch == 0){
      if (lux < minLux) {
        int suplai = 300-lux;
        Serial.print("Lux yang dibutuhkan : ");
        Serial.println(suplai);
        total=suplai/3;
        analogWrite(ledPin1,total);
        analogWrite(ledPin2,total);
        analogWrite(ledPin3,total);
        Serial.print("Lux per lampu : ");
        Serial.println(total);
        dtostrf(lux, 4, 2, str_totallux_lampu);
        client.publish("ruangkelas_arief/sensor/luxlampu",str_totallux_lampu);
      } 
      else{
        Serial.println("Lux is above maximum. Turning LEDs off.");
        analogWrite(ledPin1, 0);
        analogWrite(ledPin2, 0);
        analogWrite(ledPin3, 0);
        client.publish("ruangkelas_arief/lamp", "LEDs Off");
      }
      Serial.println("-------------------------------");
    
      if(temp_data >= 38){
        digitalWrite(Relay_Fan, HIGH);
        Serial.println("Fan Menyala");
        client.publish("ruangkelas_arief/fan", "On");
      }
      else{
        digitalWrite(Relay_Fan,LOW);
        Serial.println("Fan Mati");
        client.publish("ruangkelas_arief/fan", "Off");
      }
      if(hum_data <= 40){
        digitalWrite(Relay_Purifier, HIGH);
        Serial.println("Purifier Menyala");
        client.publish("ruangkelas_arief/purifier", "On");
      }
      else{
        digitalWrite(Relay_Purifier, LOW);
        Serial.println("Purifier Mati");
        client.publish("ruangkelas_arief/purifier", "Off");
      }
    }
    lastMsg = now;
  }
}
