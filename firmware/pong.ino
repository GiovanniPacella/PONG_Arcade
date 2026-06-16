#include <TVout.h>
#include <fontALL.h>

TVout TV; // Variabile globale di libreria

// Gestione PIN
const uint8_t coinPin = 2; // Pin gettoneria

// Interrupt e gestione loop
bool waitingCoin = true; // flag coin per loop
volatile bool coinInserted = false; // flag per interrupt


// Strutture dati
struct Ball {
  uint8_t x = 60;
  uint8_t y = 48;
  int8_t dirX = 1;
  int8_t dirY = 1;
} ball;

struct Player {
  uint8_t score = 0; // Punteggio
  uint8_t posY = 30; // Posizione veriticale della racchetta
  const uint8_t height = 14; // Lunghezza racchetta
};

Player p1; // Giocatore 1
Player p2; // Giocatore 2

// Definizione variabili - Musica schermata iniziale
const uint16_t melodia[] = {
  784, 392, 523, 659, // G5, G4, C5, E5  
  622, 392, 988, 494, // D#5, G4, B5, B4 
  880, 523, 659, 880, // A5, C5, E5, A5
  784, 523, 587, 659, // G5, C5, D5, E5
  880, 523, 698, 880, // A5, C5, F5, A5 
  830, 523, 587, 698, // G#5, C5, D5, F5
  659, 392, 523, 659, // E5, G4, C5, E5  
  587, 440, 494, 587 // D5, A4, B4, D5  
};

const uint8_t numNote = sizeof(melodia) / sizeof(melodia[0]); // Numero note (Lunghezza array)
uint8_t indiceNota = 0; // Indice nota corrente
unsigned long tempoUltimaNota = 0;
const uint8_t durataNota = 135;
const uint8_t pausaNote = 150;

// ISR (Interrupt Service Routine) per l'inserimento della moneta
void isrCoin() {
  coinInserted = true;
}

// Funzione di reset - Pallina fuori campo (Parametro: Nuova direzione pallina)
void resetBall(int8_t newballDirX) {
  TV.tone(200, 400); // 200 Hz, 400 ms
  TV.delay(1000);
  
  ball.x = 60; // Posizione X centrale
  ball.y = 48; // Posizione Y centrale
  ball.dirX = newballDirX > 0 ? 1 : -1; // Direzione pallina invertita
  ball.dirY = random(0, 2) == 0 ? 1 : -1; // Direzione verticale di partenza casuale
}

// Stampa elementi a schermo
void stampaElementi(){
  // Punteggio
  TV.print(30, 10, (int)p1.score);
  TV.print(74, 10, (int)p2.score);

  // Racchetta sinistra (Giocatore 1)
  TV.draw_line(0, p1.posY, 0, p1.posY + p1.height, WHITE);   
  
  // Racchetta destra (Giocatore 2)
  TV.draw_line(112, p2.posY, 112, p2.posY + p2.height, WHITE); 

  // Bordi superiore e inferiore
  TV.draw_line(2, 0, 110, 0, WHITE);   
  TV.draw_line(2, 95, 110, 95, WHITE); 

  // Rete centrale
  for(uint8_t i = 1; i < 96; i += 6) {
    TV.draw_line(56, i, 56, i + 3, WHITE);
  }
}

// Gestione pallina (2x2 pixel)
void movimentoPallina(){
  TV.set_pixel(ball.x, ball.y, WHITE);
  TV.set_pixel(ball.x + 1, ball.y, WHITE);
  TV.set_pixel(ball.x, ball.y + 1, WHITE);
  TV.set_pixel(ball.x + 1, ball.y + 1, WHITE);
}

// Schermo di fine partita
void victoryScreen(const char* vincitore) {
  TV.clear_screen(); // Pulizia schermo

  // messaggio vittoria
  TV.print(0, 35, "V I C T O R Y !"); TV.print(28, 55, vincitore);
  Serial.write(85); // Power off display (85)

  // Melodia Buzzer
  TV.tone(392, 120); TV.delay(120); // Sol
  TV.tone(523, 120); TV.delay(120); // Do 
  TV.tone(659, 120); TV.delay(120); // Mi
  TV.tone(784, 250); TV.delay(250); // Sol
  TV.tone(659, 120); TV.delay(120); // Mi
  TV.tone(784, 600);                // Sol

  TV.delay(4000); // Pausa di 4 secondi
  TV.clear_screen(); // Pulizia schermo
  
  // Reset dei punteggi
  p1.score = 0;
  p2.score = 0;
  
  // Reset posizione pallina
  ball.x = 60; ball.y = 48;
  
  waitingCoin = true; // Reset Flag Moneta
  coinInserted = false; 

  EIFR = bit(INTF0); // Pulizia della memoria hardware (causa attivazione immediata dell'interrupt)
  attachInterrupt(digitalPinToInterrupt(coinPin), isrCoin, RISING); // Attivazione Interrupt
}

