// Inclusione delle librerie necessarie
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define PORT 12345              // Porta su cui il server universitario ascolta
#define MAXLINE 10240           // Dimensione massima del buffer
#define MAX_NAME_LENGTH 256    // Lunghezza massima per ID e nome dell'esame
#define MAX_RESERVATION 256   // Dimensione massima per le prenotazioni
#define MAX_STUDENTS 100      // Numero massimo di studenti
#define MAX_EXAM 100      // Numero massimo di esami

// Struttura per memorizzare le informazioni sugli esami
typedef struct {
    char id[MAX_NAME_LENGTH];     // ID dell'esame
    char name[MAX_NAME_LENGTH];   // Nome dell'esame
    char date[MAX_NAME_LENGTH];   // Data dell'esame
    int num_exam;                 // Numero totale degli esami
} Exam;

// Struttura per memorizzare le prenotazioni degli esami
typedef struct {
    int student_id;               // ID dello studente
    Exam exam;                    // Informazioni sull'esame prenotato
    int num_prenotazioni;         // Numero di prenotazioni per questo esame
} Prenotazioni;

// Funzione per scrivere completamente nel file descriptor
ssize_t FullWrite(int fd, const void *buf, size_t count) {
    size_t nleft = count;
    ssize_t nwritten;
    const char *ptr = buf;

    // Scrive il buffer nel file descriptor in modo completo
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
    return count;
}

// Funzione per inizializzare gli esami con dati predefiniti
void init_exam(Exam *exam) {
    // Inizializzazione degli esami
    strcpy(exam[0].id, "Reti");   
    strcpy(exam[0].name, "Reti di Calcolatori");
    strcpy(exam[0].date, "9-10-2024");

    strcpy(exam[1].id, "Algoritmi");   
    strcpy(exam[1].name, "Algoritmi e Strutture Dati");
    strcpy(exam[1].date, "15-9-2024");

    strcpy(exam[2].id, "Prog1");   
    strcpy(exam[2].name, "Programmazione 1");
    strcpy(exam[2].date, "1-10-2024");

    strcpy(exam[3].id, "Reti");   
    strcpy(exam[3].name, "Reti di Calcolatori");
    strcpy(exam[3].date, "20-8-2024");

    strcpy(exam[4].id, "Prog2");   
    strcpy(exam[4].name, "Programmazione 2");
    strcpy(exam[4].date, "10-10-2024");

    strcpy(exam[5].id, "Prog3");   
    strcpy(exam[5].name, "Programmazione 3");
    strcpy(exam[5].date, "1-11-2024");

    strcpy(exam[6].id, "Web");   
    strcpy(exam[6].name, "Tecnologie Web");
    strcpy(exam[6].date, "1-9-2024");

    exam->num_exam = 7; // Imposta il numero totale degli esami
}

// Funzione per gestire le richieste di prenotazione degli esami
void Richiesta_Prenotazione(char *buffer, char *response, Exam exam[], Prenotazioni prenotazioni[]) {
    int id_studente;
    int scelta;
    int count = 0;

    system("clear"); // Pulisce lo schermo del terminale

    // Se la richiesta è per mostrare gli esami disponibili
    if (strcmp(buffer, "MostraEsami_2") == 0) {
        // Mostra tutte le date disponibili per gli esami
        char temp[MAXLINE] = "Date Disponibili:\n";
        for (int i = 0; i < exam->num_exam; i++) {
            printf("%d) %s, Data: %s\n", i + 1, exam[i].name, exam[i].date);
            snprintf(temp + strlen(temp), MAXLINE - strlen(temp), "%d) %s, Data: %s\n", i + 1, exam[i].name, exam[i].date);
            count++;
        }
        snprintf(response, MAXLINE, "Numero di esami: %d\n%s", count, temp);
    } else {
        // Estrai l'ID dello studente e la scelta dell'esame dalla richiesta
        sscanf(buffer, "%d:%d_2", &id_studente, &scelta);

        // Controlla se l'esame è già stato prenotato dallo studente
        for (size_t i = 0; i < prenotazioni->num_prenotazioni; i++) {
            if ((prenotazioni[i].student_id == id_studente) && strcmp(prenotazioni[i].exam.id, exam[scelta].id) == 0 && strcmp(prenotazioni[i].exam.date, exam[scelta].date) == 0) {
                snprintf(response, MAXLINE, "Prenotazione Già Effettuata");
                return;
            }
        }

        // Registra la prenotazione per lo studente
        prenotazioni[prenotazioni->num_prenotazioni].student_id = id_studente;
        prenotazioni[prenotazioni->num_prenotazioni].exam = exam[scelta];
        prenotazioni->num_prenotazioni++;

        // Mostra tutte le prenotazioni effettuate
        printf("Elenco delle prenotazioni:\n");
        for (int i = 0; i < prenotazioni->num_prenotazioni; i++) {
            printf("%d) ID Studente: %d, Esame: %s, Data: %s\n", i + 1, prenotazioni[i].student_id, prenotazioni[i].exam.name, prenotazioni[i].exam.date);
        }

        // Conta le prenotazioni per l'esame scelto dallo studente
        count = 0;
        for (int i = 0; i < prenotazioni->num_prenotazioni; i++) {
            if ((prenotazioni[i].student_id == id_studente) && strcmp(prenotazioni[i].exam.id, exam[scelta].id) == 0) {
                count++;
            }
        }
        snprintf(response, MAXLINE, "Esame Prenotato. Numero Prenotazioni per l'esame %s: %d", exam[scelta].name, count);
    }
}


