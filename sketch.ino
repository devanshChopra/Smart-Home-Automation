/* Simulation-ready Arduino Uno sketch for Wokwi
   - DHT11 on D2
   - PIR (or button) on D3
   - HC-SR04 TRIG D8, ECHO D9
   - MQ-2 simulated by POT on A0
   - LDR simulated by POT on A1
   - Relay LEDs: RELAY1 -> D4 (Fan), RELAY2 -> D5 (Light)
   - Gas alarm LED/Buzzer -> D6
   - Serial Monitor emulates HC-05 Bluetooth (type commands into serial)
*/

#include <DHT.h>
#include <NewPing.h>

#define DHTPIN 2
#define DHTTYPE DHT22  
DHT dht(DHTPIN, DHTTYPE);


#define PIRPIN 3
#define MQ2_PIN A0
#define LDR_PIN A1

#define TRIG_PIN 8
#define ECHO_PIN 9
#define MAX_DISTANCE 200
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

#define RELAY1 4 // Fan (LED simulating relay)
#define RELAY2 5 // Light
#define GAS_ALARM 6 // Buzzer/LED

// thresholds (tune in simulation)
const float TEMP_THRESHOLD = 30.0;
const int MQ2_THRESHOLD = 600; // analog 0-1023, set appropriately in sim
const int LDR_DARK_THRESHOLD = 400; // adjust in sim

unsigned long lastSend = 0;
const unsigned long SEND_INTERVAL = 3000; // 3s

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(PIRPIN, INPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(GAS_ALARM, OUTPUT);
  digitalWrite(RELAY1, LOW); // off
  digitalWrite(RELAY2, LOW);
  digitalWrite(GAS_ALARM, LOW);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.println("Simulation Ready. Use Serial to send commands (e.g., RELAY1 ON).");
}

void loop() {
  // Read sensors
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int mq2 = analogRead(MQ2_PIN);   // simulate gas
  int ldr = analogRead(LDR_PIN);   // simulate light
  int pir = digitalRead(PIRPIN);   // motion (1 when triggered)
  delay(50);
  unsigned int distance = sonar.ping_cm();

  // Automation logic (from your PDF)
  if (!isnan(temperature) && temperature > TEMP_THRESHOLD) {
    digitalWrite(RELAY1, HIGH); // fan ON
  } else {
    digitalWrite(RELAY1, LOW);
  }

  // Motion + LDR logic for light
  if (pir == HIGH) {
    digitalWrite(RELAY2, HIGH); // light ON when motion detected
  } else {
    if (ldr < LDR_DARK_THRESHOLD) digitalWrite(RELAY2, HIGH); // dark -> light ON
    else digitalWrite(RELAY2, LOW);
  }

  // Gas detection
  if (mq2 > MQ2_THRESHOLD) {
    digitalWrite(GAS_ALARM, HIGH);
    Serial.println("ALERT: Gas detected!");
  } else {
    digitalWrite(GAS_ALARM, LOW);
  }

  // Periodic telemetry (CSV)
  if (millis() - lastSend > SEND_INTERVAL) {
    lastSend = millis();
    Serial.print("DATA,"); // CSV prefix
    Serial.print(temperature); Serial.print(",");
    Serial.print(humidity); Serial.print(",");
    Serial.print(mq2); Serial.print(",");
    Serial.print(ldr); Serial.print(",");
    Serial.print(pir); Serial.print(",");
    Serial.println(distance);
  }

  // Read Serial as simulated Bluetooth commands
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    Serial.print("CMD: "); Serial.println(cmd);
    if (cmd.equalsIgnoreCase("RELAY1 ON")) digitalWrite(RELAY1, HIGH);
    else if (cmd.equalsIgnoreCase("RELAY1 OFF")) digitalWrite(RELAY1, LOW);
    else if (cmd.equalsIgnoreCase("RELAY2 ON")) digitalWrite(RELAY2, HIGH);
    else if (cmd.equalsIgnoreCase("RELAY2 OFF")) digitalWrite(RELAY2, LOW);
    else if (cmd.equalsIgnoreCase("STATUS")) {
      Serial.print("STATUS: R1=");
      Serial.print(digitalRead(RELAY1));
      Serial.print(", R2=");
      Serial.print(digitalRead(RELAY2));
      Serial.print(", GAS=");
      Serial.println(digitalRead(GAS_ALARM));
    } else {
      Serial.println("Unknown command. Use RELAY1 ON/OFF, RELAY2 ON/OFF, STATUS.");
    }
  }

  delay(150);
}
