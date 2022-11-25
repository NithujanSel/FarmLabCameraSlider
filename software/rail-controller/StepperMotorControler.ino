//Stepper Board Test Code
//Kevin Darrah  2017

#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>


const char* ssid = "WwiFi";
const char* password = "123456789";
const char* mqtt_server = "172.16.139.2";
const int mqtt_port = 1884;

WiFiClient espClient;
PubSubClient client(espClient);

String TOPIC;
String bericht;

int stepR = 0;
int stepL = 0;
int start = 0;
int speedMotor = 150;
int motorPos = 0;

const int stepPin = 15;
const int dirPin = 14;

const float motorAngle = 0.9;
const float stepSize = 0.9;  //vol=1, half=0.5, qua=0.25
const int potPin = 34;
int val = 0;
void stepperRotate(float rotation, float rpm);

//Array manipulatie
int getPos[15];



void setup() {
  // put your setup code here, to run once:
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  //pinMode(potPin, INPUT);
  //pinMode(actPin, OUTPUT);  hooked to VCC, so no Arduino control
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
}

void loop() {
  client.loop();

  if (start == 1) {
    if (stepL == 1) {
      stepperRotate(-1, speedMotor);
    }
    if (stepR == 2) {
      stepperRotate(1, speedMotor);
    }
  }

}

void stepperRotate(float rotation, float rpm) {
  if (rotation > 0) {
    digitalWrite(dirPin, HIGH);
  } else {
    digitalWrite(dirPin, LOW);
    rotation = rotation * -1;
  }



  for (unsigned long i = 0; i < totalSteps; i++) {
    digitalWrite(stepPin, 1);
    //PORTD |= (1 << 2);
    delayMicroseconds(stepPeriodmicroSec);
    digitalWrite(stepPin, 0);
    //PORTD &= ~(1 << 2);
    delayMicroseconds(stepPeriodmicroSec);
  }
}


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Stepper";
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("Farmlab2/stepper/links");
      client.subscribe("Farmlab2/stepper/rechts");
      client.subscribe("Farmlab2/stepper/start");
      client.subscribe("Farmlab2/stepper/speed");
      client.subscribe("Farmlab2/stepper/getPos");
      client.subscribe("Farmlab2/stepper/getPosList");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
int j = 0;
int arr[50] = { 0 };
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  Serial.println();
  bericht = "";
  TOPIC = topic;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    bericht += (char)message[i];
    Serial.println();
  }
  if (topic == "Farmlab2/stepper/links") {
    stepL = bericht.toInt();
    // if (stepL == 1)
    //   stepperRotate(-1, 200);
  }

  if (topic == "Farmlab2/stepper/rechts") {
    stepR = bericht.toInt();
    // if (stepR == 2)
    //   stepperRotate(1, 200);
  }

  if (topic == "Farmlab2/stepper/start") {
    start = bericht.toInt();
    stepL = 0;
    stepR = 0;
  }
  if (topic == "Farmlab2/stepper/speed") {
    speedMotor = bericht.toInt();
  }
  if (topic == "Farmlab2/stepper/postPos") {
    motorPos = bericht.toInt();
  }
  if (topic == "Farmlab2/stepper/postPosList") {
    String k = bericht;
    for (int i = 0; k[i] != '\0'; i++) {
      if (k[i] == ',') {
      }
      else {
        getPos[i] = k[i];
      }
    }
    for (byte i = 0; i < 5 ; i++) {
      Serial.println(getPos[i]);
    }
  }
}




void convert_array(String str) {
  int str_length = str.length();
  int i = 0;
  //traversing the string
  for (i = 0; str[i] != '\0'; i++) {
    if (str[i] == ', ') {
      j++;
    }
    else {
      arr[j] = arr[j] * 10 + (str[i] - 48);
    }
  }
}
