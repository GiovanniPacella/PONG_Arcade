# PONG-Arcade

![Foto Progetto](images/progetto.jpg)

## Indice
- [Descrizione del Progetto](#-descrizione-del-progetto)
- [Schema di Collegamento](#-schema-di-collegamento)
- [Componenti Hardware](#-componenti-hardware)

## Descrizione del Progetto
PONG Arcade è un cabinato retrò dedicato al videogioco omonimo, costruito utilizzando una piccola TV a tubo catodico. Il progetto è basato sull'utilizzo di un Arduino UNO come modulo centrale (logica applicativa del gioco e trasmissione del segnale video) e sull'utilizzo di due controller ESP8266 separati. Questi ultimi elaborano l'input dato dai giocatori tramite due potenziometri lineari e gestiscono display a 7 segmenti e led per la gestione e visualizzazione del punteggio. 

## Schema di Collegamento
![Schema di Collegamento](images/schemaCollegamento.png)

## Componenti Hardware
- 1 x Arduino Uno R3
- 2 x Wemos D1 Mini (ESP8266)
- 1 x TV a tubo catodico (CRT) con ingresso RCA composito
- 1 x Sensore IR FC-03
- 3 x Potenziometri lineari (2 x 250kΩ, 1 x 10kΩ)
- 2 x Display a 7 segmenti
- 1 x Modulo Step-Down DC-DC (da 12V a 5V)
- 1 x Speaker 1W 8Ω
- Componenti passivi (resistenze da 100Ω / 220Ω / 330Ω / 470Ω / 1000Ω)
