#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// --- Configurazione Wi-Fi ---
const char* ssid = "iPhone di Giorgia";
const char* password = "giuliabroccolo";

// --- Configurazione Telegram ---
const char* botToken = "8229868092:AAGMNqGvgohFvM_43pFaHmv2ZYAw0XMz58Y";
const char* chatID   = "6573132318";

WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

// --- Configurazione OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- Configurazione HC-SR04 ---
const int trigPin = 5;
const int echoPin = 18;

//variabili utilizzate 
long duration;
float distanceCm;
float distancePrev = 0;
unsigned long timePrev = 0;
float velocity = 0;


void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Connessione Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connessione Wi-Fi");
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) {
    delay(500);
    Serial.print(".");
  }
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("\nWi-Fi connesso!");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nErrore connessione Wi-Fi!");
  }

  client.setInsecure(); 
}

void loop() {
  // --- Lettura sensore HC-SR04 ---
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000); // timeout 30ms
  if(duration == 0) duration = 30000; // nessun oggetto rilevato

  distanceCm = duration * 0.034 / 2;

  // ---  velocità (cm/s) ---
  unsigned long now = millis();
  if(timePrev > 0){
    float deltaT = (now - timePrev) / 1000.0; // secondi
    velocity = (distancePrev - distanceCm) / deltaT; 
  }
  distancePrev = distanceCm;
  timePrev = now;

  // --- Serial Monitor ---
  Serial.print("Distanza: ");
  Serial.print(distanceCm);
  Serial.print(" cm  |  Velocità: ");
  Serial.print(velocity);
  Serial.println(" cm/s");

  // --- Display OLED ---
  display.clearDisplay();
  if (distanceCm < 40) {
    display.setCursor(0, 25);
    display.setTextSize(2);
    display.print(distanceCm, 0);
    display.print(" cm");

    display.setCursor(0, 45);
    display.setTextSize(2);
    display.print(velocity, 2);
    display.print(" cm/s");

    /*
    String message = "Oggetto rilevato a " + String(distanceCm, 2) + " cm.\n";
    message += "Velocità: " + String(velocity, 2) + " cm/s";
    if (bot.sendMessage(chatID, message, "Markdown")) {
      Serial.println("Messaggio Telegram inviato!");
    } else {
      Serial.println("Errore invio Telegram!");
    }
    */

  } else {
    display.setCursor(10, 25);
    display.setTextSize(2);
    display.println("Nessun");
    display.setCursor(10, 45);
    display.println("oggetto");
  }
  display.display();

  delay(500);
}
