![Immagine WhatsApp 2024-10-21 ore 10 09 52_709a2ed3](https://github.com/user-attachments/assets/11b00142-90c7-401f-9466-0e05ab3affe2)
# Sistema di Gestione Esami Universitari
Questo progetto implementa un sistema distribuito basato su un'architettura client-server a tre livelli, progettato per gestire la visualizzazione e la prenotazione degli esami universitari. Il sistema coinvolge tre componenti principali: il Client Studente, il Server Segreteria e il Server Universitario. Queste componenti collaborano per fornire un'interfaccia funzionale agli studenti, consentendo loro di visualizzare gli esami disponibili e di effettuare prenotazioni.

## Obiettivo Principale
L'obiettivo principale del sistema è permettere agli studenti di consultare l'elenco degli esami disponibili e di prenotare quelli di loro interesse. I server intermedi gestiscono e inoltrano le richieste e le risposte tra le varie componenti del sistema.

## Componenti del Sistema
### 1. Client Studente
#### Funzione
Il Client Studente rappresenta l'interfaccia utente con cui gli studenti interagiscono. Consente loro di inviare richieste per visualizzare gli esami disponibili e per prenotare un esame specifico.
#### Modalità di Operazione
Il client invia le richieste al Server Segreteria, che funge da intermediario. Una volta che il Server Segreteria ha elaborato e inoltrato la risposta del Server Universitario, questa viene ritrasmessa al Client Studente per la visualizzazione o conferma della prenotazione.

### 2. Server Segreteria
#### Funzione
Il Server Segreteria agisce come intermediario tra il Client Studente e il Server Universitario.
#### Modalità di Operazione
Riceve le richieste dal Client Studente, le inoltra al Server Universitario e, successivamente, trasmette la risposta del Server Universitario indietro al Client Studente. Il Server Segreteria può anche inviare richieste al Server Universitario per la creazione di nuovi esami, oltre a gestire la normale comunicazione di visualizzazione e prenotazione degli esami.

### 3. Server Universitario
#### Funzione
Il Server Universitario è il componente centrale che gestisce tutte le informazioni sugli esami e le prenotazioni.
#### Modalità di Operazione
Riceve le richieste inoltrate dal Server Segreteria, le elabora e restituisce le risposte. È responsabile di mantenere aggiornati i dati relativi agli esami disponibili e alle prenotazioni effettuate dagli studenti. Questo server si occupa della logica principale del sistema e garantisce che tutte le informazioni siano correttamente conservate e accessibili.

## Installation
1. Clona la repository: `git clone https://github.com/NicolaMenale/RetiDiCalcolatori.git`
2. Navigare nella cartella del progetto: `cd RetiDiCalcolatori`
3. Compilare i file del progetto: `gcc -o server_universita server_universita.c` `gcc -o server_segreteria server_segreteria.c` `gcc -o studente studente.c`
4. Avvio degli eseguibili in ordine: `./server_universita` `./server_segreteria` `./studente 127.0.0.1`

## Documentazione
* [Traccia progetto](Traccia Esame.pdf)
* [Documentazione progetto](RELAZIONE RETI DI CALCOLATORI.pdf)

## Team
* [Menale Nicola](https://github.com/NicolaMenale)
* [Simpatico Giulio](https://github.com/ironmagic001)
