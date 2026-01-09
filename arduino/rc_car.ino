#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "JARVIS";
const char* password = "00000000";

WiFiUDP udp;
const int udpPort = 4210;
char incomingPacket[255];

// L239D Motor Driver pins
const int IN1 = D1; // GPIO14
const int IN2 = D2; // GPIO12
const int ENA = D5; // GPIO13
const int IN3 = D3; // GPIO5
const int IN4 = D4; // GPIO4
const int ENB = D7; // GPIO0

const int LED_BUILTIN_PIN = LED_BUILTIN;  // GPIO2, onboard LED

// Blink control
unsigned long previousMillis = 0;
const long BLINK_INTERVAL = 500;
bool ledBlinkState = LOW;
enum LedMode { OFF, SOLID, BLINK };
LedMode ledMode = OFF;

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(ENA, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);

  pinMode(LED_BUILTIN_PIN, OUTPUT);
  digitalWrite(LED_BUILTIN_PIN, HIGH); // Off (active low)

  stopMotorA();
  stopMotorB();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi connected! IP: " + WiFi.localIP().toString());
  udp.begin(udpPort);
  Serial.printf("UDP Listening on port %d\n", udpPort);
}

void loop() {
  // LED management
  if (ledMode == SOLID) {
    digitalWrite(LED_BUILTIN_PIN, LOW); // Solid ON (active low)
  } else if (ledMode == BLINK) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= BLINK_INTERVAL) {
      previousMillis = currentMillis;
      ledBlinkState = !ledBlinkState;
      digitalWrite(LED_BUILTIN_PIN, ledBlinkState ? LOW : HIGH);
    }
  } else {
    digitalWrite(LED_BUILTIN_PIN, HIGH); // OFF
  }

  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incomingPacket, 255);
    incomingPacket[len] = '\0';

    int left_pwm = 0, right_pwm = 0;
    int parsed = sscanf(incomingPacket, "%d,%d", &left_pwm, &right_pwm);
    if (parsed == 2) {
      left_pwm = constrain(left_pwm, -255, 255);
      right_pwm = constrain(right_pwm, -255, 255);
      
      setMotorA(left_pwm); setMotorB(right_pwm);

      // Determine LED mode and serial messaging
      if (left_pwm > 0 && right_pwm > 0) {
        ledMode = SOLID;
        Serial.println("Forward");
      } else if (left_pwm < 0 && right_pwm < 0) {
        ledMode = SOLID;
        Serial.println("Backward");
      } else if ((left_pwm > 0 && right_pwm < 0) || (left_pwm < 0 && right_pwm > 0)) {
        ledMode = BLINK;
        if (left_pwm > 0)
          Serial.println("Right");
        else
          Serial.println("Left");
      } else if (left_pwm == 0 && right_pwm == 0) {
        ledMode = OFF;
        Serial.println("Stopped");
      } else {
        // Handles asymmetric zero (one stopped, one moving)
        ledMode = OFF;
        if (left_pwm != 0)
//          Serial.println(left_pwm > 0 ? "Forward" : "Backward");
          Serial.println(left_pwm);
          Serial.println(right_pwm);
        if (right_pwm != 0)
          Serial.println(left_pwm);
          Serial.println(right_pwm);
//          Serial.println(right_pwm > 0 ? "Forward" : "Backward");
      }
    }
  }
}

// Motor helpers
void setMotorA(int pwm) {
  if (pwm == 0) {
    stopMotorA();
  } else if (pwm > 0) {
    Serial.println("A");
    Serial.println(pwm);
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    analogWrite(ENA, pwm * 4);  // 255 × 4 = 1020 (almost 1023)
  } else {
    Serial.println("A");
    Serial.println(pwm);
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    analogWrite(ENA, (-pwm) * 4);
  }
}

void setMotorB(int pwm) {
  if (pwm == 0) {
    stopMotorB();
  } else if (pwm > 0) {
    Serial.println("B");
    Serial.println(pwm);
    
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    analogWrite(ENB, pwm * 4);
  } else {
    Serial.println("B");
    Serial.println(pwm);
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
    analogWrite(ENB, (-pwm) * 4);
  }
}
void stopMotorA() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); analogWrite(ENA, 0);
}
void stopMotorB() {
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW); analogWrite(ENB, 0);
}