void setup() {
  // Inizializzazione schermo (PAL - Europa)
  TV.begin(PAL, 120, 96); // Risoluzione (120x96)
  TV.select_font(font8x8); // Font
  Serial.begin(9600); // Comunicazione seriale con l'esp8266
  randomSeed(analogRead(A1)); // Inzializzazione della funzione random su pin analogico scollegato
  Serial.setTimeout(10);
  
  pinMode(coinPin, INPUT); // Input - Sensore infrarossi

  attachInterrupt(digitalPinToInterrupt(coinPin), isrCoin, RISING); // Aggancio Interrupt 

  Serial.write(85); // Power off display (85)

  
  // Schermata di avvio
  TV.print(44, 25, "PONG");
  TV.print(16, 45, "Giovanni P."); 
  TV.print(4, 65, "Sist. Embedded");

  // Suono di avvio
  TV.tone(1000, 100); 
  TV.delay(100);
  TV.tone(1500, 200);

  TV.delay(5000); // Da modificare: Lo schermo CRT, se freddo, si accende lentamente
  TV.clear_screen();
}

void loop() {

  if(waitingCoin) { // Se si è in attesa di una moneta... 
    unsigned long tempoAttuale = millis();

    // Melodia
    if (tempoAttuale - tempoUltimaNota >= pausaNote) { // Controllo se è possibile riprodurre la nota successiva
      TV.tone(melodia[indiceNota], durataNota); // Riproduzione di una nota con un certo indice e data una certa durata
      tempoUltimaNota = tempoAttuale; // Aggiornamento nota
      indiceNota++; // Aggiornamento indice nota
      if (indiceNota >= numNote) indiceNota = 0; // Melodia cicilica
    }

    // Scritta lampeggiante (500 ms)
    if ((tempoAttuale / 500) % 2 != 0) { 
      TV.print(16, 36, "I N S E R T"); TV.print(32, 52, "C O I N"); // Stampa scritta
    } else {
      TV.print(16, 36, "           "); TV.print(32, 52, "       "); // Cancella scritta
    }
    TV.delay(20);

    // Controllo inserimento moneta (Variabile Interrupt)
    if (coinInserted) {
      
      // Melodia Buzzer
      TV.tone(988, 100); TV.delay(100);  
      TV.tone(1318, 400); TV.delay(400); 

      waitingCoin = false; // Flag a false
      indiceNota = 0; // Reset canzone

      coinInserted = false; // Reset variabile Interrupt
      detachInterrupt(digitalPinToInterrupt(coinPin)); // Disattivazione Interrupt

      Serial.write(84); // Accende a 0 i display (84 - RESET)

      TV.clear_screen(); // Pulizia schermo
      TV.delay(1000);
    }

  } else { // INZIO PARTITA
    TV.clear_screen(); // Pulizia schermo

    while (Serial.available() > 0) { 
      byte dato = Serial.read(); // singolo byte
      
      if (dato <= 82) { // Se il valore del byte è inferiore o uguale a 82 - Giocatore 1
        p1.posY = dato; // Impostazione posizione racchetta - Giocatore 1
      } 
      else if (dato >= 100 && dato <= 182) {
        // Se il valore del byte è compreso tra 100 e 182 - Giocatore 2
        p2.posY = dato - 100; // Impostazione posizione racchetta - Giocatore 1
      }
    }

    stampaElementi(); // Aggiornamento elementi grafici
    
    ball.x += ball.dirX; ball.y += ball.dirY; // Movimento e collisioni
  
    if (ball.y <= 0 || ball.y >= 94) { // Rimbalzo sui bordi superiore e inferiore
      ball.dirY = -ball.dirY; // Direzione verticale invertita
      TV.tone(400, 30); // 400 Hz, 30 ms 
    }

    // Controllo collisione racchetta - Giocatore 1
    if (ball.x <= 1) { // Pallina completamente a sinistra
      if (ball.y >= p1.posY && ball.y <= p1.posY + p1.height) { // Controllo se la pallina tocca la racchetta
        ball.dirX = random(1, 3); // Nuova velocità causale
        ball.dirY = (ball.dirY > 0) ? random(1, 3) : -random(1, 3);
        TV.tone(800, 30); // 800 Hz, 300 ms
      } else { // Palla fuori dallo schermo - Assegnazione punto al Giocatore 2
        p2.score++;
        Serial.write(82); // Incremento punteggio - Giocatore 2 (82)
        resetBall(1); // Reset pallina - "Servizio" verso il Giocatore che ha fatto punto (G2)
        if(p2.score > 9){ // Vittoria Giocatore 2  - Score pari a 10
          victoryScreen("Player 2"); return;
        } 
      }
    }

    // Controllo collisione racchetta - Giocatore 2
    if (ball.x >= 110) { // Pallina completamente a destra
      if (ball.y >= p2.posY && ball.y <= p2.posY + p2.height) { // Controllo se la pallina tocca la racchetta
        ball.dirX = - random(1, 3); // Nuova velocità causale
        ball.dirY = (ball.dirY > 0) ? random(1, 3) : -random(1, 3);
        TV.tone(800, 30); // 800 Hz, 300 ms
      } else { // Palla fuori dallo schermo - Assegnazione punto al Giocatore 1
        p1.score++; // Incremento punteggio al Giocatore 1
        Serial.write(81); // Incremento punteggio - Giocatore 1 (81)
        resetBall(-1); // Reset pallina - "Servizio" verso il Giocatore che ha fatto punto (G1)
        if(p1.score > 9) { // Vittoria Giocatore 1 - Score pari a 10
          victoryScreen("Player 1"); return;
        }
      }
    }
    movimentoPallina(); // Gestione e movimento della pallina
  } 

  TV.delay(20); // Framerate
}