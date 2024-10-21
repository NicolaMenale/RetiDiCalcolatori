#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAXLINE 10240

// Macro per trovare il massimo tra due valori
#define max(x, y) ({typeof (x) x_ = (x); typeof (y) y_ = (y); x_ > y_ ? x_ : y_;})

// Funzione per scrivere completamente nel file descriptor
ssize_t FullWrite(int fd, const void *buf, size_t count) {
    size_t nleft = count;
    ssize_t nwritten;
    const char *ptr = buf;

    // Scrive completamente il buffer nel file descriptor
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR) {
                continue; // Riprovare se l'operazione è interrotta
            } else {
                return -1; // Errore
            }
        }
        nleft -= nwritten; // Aggiorna il numero di byte rimasti da scrivere
        ptr += nwritten;   // Aggiorna il puntatore al buffer
    }
    return count; // Restituisce il numero totale di byte scritti
}

void Richiesta_Prenotazione(int socket, int id_studente) {
    int scelta = 0; // Variabile per memorizzare la scelta dell'esame da parte dello studente
    int nwrite; // Variabile per memorizzare il numero di byte scritti sul socket
    char buffer[MAXLINE]; // Buffer per la richiesta di prenotazione
    int count; // Variabile per memorizzare il numero di esami disponibili

    // Richiede al server la lista degli esami disponibili
    printf("Scegliere l'opzione:\n");
    nwrite = FullWrite(socket, "MostraEsami_2", strlen("MostraEsami_2")); // Invia la richiesta al server
    
    if (nwrite < 0) {
        printf("Errore in scrittura\n"); // Gestione dell'errore se la scrittura fallisce
        return;
    }

    // Legge la risposta dal server
    char recvbuff[MAXLINE]; // Buffer per ricevere la risposta dal server
    ssize_t nread = read(socket, recvbuff, sizeof(recvbuff) - 1); // Legge dal socket
    if (nread < 0) {
        perror("Errore in lettura"); // Gestione dell'errore se la lettura fallisce
        return;
    }

    recvbuff[nread] = '\0'; // Termina la stringa con il carattere nullo
    sscanf(recvbuff, "Numero di esami: %d", &count); // Estrae il numero di esami disponibili dalla risposta

    fputs(recvbuff, stdout); // Stampa la risposta del server sulla console
    printf("\n");

    // Chiede all'utente di scegliere un esame
    if (scanf("%d", &scelta) != 1) { // Controlla se l'input è valido
        printf("Input non valido.\n");
        while (getchar() != '\n'); // Pulisce il buffer di input
    }
    if (scelta == 0) {
        return; // Se la scelta è 0, torna al menu principale
    }
    if (scelta > count) { // Controlla se la scelta è valida
        system("clear");
        printf("Input non valido.\n"); // Messaggio di errore per input non valido
        return;
    }

    // Invia la richiesta di prenotazione
    snprintf(buffer, sizeof(buffer), "%d:%d_2", id_studente, scelta - 1); // Prepara il buffer per la richiesta di prenotazione
    nwrite = FullWrite(socket, buffer, strlen(buffer)); // Invia la richiesta al server

    if (nwrite < 0) {
        printf("Errore in scrittura\n"); // Gestione dell'errore se la scrittura fallisce
        return;
    }

    // Legge la conferma della prenotazione
    nread = read(socket, recvbuff, sizeof(recvbuff) - 1); // Legge la conferma dal socket
    if (nread < 0) {
        perror("Errore in lettura"); // Gestione dell'errore se la lettura fallisce
        return;
    }
    system("clear"); // Pulisce lo schermo

    recvbuff[nread] = '\0'; // Termina la stringa con il carattere nullo
    fputs(recvbuff, stdout); // Stampa la risposta del server sulla console
    printf("\n");
}

void Esami_Disponibili(int socket) {
    int scelta; // Variabile per memorizzare la scelta dell'utente
    int nwrite; // Variabile per memorizzare il risultato della scrittura nel socket

    while (1) {
        // Mostra il menu delle opzioni degli esami
        printf("Date Esami:\n");
        printf("Scegliere l'esame:\n");
        printf("1) Reti di Calcolatori\n");
        printf("2) Algoritmi e Strutture Dati\n");
        printf("3) Programmazione 1\n");
        printf("4) Programmazione 2\n");
        printf("5) Programmazione 3\n");
        printf("6) Tecnologie Web\n");
        printf("0) Torna Indietro\n");

        // Leggi la scelta dell'utente
        if (scanf("%d", &scelta) != 1) {
            printf("Input non valido.\n");
            while (getchar() != '\n'); // Pulisce il buffer di input in caso di errore
            continue; // Torna all'inizio del ciclo
        }

        // Invia la richiesta di informazioni sugli esami in base alla scelta
        switch (scelta) {
            case 1:
                system("clear"); // Pulisce lo schermo
                nwrite = FullWrite(socket, "Reti_1", strlen("Reti_1")); // Invia la richiesta per "Reti di Calcolatori"
                break;
            case 2:
                system("clear");
                nwrite = FullWrite(socket, "Algoritmi_1", strlen("Algoritmi_1")); // Invia la richiesta per "Algoritmi e Strutture Dati"
                break;
            case 3:
                system("clear");
                nwrite = FullWrite(socket, "Prog1_1", strlen("Prog1_1")); // Invia la richiesta per "Programmazione 1"
                break;
            case 4:
                system("clear");
                nwrite = FullWrite(socket, "Prog2_1", strlen("Prog2_1")); // Invia la richiesta per "Programmazione 2"
                break;
            case 5:
                system("clear");
                nwrite = FullWrite(socket, "Prog3_1", strlen("Prog3_1")); // Invia la richiesta per "Programmazione 3"
                break;
            case 6:
                system("clear");
                nwrite = FullWrite(socket, "Web_1", strlen("Web_1")); // Invia la richiesta per "Tecnologie Web"
                break;
            case 0:
                system("clear");
                return; // Torna al menu principale
            default:
                system("clear");
                printf("Opzione non disponibile"); // Gestisce un'input non valido
                return;
        }

        // Controlla se la scrittura nel socket ha avuto successo
        if (nwrite < 0) {
            printf("Errore in scrittura\n"); // Stampa errore se scrittura fallisce
            return;
        }

        // Legge e stampa la lista degli esami disponibili dal server
        char recvbuff[MAXLINE]; // Buffer per memorizzare la risposta del server
        ssize_t nread = read(socket, recvbuff, sizeof(recvbuff) - 1); // Legge dal socket
        if (nread < 0) {
            perror("Errore in lettura"); // Stampa errore in caso di fallimento della lettura
            return;
        }
        recvbuff[nread] = '\0'; // Termina la stringa con il carattere null
        fputs(recvbuff, stdout); // Stampa la risposta del server
        printf("\n");
        return; // Esce dal ciclo
    }
}

