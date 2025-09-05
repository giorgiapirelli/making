#include <Servo.h>

Servo radarServo;

// Configurazione pin
const int trigPin = 11;
const int echoPin = 10;
const int servoPin = 12;

const int redLed = 3;
const int greenLed = 4;
const int buzzer = 2;

const int alertDistance = 40;       // soglia di allarme cm
const unsigned long updateInterval = 20; // ms tra aggiornamenti servo

int currentAngle = 0;
int increment = 1;
float lastDistance = 0;
unsigned long lastServoUpdate = 0;

void setup() {
  radarServo.attach(servoPin); 
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(buzzer, OUTPUT);

  Serial.begin(9600);
  setNormalState(); // stato iniziale, funzione riportata sotto
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastServoUpdate >= updateInterval) {
    lastServoUpdate = currentMillis;
    radarServo.write(currentAngle);

    // misura ogni 5°
    if (currentAngle % 5 == 0) {
      float distance = getDistance(); // misura distanza, funzione riportata sotto
      float velocity = (distance - lastDistance) / (updateInterval / 1000.0);
      lastDistance = distance;

      // gestione LED e buzzer
      if (distance > 0 && distance < alertDistance) {
        alertMode(); // accendi allarme, funzione riportata sotto
      } else {
        setNormalState(); // ripristina stato normale
      }

      // invio dati SEMPRE (anche se distance = 0)
      Serial.print(currentAngle);
      Serial.print(",");
      Serial.println(distance);
    }

    // movimento servo avanti/indietro
    currentAngle += increment;
    if (currentAngle >= 180) { currentAngle = 180; increment = -1; }
    if (currentAngle <= 0)   { currentAngle = 0;   increment = 1; }
  }
}

// misurazione distanza in cm
float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // timeout 30ms
  // Calcoliamo la distanza in centimetri
  // Formula: distanza = (durata * velocità_suono) / 2
  // Velocità suono ≈ 343 m/s = 0.034 cm/microsec
  float distance = duration * 0.034 / 2;
  if (distance > alertDistance) distance = 0; // fuori range = 0
  return distance;
}

// modalità allarme: LED rosso + buzzer
void alertMode() {
  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, HIGH);
  playTonePWM(2000, 50);
}

// stato normale: LED verde acceso
void setNormalState() {
  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, LOW);
  digitalWrite(buzzer, LOW);
}

// buzzer passivo PWM
void playTonePWM(int frequency, int duration) { // frequenza in Hz, durata in ms, in questo caso si ha 2000 hz e 50 ms 
  int period = 1000000 / frequency; 
  int pulse = period * 0.6; // durata impulso
  long cycles = (long)frequency * duration / 1000;
  for (long i = 0; i < cycles; i++) {
    digitalWrite(buzzer, HIGH);
    delayMicroseconds(pulse);
    digitalWrite(buzzer, LOW);
    delayMicroseconds(period - pulse);
  }
}
