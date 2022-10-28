#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>


const char* ssid = "WwiFi";
const char* password = "123456789";
const char* mqtt_server = "10.150.185.203";
const int mqtt_port = 1884;

WiFiClient espClient;
PubSubClient client(espClient);

String TOPIC;
String bericht;

int stepR = 0;
int stepL = 0;
int start = 0;
int speed = 150;

const int stepPin = 15;  
const int dirPin = 14;

const float motorAngle = 0.9;
const float stepSize = 0.9;  //full=1, half=0.5, quarter=0.25, etc...
const int potPin = 34;
int val = 0;
void stepperRotate(float rotation, float rpm);

void setup() {
  // put your setup code here, to run once:
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
}

void loop() {

  // put your main code here, to run repeatedly:
  if (start == 1) {
    if (stepL == 1) {
      stepperRotate(-1, speed);
    }
    if (stepR == 2) {
      stepperRotate(1, speed);
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

  
  float stepsPerRotation = (360.00 / motorAngle) / stepSize;

  
  float totalSteps = rotation * stepsPerRotation;

  
  
  unsigned long stepPeriodmicroSec = ((60.0000 / (rpm * stepsPerRotation)) * 1E6 / 2.0000) - 5;

  

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
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

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
    speed = bericht.toInt();
  }
}