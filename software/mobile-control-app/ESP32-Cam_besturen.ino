#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "esp_camera.h"


const char* ssid = "WwiFi";
const char* password = "123456789";
const char* mqtt_server = "10.150.185.203";
const int mqtt_port = 1884;
//const char* mqttUser = "XXXXXXXXXXXXXXXXXX";
//const char* mqttPassword = "XXXXXXXXXXXXXXXXXX";

WiFiClient espClient;
PubSubClient client(espClient);

String TOPIC;
String bericht;

Servo servoPan;
Servo servoTilt;

byte pinPan = 14;
byte pinTilt = 15;

byte panPositie = 0;
byte tiltPositie = 0;

//==CAMERA===
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"
// Flash
#define LED_BUILTIN 4

const char* topic_PHOTO = "SMILE";
const char* topic_PUBLISH = "PICTURE";
const char* topic_FLASH = "FLASH";
const char* topic_Kalibreren = "Kalibreren";
const int MAX_PAYLOAD = 60000;

bool flash;

void startCameraServer();

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
    String clientId = "ESP32CamClient";
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("Farmlab2/slider/pan");
      client.subscribe("Farmlab2/slider/tilt");
      client.subscribe(topic_PHOTO);
      client.subscribe(topic_FLASH);
      client.subscribe(topic_Kalibreren);
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
  bericht = "";
  TOPIC = topic;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    bericht += (char)message[i];
  }
  Serial.println();
  if (topic == topic_PHOTO) {
    take_picture();
  }
  if (topic == topic_FLASH) {
    set_flash();
  }

  if (topic == topic_Kalibreren) {
    take_picture();
    delay(5000);
  }
  Serial.println();

  if (TOPIC == "Farmlab2/slider/pan") {
    panPositie = bericht.toInt();
    servoPan.write(panPositie);
  }
  if (TOPIC == "Farmlab2/slider/tilt") {
    tiltPositie = bericht.toInt();
    servoTilt.write(tiltPositie);
  }
}



void setup() {
  // Allow allocation of all timers
  Serial.begin(115200);
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  servoPan.setPeriodHertz(50);          // standard 50 hz servo
  servoPan.attach(pinPan, 1000, 2000);  // attaches the servo on pin 18 to the servo object
                                        // using default min/max of 1000us and 2000us
                                        // different servos may require different min/max settings
                                        // for an accurate 0 to 180 sweep

  servoTilt.setPeriodHertz(50);  // standard 50 hz servo
  servoTilt.attach(pinTilt, 1000, 2000);
  cameraSetup();
  setup_wifi();
  startCameraServer();
  client.setServer(mqtt_server, mqtt_port);
  client.setBufferSize(MAX_PAYLOAD);  //This is the maximum payload length
  client.setCallback(callback);
  reconnect();
}

void loop() {
  client.loop();
  // panPositie = map(analogRead(potPan), 0, 1023, 0, 180);
  // tiltPositie = map(analogRead(potTilt), 0, 1023, 0, 180);
   if (TOPIC == "Farmlab2/slider/pan") {
      panPositie = bericht.toInt();
      servoPan.write(panPositie);
    }
    if (TOPIC == "Farmlab2/slider/tilt") {
      tiltPositie = bericht.toInt();
      servoTilt.write(tiltPositie);
    }

  delay(20);
}


void take_picture() {
  camera_fb_t* fb = NULL;
  if (flash) { digitalWrite(LED_BUILTIN, HIGH); };
  Serial.println("Taking picture");
  fb = esp_camera_fb_get();  // used to get a single picture.
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  Serial.println("Picture taken");
  digitalWrite(LED_BUILTIN, LOW);
  sendMQTT(fb->buf, fb->len);
  esp_camera_fb_return(fb);  // must be used to free the memory allocated by esp_camera_fb_get().
}

void set_flash() {
  flash = !flash;
  Serial.print("Setting flash to ");
  Serial.println(flash);
  if (!flash) {
    for (int i = 0; i < 6; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
    take_picture();
  }
  if (flash) {
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(500);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
    take_picture();
  }
}

void sendMQTT(const uint8_t* buf, uint32_t len) {
  Serial.println("Sending picture...");
  if (len > MAX_PAYLOAD) {
    Serial.print("Picture too large, increase the MAX_PAYLOAD value");
  } else {
    Serial.print("Picture sent ? : ");
    Serial.println(client.publish(topic_PUBLISH, buf, len, false));
  }
}





void cameraSetup() {
  // Define Flash as an output
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialise the Serial Communication
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Config Camera Settings
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  flash = true;

// Not used in our project
#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  delay(5000);
  sensor_t* s = esp_camera_sensor_get();
  //initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);        //flip it back
    s->set_brightness(s, 1);   //up the blightness just a bit
    s->set_saturation(s, -2);  //lower the saturation
  }
  //drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

// Not used in our project
#if defined(CAMERA_MODEL_M5STACK_WIDE)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif
}