// Funzione per mostrare le date disponibili per un determinato esame
void Esami_Disponibili(char *buffer, char *response, Exam exam[]) {
    size_t len = strlen(buffer);

    // Estrai l'ID dell'esame dal buffer, rimuovendo gli ultimi due caratteri (es. '\n' e '\0')
    char esame[MAX_NAME_LENGTH];
    strncpy(esame, buffer, len - 2);
    esame[len - 2] = '\0';
    
    printf("ID Esame: %s\n", esame);

    // Costruisci la risposta con i dettagli degli esami disponibili
    char temp[MAXLINE] = "Date Disponibili:\n";

    for (int i = 0; i < exam->num_exam; i++) {
        if (strcmp(exam[i].id, esame) == 0) {
            printf("%d) %s, Data: %s\n", i + 1, exam[i].name, exam[i].date);
            snprintf(temp + strlen(temp), MAXLINE - strlen(temp), "%d) %s, Data: %s\n", i + 1, exam[i].name, exam[i].date);
        }
    }

    // Copia il risultato nella risposta da restituire al chiamante
    snprintf(response, MAXLINE, "%s", temp);
}


// Funzione per aggiungere un nuovo esame
void aggiungi_esame(Exam *exam, int *num_exam, const char *id, const char *name, const char *date, char *response) {
    if (*num_exam < MAX_RESERVATION) {
        strncpy(exam[*num_exam].id, id, MAX_NAME_LENGTH);
        strncpy(exam[*num_exam].name, name, MAX_NAME_LENGTH);
        strncpy(exam[*num_exam].date, date, MAX_NAME_LENGTH);
        snprintf(response, MAXLINE, "SERVER) Appello aggiunto: ID=%s, Data=%s\n", id, date);
        printf("Appello aggiunto: ID=%s, Data=%s\n", id, date);
        (*num_exam)++;
    } else {
        snprintf(response, MAXLINE, "Impossibile aggiungere l'appello, limite raggiunto.\n");
        printf("Impossibile aggiungere l'appello, limite raggiunto.\n");
    }
}

// Funzione per gestire l'aggiunta di un nuovo esame
void Aggiungi_Esame(char *buffer, char *response, Exam exam[]) {
    char id[MAX_NAME_LENGTH];
    char date[MAX_NAME_LENGTH];
    char suffix[MAXLINE];
    
    // Estrai i dettagli dell'esame dal buffer
    if (sscanf(buffer, "%[^:]:%[^_]_%s", id, date, suffix) == 3 && strcmp(suffix, "3") == 0) {
        // Controlla se la data per l'esame ID già esiste
        for (int i = 0; i < exam->num_exam; i++) {
            if (strcmp(exam[i].id, id) == 0 && strcmp(exam[i].date, date) == 0) {
                snprintf(response, MAXLINE, "Data %s per l'esame ID=%s già esistente.\n", date, id);
                printf("%s", response);
                return;
            }
        }
        
        // Definisci i nomi degli esami e i loro ID
        const char *names[] = {
            "Reti di Calcolatori", 
            "Algoritmi e Strutture Dati",
            "Programmazione 1",
            "Programmazione 2",
            "Programmazione 3",
            "Tecnologie Web"
        };
        const char *ids[] = {"Reti", "Algoritmi", "Prog1", "Prog2", "Prog3", "Web"};
        
        // Aggiungi il nuovo esame se l'ID è valido
        for (int i = 0; i < 6; i++) {
            if (strcmp(id, ids[i]) == 0) {
                aggiungi_esame(exam, &exam->num_exam, id, names[i], date, response);
                return;
            }
        }
        
        // Mostra tutte le date degli esami disponibili nel caso l'ID non corrisponda
        char temp[MAXLINE] = "Date Disponibili:\n";
        for (int i = 0; i < exam->num_exam; i++) {
            printf("ID: %s, Name: %s, Data: %s\n", exam[i].id, exam[i].name, exam[i].date);
        }
    }
}


