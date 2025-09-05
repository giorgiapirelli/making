#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Includiamo le credenziali 
#include "credentials.h"

// Inizializzazione client Telegram
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// Configurazione display OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Configurazione sensore HC-SR04
const int TRIG_PIN = 5;    // Pin trigger del sensore
const int ECHO_PIN = 18;   // Pin echo del sensore

// Variabili per le misurazioni
long duration;             // Durata dell'impulso ultrasonico
float distanceCm;          // Distanza corrente in centimetri
float distancePrev = 0;    // Distanza precedente per calcolo velocità
unsigned long timePrev = 0; // Tempo precedente
float velocity = 0;        // Velocità calcolata in cm/s

// Configurazioni di sistema
const float DETECTION_THRESHOLD = 40.0;  // Soglia di rilevamento in cm
const unsigned long WIFI_TIMEOUT = 20000; 
const unsigned long SENSOR_TIMEOUT = 30000;
const int LOOP_DELAY = 500;               


void setup() {
  // Inizializzazione comunicazione seriale
  Serial.begin(115200);
  
  // Configurazione pin del sensore HC-SR04
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.println("Sensore HC-SR04 configurato");
  
  // Inizializzazione display OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("ERRORE: Impossibile inizializzare display OLED!");
    // Blocca l'esecuzione se il display non funziona
    while(true) {
      delay(1000);
    }
  }
  
  // Configurazione iniziale del display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.display();
  Serial.println("Display OLED inizializzato");
  
  // Connessione alla rete Wi-Fi
  connectToWiFi();
  
  // Configurazione client Telegram
  client.setInsecure();
  Serial.println("Client Telegram configurato");
  
  Serial.println("tutto pronto");
  showStartupMessage(); //funzione per messaggio di avvio
}

// Funzione per connessione wifi
void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connessione a Wi-Fi");
  
  // Mostra stato della connessione sul display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Connessione Wi-Fi...");
  display.display();
  
  unsigned long startAttemptTime = millis();
  
  // Attende connessione con timeout
  while (WiFi.status() != WL_CONNECTED && 
         millis() - startAttemptTime < WIFI_TIMEOUT) {
    delay(500);
    Serial.print(".");
  }
  
  // Verifica risultato connessione
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connesso!");
    Serial.print("Indirizzo IP: ");
    Serial.println(WiFi.localIP());
    
    // Conferma connessione
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Wi-Fi OK!");
    display.setCursor(0, 20);
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.display();
    delay(2000);
    
  } else {
    Serial.println("\nERRORE: Impossibile connettersi al Wi-Fi!");
    
    // Errore connessione
    display.clearDisplay();
    display.setCursor(0, 25);
    display.println("ERRORE Wi-Fi!");
    display.display();
    delay(3000);
  }
}


void showStartupMessage() { // Messaggio di avvio sul display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("RADAR ULTRASONICO");
  display.println("Soglia: " + String(DETECTION_THRESHOLD) + " cm");
  display.println("Pronto...");
  display.display();
  delay(2000);
}


void loop() {
  // Calcoliamo distanza
  measureDistance();
  
  // Calcoliamo velocità dell'oggetto
  calculateVelocity();
  
  // Stampa dei dati 
  printSerialData();
  
  // Aggiornamento il display OLED
  updateDisplay();
  
  delay(LOOP_DELAY);
}


void measureDistance() { // funzione per misurare distanza 
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Misura la durata dell'eco con timeout
  duration = pulseIn(ECHO_PIN, HIGH, SENSOR_TIMEOUT);
  
  // Se non riceve eco
  if(duration == 0) {
    duration = SENSOR_TIMEOUT;
  }
  
  // Calcoliamo la distanza in centimetri
  // Formula: distanza = (durata * velocità_suono) / 2
  // Velocità suono ≈ 343 m/s = 0.034 cm/microsec
  distanceCm = duration * 0.034 / 2;
}


void calculateVelocity() { //funzione per calcolare velocità
  unsigned long currentTime = millis();
  
  // Calcola la velocità solo se abbiamo una misurazione precedente
  if(timePrev > 0) {
    float deltaTime = (currentTime - timePrev) / 1000.0; // Converti in secondi
    
    // Velocità = variazione distanza / tempo trascorso
    // valore positivo = oggetto si avvicina
    // valore negativo = oggetto si allontana
    velocity = (distancePrev - distanceCm) / deltaTime;
  }
  
  // Aggiornamento valori per la prossima iterazione
  distancePrev = distanceCm;
  timePrev = currentTime;
}


void printSerialData() { // Stampa dei dati 
  Serial.print("  Distanza: ");
  Serial.print(distanceCm, 1);
  Serial.print(" cm  |  Velocità: ");
  Serial.print(velocity, 2);
  Serial.print(" cm/s");
  
}

void updateDisplay() {
  display.clearDisplay();
  
  if (distanceCm < DETECTION_THRESHOLD) {
    // OGGETTO RILEVATO
    
    display.setCursor(0, 25);
    display.setTextSize(2);
    display.print(distanceCm, 0);
    display.print(" cm");

    display.setCursor(0, 45);
    display.setTextSize(2);
    display.print(velocity, 2);
    display.print(" cm/s");
    
    // Inviamo notifica Telegram
    sendTelegramNotification();
    
  } else {
    // NESSUN OGGETTO
    
    display.setTextSize(2);
    display.setCursor(10, 25);
    display.println("Nessun");
    display.setCursor(10, 45);
    display.println("oggetto");
  }
  
  // Stato Wi-Fi
  display.setTextSize(1);
  display.setCursor(100, 0);
  if(WiFi.status() == WL_CONNECTED) {
    display.println("WiFi");
  } else {
    display.println("X");
  }
  
  display.display();
}

void sendTelegramNotification() {
  // Verifica connessione Wi-Fi
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("AVVISO: Wi-Fi disconnesso, impossibile inviare notifica Telegram");
    return;
  }
  String message = "Oggetto rilevato a " + String(distanceCm, 2) + " cm.\n";
  message += "Velocità: " + String(velocity, 2) + " cm/s.\n";
  
  
  // Invia il messaggio
  Serial.print("  Invio notifica Telegram... ");
  if (bot.sendMessage(CHAT_ID, message, "Markdown")) {
    Serial.println("SUCCESSO!");
  } else {
    Serial.println("ERRORE!");
  }
}