void Student_Function(FILE *filein, int socket, int id_studente) {
    char sendbuff[MAXLINE + 1], recvbuff[MAXLINE + 1]; // Buffer per inviare e ricevere dati
    int scelta; // Variabile per memorizzare la scelta dell'utente
    fd_set fset; // Set di file descriptor per select
    int maxfd; // Variabile per il massimo file descriptor
    system("clear"); // Pulisce lo schermo

    while (1) {
        // Mostra il menu delle opzioni
        printf("Scegliere l'opzione:\n");
        printf("1) Esami disponibili per corso\n");
        printf("2) Richiedi prenotazione esame\n");
        printf("0) Uscire\n");

        // Inizializzazione del set di file descriptor per select
        FD_ZERO(&fset); // Pulisce il set
        FD_SET(socket, &fset);          // Aggiungi il socket al set
        FD_SET(fileno(filein), &fset);  // Aggiungi l'input standard (filein) al set
        maxfd = max(fileno(filein), socket) + 1; // Trova il massimo file descriptor

        // Attende che uno dei file descriptor nel set diventi pronto
        if (select(maxfd, &fset, NULL, NULL, NULL) < 0) {
            perror("Select error"); // Stampa errore se select fallisce
            return;
        }

        // Controlla se ci sono dati disponibili sull'input standard
        if (FD_ISSET(fileno(filein), &fset)) {
            // Leggi la scelta dell'utente
            if (scanf("%d", &scelta) != 1) {
                printf("Input non valido.\n"); // Gestisce l'input non valido
                while (getchar() != '\n'); // Pulisce il buffer di input
                continue; // Torna all'inizio del ciclo
            }

            // Gestione delle opzioni dell'utente
            switch (scelta) {
                case 1:
                    system("clear"); // Pulisce lo schermo
                    Esami_Disponibili(socket); // Mostra gli esami disponibili
                    break;
                case 2:
                    system("clear"); // Pulisce lo schermo
                    Richiesta_Prenotazione(socket, id_studente); // Richiede la prenotazione dell'esame
                    break;
                case 0:
                    return; // Esce dal loop e termina il programma
                default:
                    printf("Opzione non valida.\n"); // Messaggio di errore per opzione non valida
                    break;
            }
        }

        // Controlla se ci sono dati disponibili dal server tramite il socket
        if (FD_ISSET(socket, &fset)) {
            // Legge i dati dal socket
            ssize_t nread = read(socket, recvbuff, sizeof(recvbuff) - 1);
            if (nread == 0) {
                // Il server ha chiuso la connessione
                system("clear");
                printf("Il server ha chiuso la connessione.\n");
                return; // Esce dalla funzione
            }
            if (nread < 0) {
                perror("Errore in lettura dal socket"); // Stampa errore in caso di lettura fallita
                return; // Esce dalla funzione
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int sock; // Variabile per il socket
    struct sockaddr_in serv_add; // Struttura per l'indirizzo del server

    // Inizializzazione del generatore di numeri casuali
    srand(time(NULL)); // Seed per generare numeri casuali
    int id_studente = rand(); // Assegna un ID casuale allo studente

    // Verifica che l'argomento dell'indirizzo IP sia fornito
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <server-ip-address>\n", argv[0]); // Messaggio di errore se non ci sono argomenti corretti
        return 1; // Esce con errore
    }

    // Apertura del socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error"); // Stampa errore se la creazione del socket fallisce
        return 1; // Esce con errore
    }

    // Imposta i campi della struttura sockaddr_in per la connessione al server
    serv_add.sin_family = AF_INET; // Imposta la famiglia di indirizzi (IPv4)
    serv_add.sin_port = htons(54321); // Porta del server, convertita in formato di rete

    // Converte l'indirizzo IP da stringa a formato binario
    if (inet_pton(AF_INET, argv[1], &serv_add.sin_addr) <= 0) {
        perror("Address creation error"); // Stampa errore se l'indirizzo IP non è valido
        close(sock); // Chiude il socket
        return 1; // Esce con errore
    }

    // Connessione al server
    if (connect(sock, (struct sockaddr *)&serv_add, sizeof(serv_add)) < 0) {
        perror("Connection error"); // Stampa errore se la connessione fallisce
        close(sock); // Chiude il socket
        return 1; // Esce con errore
    }

    // Funzioni dello studente per interagire con il server
    Student_Function(stdin, sock, id_studente); // Chiama la funzione per gestire le operazioni dello studente

    // Chiusura del socket alla fine
    close(sock); // Chiude il socket
    return 0; // Termina il programma con successo
}