#include <WiFi.h>
#include <PubSubClient.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

//Screen setup
#define SDA 5                    //Define SDA pins
#define SCL 4                    //Define SCL pins
LiquidCrystal_I2C lcd(0x27,16,2);

// define the symbols on the buttons of the keypad
char keys[4][4] = {
  {'*', '0', '#', 'D'},
  {'7', '8', '9', 'C'},
  {'4', '5', '6', 'B'},
  {'1', '2', '3', 'A'}
};

byte rowPins[4] = {14, 27, 26, 25}; // connect to the row pinouts of the keypad
byte colPins[4] = {13, 21, 22, 23};   // connect to the column pinouts of the keypad

// initialize an instance of class NewKeypad
Keypad myKeypad = Keypad(makeKeymap(keys), rowPins, colPins, 4, 4);

//create password
char pinNum[] = {"5555"};

// WiFi
const char *ssid = "SpectrumSetup-9F"; // Enter your WiFi name
const char *password = "monthunion197";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "192.168.1.63";
const char *topic1 = "esp32/LEDS/1";
const char *topic2 = "esp32/LEDS/2";
const char *returnTopic = "esp32/password";
const char *returnState = "esp32/lightstate";
const char *mqtt_username = "emqx";
const char *mqtt_password = "123456789";
const int mqtt_port = 1883;

// initialize espClient
WiFiClient espClient;
PubSubClient client(espClient);

// LED Pins
const int LEDOne = 19;
const int LEDTwo = 2;

int returnTopicNum = 0;
char str[2];

void setup() {
  pinMode(LEDOne, OUTPUT);
  pinMode(LEDTwo, OUTPUT);
  Serial.begin(115200); // Initialize the serial port and set the baud rate to 115200
  Serial.println("ESP32 is ready!");  // Print the string "UNO is ready!"
  Wire.begin(SDA, SCL);           // attach the IIC pin
  lcd.init();                     // LCD driver initialization
  lcd.backlight();                // Open the backlight
  lcd.setCursor(0,0);             // Move the cursor to row 0, column 0
  lcd.print("enter password");
  
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
    Serial.print("Changing GPIO 19 to ");
    if((char)payload[0] == '1'){
      digitalWrite(LEDOne, HIGH);
      digitalWrite(LEDTwo, LOW);
      lcd.clear();
      lcd.print("Blue Password");
      strcpy(pinNum, "1234");
      returnTopicNum = 10;
      Serial.print("On");
    }
    else if((char) payload[0] == '0'){
      digitalWrite(LEDOne, LOW);
      digitalWrite(LEDTwo, LOW);
      lcd.clear();
      lcd.print("default pass");
      Serial.print("Off");
      strcpy(pinNum, "5555");
      returnTopicNum = 0;
    }
  }
  if(strcmp(topic, "esp32/LEDS/2") == 0){
    Serial.print("Changing GPIO 2 to ");
    if((char) payload[0] == '1'){
      digitalWrite(LEDTwo, HIGH);
      digitalWrite(LEDOne, LOW);
      lcd.clear();
      lcd.print("Red Password");
      strcpy(pinNum, "ABCD");
      returnTopicNum = 20;
      Serial.print("On");
    }
    else if((char) payload[0] == '0'){
      digitalWrite(LEDTwo, LOW);
      digitalWrite(LEDOne, LOW);
      lcd.clear();
      lcd.print("default pass");
      Serial.print("Off");
      strcpy(pinNum, "5555");
      returnTopicNum = 0;
    }
  }
  Serial.println();
}

void loop() {
  client.loop();
  static char keyIn[4];     // Save the input character
  static byte keyInNum = 0; // Save the the number of input characters 
  // Get the character input
  char keyPressed = myKeypad.getKey();
  // If there is a character input, sent it to the serial port
  if (keyPressed) {
    // Make a prompt tone each time press the key
    Serial.println(keyPressed);
    lcd.setCursor(keyInNum,1);
    lcd.print(keyPressed);
    // Save the input characters
    keyIn[keyInNum++] = keyPressed;
    //check
    if (keyInNum == 4) {
      bool isRight = true;            // Save password is correct or not
      for (int i = 0; i < 4; i++) {   // Judge each character of the password is correct or not
        if (keyIn[i] != pinNum[i])
          isRight = false;            // Mark wrong passageword if there is any wrong character.
      }
      if (isRight) {                  // If the input password is right
        Serial.println("password right!");
        delay(500);
        lcd.setCursor(0,0);
        lcd.clear();
        lcd.print("password right!");
        returnTopicNum += 1;
        sprintf(str, "%d", returnTopicNum);
        client.publish(returnTopic, str);
        returnTopicNum -= 1;
      }
      else {                          // If the input password is wrong
        Serial.println("password wrong!");
        delay(500);
        lcd.setCursor(0,0);
        lcd.clear();
        lcd.print("password wrong!");
        sprintf(str, "%d", returnTopicNum);
        client.publish(returnTopic, str);
      }
      keyInNum = 0; // Reset the number of the input characters to 0
    }
  }
}