// Funzione per gestire la comunicazione con il client
void handle_client(int client_socket, Exam exam[], Prenotazioni prenotazioni[]) {
    char buffer[MAXLINE];
    int n;
    
    // Ciclo per leggere i dati dal client
    while ((n = read(client_socket, buffer, MAXLINE - 1)) > 0) {
        buffer[n] = '\0'; // Aggiungi il terminatore di stringa
        printf("Received: %s\n", buffer);

        char last_char = buffer[strlen(buffer) - 1]; // Ottieni l'ultimo carattere del buffer

        // Preparazione della risposta
        char response[MAXLINE];
        
        // Gestisci le richieste in base all'ultimo carattere del buffer
        if (last_char == '1') {
            Esami_Disponibili(buffer, response, exam); // Richiesta di esami disponibili
        } else if (last_char == '2') {
            Richiesta_Prenotazione(buffer, response, exam, prenotazioni); // Prenotazione esame
        } else if (last_char == '3') {
            Aggiungi_Esame(buffer, response, exam); // Aggiungi un nuovo esame
        }

        // Invia la risposta al client
        if (FullWrite(client_socket, response, strlen(response)) < 0) {
            perror("Write error");
            close(client_socket);
            return;
        }
    }
    close(client_socket); // Chiudi la connessione con il client
}

// Funzione principale
int main() {
    int server_socket, client_socket; // Socket del server e del client
    struct sockaddr_in server_addr, client_addr; // Strutture per gli indirizzi del server e del client
    socklen_t client_len = sizeof(client_addr); // Lunghezza dell'indirizzo del client
    Prenotazioni prenotazioni[MAX_RESERVATION]; // Array di prenotazioni
    prenotazioni->num_prenotazioni = 0; // Inizializza il numero di prenotazioni
    Exam exam[MAX_EXAM]; // Array di esami
    init_exam(exam); // Inizializza gli esami

    // Creazione del socket del server
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation error"); // Stampa errore se la creazione del socket fallisce
        exit(1); // Esce con errore
    }

    // Configurazione dell'indirizzo del server
    memset(&server_addr, 0, sizeof(server_addr)); // Pulisce la struttura dell'indirizzo
    server_addr.sin_family = AF_INET; // Imposta la famiglia di indirizzi (IPv4)
    server_addr.sin_addr.s_addr = INADDR_ANY; // Accetta connessioni da qualsiasi indirizzo
    server_addr.sin_port = htons(PORT); // Imposta la porta del server

    // Associa l'indirizzo al socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind error"); // Stampa errore se il bind fallisce
        close(server_socket); // Chiude il socket
        exit(1); // Esce con errore
    }

    // Metti il socket in ascolto delle connessioni in entrata
    if (listen(server_socket, 10) < 0) {
        perror("Listen error"); // Stampa errore se il listen fallisce
        close(server_socket); // Chiude il socket
        exit(1); // Esce con errore
    }
    
    system("clear"); // Pulisce lo schermo
    printf("Server Universitario in ascolto sulla porta %d...\n", PORT); // Stampa stato del server

    // Ciclo principale per accettare e gestire le connessioni dei client
    while (1) {
        // Accetta la connessione in entrata da un client
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Accept error"); // Stampa errore se l'accept fallisce
            return 0; // Esce dal programma
        }
        
        // Gestisce la connessione del client tramite la funzione handle_client
        handle_client(client_socket, exam, prenotazioni); // Invoca la funzione per gestire il client
    }

    close(server_socket); // Chiudi il socket del server (questa linea non sarà mai raggiunta)
    return 0; // Termina il programma
}

