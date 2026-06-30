#include <Comandi.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

// MAC ADDRESS - Giocatore 2
uint8_t macAddrP2[] = {0xE8, 0xDB, 0x84, 0x9B, 0xE3, 0x73};

// PIN
const uint8_t pinDisplay[] = {D1, D2, D0, D5, D6, D7, D8}; // A, B, C, D, E, F, G
const uint8_t pinPot = A0; // Potenziometro
const uint8_t pinLed = D4; // Led Giocatore 1

int8_t ultimaPosizioneY = -1; // Variabile per la gestione dell'ultima posizione registrata

// Punteggi dei giocatori
uint8_t score_P1 = 0;
uint8_t score_P2 = 0;

// Display a 7 segmenti
const byte numeri[11][7] = { 
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1},  // 9
  {0, 0, 0, 0, 0, 0, 0} // 10 - NULL
};

typedef struct dati {
  uint8_t tipo; // Tipologia di dato
  uint8_t valore; // Valore
} dati;

dati datiDaInviare; // Dati da inviare
dati datiRicevuti; // Dati da ricevere

// Ricezione dati wireless da Giocatore 2
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&datiRicevuti, incomingData, sizeof(datiRicevuti)); // Copia dei dati ricevuti
  if (datiRicevuti.tipo == 1) { 
    Serial.write(datiRicevuti.valore); // Racchetta Giocatore 2
  }
}

// Aggiornamento display con numero corrispondente
void aggiornaDisplay(uint8_t numero) {
  for (uint8_t i = 0; i < 7; i++) {
    digitalWrite(pinDisplay[i], numeri[numero][i]); // Impostazione accensione/spegnimento dei segmenti del display
  }
}

// Lampeggio led per un secondo
void blinkLed() {
  digitalWrite(pinLed, HIGH); // Accensione led
  delay(1000);
  digitalWrite(pinLed, LOW); // Spegnimento led
  return;
}

// Lampeggio led multiplo in caso di vittoria
void blinkVictory() {
  for (uint8_t i = 0; i < 4; i++) {
    digitalWrite(pinLed, HIGH); // Accensione led
    delay(300);
    digitalWrite(pinLed, LOW); // Spegnimento led
    delay(300);
  }
}

void setup() {

  for (uint8_t i = 0; i < 7; i++) { 
    pinMode(pinDisplay[i], OUTPUT); // PinMode del display a 7 segmenti
  }

  aggiornaDisplay(1); // 1 sul display all'accensione

  Serial.begin(9600); // Velocita' di comunicazione
  delay(1000);


  // ACCENSIONE WI-FI
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA); 
  WiFi.disconnect(); 
  delay(100);
  
  if (esp_now_init() != 0) {
    //Serial.println("ERRORE: ESP-NOW non avviato");
    return;
  }
  
  //Serial.println("ESP-NOW Avviato!");
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(OnDataRecv); // Registra ascolto
  esp_now_add_peer(macAddrP2, ESP_NOW_ROLE_COMBO, 1, NULL, 0); // Connessione al Giocatore 2

  // Avviso di fine setup (blink del led)
  pinMode(pinLed, OUTPUT);
  blinkLed();

}

void loop() {
  // 1. GESTIONE SCORE
  while (Serial.available() > 0) { 
    byte comando = Serial.read(); // Legge il singolo byte 
    
    switch(comando) {
      case POINT_P1: // Punto Giocatore 1
        blinkLed();
        score_P1++; aggiornaDisplay(score_P1); // Aggiornamento display

        break;
      case POINT_P2: // Punto Giocatore 2
        score_P2++; datiDaInviare.tipo = 2; datiDaInviare.valore = score_P2;
        esp_now_send(macAddrP2, (uint8_t *) &datiDaInviare, sizeof(datiDaInviare)); // Invio comando al Controller 2
        break;
      case RESET: // Inizio partita
        aggiornaDisplay(0); // Display Giocatore 1 a 0
        datiDaInviare.tipo = 2; datiDaInviare.valore = 0; // Display Giocatore 2 a 0
        esp_now_send(macAddrP2, (uint8_t *) &datiDaInviare, sizeof(datiDaInviare)); // Invio comando al Controller 2
        score_P1 = 0; score_P2 = 0; // Reset punteggi
        break;
      case POWER_OFF: // Power off display
        aggiornaDisplay(10); // Spegnimento display locale (Giocatore 1)
        datiDaInviare.tipo = 2; datiDaInviare.valore = 11; // Invia il comando di spegnimento (Giocatore 2)
        esp_now_send(macAddrP2, (uint8_t *) &datiDaInviare, sizeof(datiDaInviare)); // Invio comando al Controller 2

        // Controllo Vittoria
        if (score_P1 == 10) {
          delay(800);
          blinkVictory();
        }
        break;
        
      default:
        break;
    }  
  }

  // 2. LETTURA E INVIO VALORE POTENZIOMETRO
  int potValue = analogRead(pinPot); // Lettura valore

  int8_t paddleY = map(potValue, 0, 1023, MIN_PADDLE_P1, MAX_PADDLE_P1); 

  if (paddleY != ultimaPosizioneY) { // Se il valore letto è diverso dall'ultima posizione registrata...
    Serial.write(paddleY); // Scrittura racchetta Giocatore 1
    ultimaPosizioneY = paddleY; // Aggiornamento ultima posizione rilevata
  }

  delay(15); // Pausa
}