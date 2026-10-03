#include <ESP8266WiFi.h>
#include <espnow.h>

// Motor pins (adjust as per your wiring)
#define IN1 D5
#define IN2 D6
#define IN3 D7
#define IN4 D8

// Structure to receive
typedef struct struct_message {
  char gesture;
} struct_message;

struct_message msg;

// Handle received data
void onDataReceive(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&msg, incomingData, sizeof(msg));

  Serial.print("Gesture received: ");
  Serial.println(msg.gesture);

  switch (msg.gesture) {
    case 'F': forward();  break;
    case 'B': backward(); break;
    case 'L': left();     break;
    case 'R': right();    break;
    case 'S': stop();     break;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Set as station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register receive callback
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(onDataReceive);
}

void loop() {
  // Nothing here; everything handled via callback
}

// Motor control functions
void forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void left() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void right() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
