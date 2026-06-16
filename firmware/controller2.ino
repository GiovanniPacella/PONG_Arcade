#include <ESP8266WiFi.h>
#include <espnow.h>

// MAC ADDRESS - Giocatore 1
uint8_t macAddrP1[] = {0xE8, 0xDB, 0x84, 0x9B, 0xE1, 0x57};

// PIN
const uint8_t pinDisplay[] = {D1, D2, D0, D5, D6, D7, D8}; // A, B, C, D, E, F, G
const uint8_t pinPot = A0; // Potenziometro
const uint8_t pinLed = D4; // Led Giocatore 2

int8_t ultimaPosizioneY = -1; // Variabile per la gestione dell'ultima posizione registrata

// Gestione led e punteggio
bool isScore = false; // flag per la ricezione di un punteggio
uint8_t score = 0; // punteggio memorizzato

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

typedef struct dati { // 1 = Racchetta, 2 = Punteggio/Reset
  uint8_t tipo; 
  uint8_t valore;
} dati;

dati datiInviati; // Dati da inviare
dati datiRicevuti; // Dati da ricevere


// Ricezione dati wireless da Giocatore 1
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&datiRicevuti, incomingData, sizeof(datiRicevuti)); // Copia dei dati ricevuti
  if (datiRicevuti.tipo == 2) { // Punteggio/Reset
    if (datiRicevuti.valore == score + 1 && datiRicevuti.valore <= 10) { // Se e' un punteggio valido (uguale al punto successivo previsto)
        score = datiRicevuti.valore; // Salvataggio punteggio
        isScore = true; // flag per l'accendimento del led
    } else if (datiRicevuti.valore == 11) { // POWER OFF
        aggiornaDisplay(datiRicevuti.valore - 1);
      } 
      else { // RESET
        score = 0;
        aggiornaDisplay(datiRicevuti.valore);
      }
  }
}

// Lampeggio led per un secondo (punto)
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

// Aggiornamento display con numero corrispondente
void aggiornaDisplay(uint8_t numero) {
  for (uint8_t i = 0; i < 7; i++) {
    digitalWrite(pinDisplay[i], numeri[numero][i]); // Impostazione accensione/spegnimento dei segmenti del display
  }
}

void setup() {

  for (uint8_t i = 0; i < 7; i++) { // PinMode del display a 7 segmenti
    pinMode(pinDisplay[i], OUTPUT);
  }
  aggiornaDisplay(2); // 2 sul display all'accensione

  Serial.begin(9600); // Velocita' di comunicazione
  delay(1000);

  // ACCENSIONE WI-FI
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA); // Modalità Wi-Fi
  WiFi.disconnect(); 
  delay(100);

  if (esp_now_init() != 0) {
    return; // Errore di inizializzazione ESP-NOW
  }
  
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(OnDataRecv); // Registra ascolto
  esp_now_add_peer(macAddrP1, ESP_NOW_ROLE_COMBO, 1, NULL, 0); // Connessione al Giocatore 1

  // Avviso di fine setup (blink del led)
  pinMode(pinLed, OUTPUT);
  blinkLed();
}

void loop() {
  // GESTIONE LED E DISPLAY
  if (isScore == true) {
    blinkLed();
    aggiornaDisplay(score); // Aggiornamento punteggio
    isScore = false;

    if (score == 10) {
      delay(800);
      blinkVictory();
    }
  }

  // LETTURA E INVIO VALORE POTENZIOMETRO
  int potValue = analogRead(pinPot); // Lettura valore potenziometro
  int8_t paddleY = map(potValue, 0, 1023, 0, 82); // Mapping sui pixel dello schermo

  if (paddleY != ultimaPosizioneY) { // Se il valore letto è diverso dall'ultima posizione registrata...
    datiInviati.tipo = 1; // Racchetta
    datiInviati.valore = paddleY; // Nuova Posizione
    esp_now_send(macAddrP1, (uint8_t *) &datiInviati, sizeof(datiInviati)); // Invio comando al Controller 1
    ultimaPosizioneY = paddleY; // Aggiornamento ultima posizione 
  }
  
  delay(15); // Pausa
}
