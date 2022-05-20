#include <WiFi.h>
#include <PubSubClient.h>

// WiFi
const char *ssid = "SpectrumSetup-9F"; // Enter your WiFi name
const char *password = "monthunion197";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "192.168.1.63";
const char *topic1 = "esp32/LEDS/1";
const char *topic2 = "esp32/LEDS/2";
const char *mqtt_username = "emqx";
const char *mqtt_password = "123456789";
const int mqtt_port = 1883;

// LED Pins
const int LEDOne = 2;
const int LEDTwo = 4;

// initialize espClient
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  pinMode(LEDOne, OUTPUT);
  pinMode(LEDTwo, OUTPUT);
  
 // Set software serial baud to 115200;
 Serial.begin(115200);
 // connecting to a WiFi network
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.println("Connecting to WiFi..");
 }
 Serial.println("Connected to the WiFi network");
 //connecting to a mqtt broker
 client.setServer(mqtt_broker, mqtt_port);
 client.setCallback(callback);
 while (!client.connected()) {
     String client_id = "esp32-client-";
     client_id += String(WiFi.macAddress());
     Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
     if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
         Serial.println("Public emqx mqtt broker connected");
     } else {
         Serial.print("failed with state ");
         Serial.print(client.state());
         delay(2000);
     }
 }
 client.subscribe(topic1);
 client.subscribe(topic2);
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
     Serial.print((char) payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
  
  // If a message is received on the topic esp32/LEDS/led1, you check if the message is either 1 or 0. Turns the ESP GPIO according to the message
  if(strcmp(topic, "esp32/LEDS/1") == 0){
    Serial.print("Changing GPIO 2 to ");
    if((char)payload[0] == '1'){
      digitalWrite(LEDOne, HIGH);
      Serial.print("On");
    }
    else if((char) payload[0] == '0'){
      digitalWrite(LEDOne, LOW);
      Serial.print("Off");
    }
  }
  if(strcmp(topic, "esp32/LEDS/2") == 0){
    Serial.print("Changing GPIO 0 to ");
    if((char) payload[0] == '1'){
      digitalWrite(LEDTwo, HIGH);
      Serial.print("On");
    }
    else if((char) payload[0] == '0'){
      digitalWrite(LEDTwo, LOW);
      Serial.print("Off");
    }
  }
  Serial.println();
}

void loop() {
  client.loop();
}
