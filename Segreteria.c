#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>

#define PORT 54321 // Porta su cui il server ascolterà le connessioni
#define MAXLINE 10240 // Dimensione massima dei buffer

// Socket globale per comunicazione con l'università
int university_socket = -1;
int server_socket = -1; // Socket del server
int keep_running = 1; // Variabile globale per controllare l'esecuzione del server

// Mutex per la sincronizzazione dell'accesso al socket universitario
pthread_mutex_t university_socket_mutex = PTHREAD_MUTEX_INITIALIZER;

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

// Verifica se una stringa contiene solo numeri
bool is_digit_string(const char *str, size_t length) {
    // Scorre ogni carattere della stringa fino alla lunghezza specificata
    for (size_t i = 0; i < length; i++) {
        // Se uno dei caratteri non è un numero, restituisce false
        // isdigit(): È una funzione della libreria <ctype.h> che restituisce un valore non-zero se il carattere passato è una cifra (da '0' a '9'), altrimenti restituisce 0.
        if (!isdigit(str[i])) {
            return false;
        }
    }
    // Se tutti i caratteri sono numeri, restituisce true
    return true;
}

// Verifica se una data è valida considerando i giorni per ogni mese e anno bisestile
bool is_valid_day(int day, int month, int year) {
    // Controlla se il mese è valido (tra 1 e 12)
    if (month < 1 || month > 12) {
        return false;
    }
    
    // Controlla se il giorno è valido (tra 1 e 31)
    if (day < 1 || day > 31) {
        return false;
    }

    // Mesi con 30 giorni: aprile, giugno, settembre, novembre
    if (month == 4 || month == 6 || month == 9 || month == 11) {
        if (day > 30) {
            return false;
        }
    }

    // Febbraio: gestisce i casi di anno bisestile e non bisestile
    if (month == 2) {
        // Controlla se l'anno è bisestile
        bool is_leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
        // Se è bisestile, massimo 29 giorni; altrimenti massimo 28
        if ((is_leap && day > 29) || (!is_leap && day > 28)) {
            return false;
        }
    }

    // Se tutte le condizioni sono rispettate, il giorno è valido
    return true;
}

// Normalizza il formato del giorno e del mese (1 -> 01, 12 -> 12)
void normalize_date_parts(char *day_str, char *month_str) {
    // La funzione atoi() (della libreria <stdlib.h>) converte le stringhe day_str e month_str in interi.
    int day = atoi(day_str);
    int month = atoi(month_str);

    // Normalizzazione del giorno: se il giorno è tra 1 e 9, aggiungi uno zero davanti
    if (day >= 1 && day <= 9) {
        snprintf(day_str, 3, "0%d", day);  // Scrivi il giorno formattato come "0X"
    } else {
        snprintf(day_str, 3, "%d", day);   // Scrivi il giorno formattato come "X"
    }

    // Normalizzazione del mese: se il mese è tra 1 e 9, aggiungi uno zero davanti
    if (month >= 1 && month <= 9) {
        snprintf(month_str, 3, "0%d", month);  // Scrivi il mese formattato come "0X"
    } else {
        snprintf(month_str, 3, "%d", month);   // Scrivi il mese formattato come "X"
    }
}

// Verifica se una data è valida e normalizza la data nel formato "dd-mm-yyyy"
bool validate_date(const char *day_str, const char *month_str, const char *year_str) {
    // Creazione di copie locali di giorno e mese con lunghezza massima di 2 caratteri
    char day_copy[3];
    char month_copy[3];
    strncpy(day_copy, day_str, 2);  // Copia i primi 2 caratteri di day_str
    strncpy(month_copy, month_str, 2);  // Copia i primi 2 caratteri di month_str
    day_copy[2] = '\0';  // Terminazione della stringa
    month_copy[2] = '\0';  // Terminazione della stringa

    // Normalizza le parti della data (aggiunge uno zero davanti se necessario)
    normalize_date_parts(day_copy, month_copy);

    // Verifica che i campi siano numerici e abbiano lunghezza corretta
    if (!is_digit_string(day_copy, 2) || !is_digit_string(month_copy, 2) || !is_digit_string(year_str, 4)) {
        return false;
    }

    // Converti le stringhe in numeri interi
    int day = atoi(day_copy);
    int month = atoi(month_copy);
    int year = atoi(year_str);

    // Ottieni il tempo corrente
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);  // Converte in ora locale
    
    // Ottieni l'anno, mese e giorno corrente
    int current_year = tm_info->tm_year + 1900;  // tm_year è l'anno dal 1900
    int current_month = tm_info->tm_mon + 1;  // tm_mon è il mese da 0 (gennaio)
    int current_day = tm_info->tm_mday;  // Giorno del mese

    // Controllo della validità della data rispetto a quella attuale
    if (year < current_year) {
        return false;  // Anno precedente all'anno corrente
    } else if (year == current_year) {
        if (month < current_month) {
            return false;  // Mese precedente al mese corrente nello stesso anno
        } else if (month == current_month) {
            if (day <= current_day) {
                return false;  // Giorno precedente o uguale a quello corrente nello stesso mese
            }
        }
    }

    // Verifica se il giorno è valido per il mese e l'anno forniti
    return is_valid_day(day, month, year);
}

// Elabora e formatta una data nel formato "dd-mm-yyyy"
char* process_date(const char *giorno, const char *mese, const char *anno, int *should_return) {
    static char formatted_date[11]; // Spazio sufficiente per "dd-mm-yyyy\0"
    char formatted_day[3], formatted_month[3];

    // Normalizza giorno e mese con zeri iniziali
    snprintf(formatted_day, sizeof(formatted_day), "%02d", atoi(giorno));
    snprintf(formatted_month, sizeof(formatted_month), "%02d", atoi(mese));

    // Verifica se la data è valida
    if (!validate_date(formatted_day, formatted_month, anno)) {
        printf("Data non valida.\n"); // Indica che la data non è valida
        *should_return = 1;
        return NULL;
    }

    // Unisci giorno, mese e anno in una singola stringa
    snprintf(formatted_date, sizeof(formatted_date), "%s-%s-%s", formatted_day, formatted_month, anno);
    *should_return = 0; // Data valida
    return formatted_date;
}

// Gestisce l'inserimento e l'invio della data per un esame
void handle_exam_date(const char *exam_name, const char *exam_code, int exam_type, int *should_return, int university_socket) {
    char giorno[3], mese[3], anno[5];  // Buffer per giorno, mese, anno
    char buffer[MAXLINE];  // Buffer per l'invio dei dati
    int nwrite;  // Numero di byte scritti

    // Pulisce il terminale e stampa il nome dell'esame
    system("clear");
    printf("Inserimento data di esame di %s: \n", exam_name);

    // Prepara il messaggio da inviare con codice esame seguito da "_1"
    strcpy(buffer, exam_code);
    strcat(buffer, "_1");

    // Invio del messaggio al server
    nwrite = FullWrite(university_socket, buffer, strlen(buffer));
    if (nwrite < 0) {
        printf("Errore in scrittura\n");
        return;
    }

    // Lettura della lista degli esami dal server
    char recvbuff[MAXLINE];  // Buffer per ricevere i dati
    ssize_t nread = read(university_socket, recvbuff, sizeof(recvbuff) - 1);
    if (nread < 0) {
        perror("Errore in lettura");
        return;
    }
    recvbuff[nread] = '\0';  // Termina la stringa ricevuta
    fputs(recvbuff, stdout);  // Stampa la risposta del server

    // Richiesta all'utente di inserire giorno, mese e anno
    printf("Inserisci il giorno: ");
    scanf("%2s", giorno);  // Legge massimo 2 caratteri per il giorno

    printf("Inserisci il mese: ");
    scanf("%2s", mese);  // Legge massimo 2 caratteri per il mese

    printf("Inserisci l'anno: ");
    scanf("%4s", anno);  // Legge massimo 4 caratteri per l'anno

    // Verifica se la data inserita è valida
    if (!validate_date(giorno, mese, anno)) {
        system("clear");  // Pulisce lo schermo
        printf("Data non valida.\n");
        *should_return = 1;  // Indica che bisogna tornare al menu principale
        return;
    }

    // Processa la data normalizzandola e formattandola
    char *date = process_date(giorno, mese, anno, should_return);
    if (*should_return == 1) {
        return;  // Se ci sono errori nella data, ritorna immediatamente
    }

    // Pulisce lo schermo e conferma l'inserimento dell'esame
    system("clear");
    printf("SEGRETERIA) Esame: %s Data: %s Aggiunto\n", exam_name, date);

    // Prepara il messaggio da inviare contenente il codice esame, la data e il tipo di esame
    snprintf(buffer, sizeof(buffer), "%s:%s_%d", exam_code, date, exam_type);

    // Invia il messaggio al server
    nwrite = FullWrite(university_socket, buffer, strlen(buffer));
    if (nwrite < 0) {
        printf("Errore in scrittura: %s\n", strerror(errno));
        return;
    }

    // Lettura della risposta del server
    nread = read(university_socket, recvbuff, sizeof(recvbuff) - 1);
    if (nread < 0) {
        perror("Errore in lettura");
        return;
    }
    recvbuff[nread] = '\0';  // Termina la stringa ricevuta
    fputs(recvbuff, stdout);  // Stampa la risposta del server

    *should_return = 1;  // Imposta il flag per tornare al menu principale
}

// Funzione per l'inserimento dell'esame tramite l'interfaccia utente
void Inserisci_esame(int university_socket, int *should_return) {
    int scelta;  // Variabile per memorizzare la scelta dell'utente

    while (1) {
        // Pulisce lo schermo e mostra le opzioni degli esami
        system("clear");
        printf("Date Esami:\n");
        printf("Scegliere l'esame:\n");
        printf("1) Reti di Calcolatori\n");
        printf("2) Algoritmi e Strutture Dati\n");
        printf("3) Programmazione 1\n");
        printf("4) Programmazione 2\n");
        printf("5) Programmazione 3\n");
        printf("6) Tecnologie Web\n");
        printf("0) Torna Indietro\n");

        // Legge la scelta dell'utente con controllo sull'input
        if (scanf("%d", &scelta) != 1) {
            printf("Input non valido.\n");
            while (getchar() != '\n');  // Pulisce il buffer di input
            continue;  // Torna all'inizio del ciclo per ripetere la scelta
        }

        // Gestisce la scelta dell'utente tramite uno switch case
        switch (scelta) {
            case 1:
                handle_exam_date("Reti di Calcolatori", "Reti", 3, should_return, university_socket);
                break;
            case 2:
                handle_exam_date("Algoritmi e Strutture Dati", "Algoritmi", 3, should_return, university_socket);
                break;
            case 3:
                handle_exam_date("Programmazione 1", "Prog1", 3, should_return, university_socket);
                break;
            case 4:
                handle_exam_date("Programmazione 2", "Prog2", 3, should_return, university_socket);
                break;
            case 5:
                handle_exam_date("Programmazione 3", "Prog3", 3, should_return, university_socket);
                break;
            case 6:
                handle_exam_date("Tecnologie Web", "Web", 3, should_return, university_socket);
                break;
            case 0:
                *should_return = 1;  // Indica di tornare al menu principale
                return;  // Esce dalla funzione
            default:
                system("clear");
                printf("Opzione non disponibile\n");
                *should_return = 1;  // Opzione non valida, torna al menu principale
                return;
        }

        // Se `should_return` è settato, esce dalla funzione
        if (*should_return) {
            return;
        }
    }
}


// Funzione per gestire le richieste dei client
// Funzione per gestire le richieste dei client
void *handle_client(void *arg) {
    int client_socket = *(int *)arg;  // Recupera il socket del client passato come argomento
    free(arg);  // Libera la memoria allocata per il socket client
    char buffer[MAXLINE];  // Buffer per memorizzare i dati in entrata e uscita
    int n;  // Variabile per memorizzare il numero di byte letti/scritti
    struct sockaddr_in university_addr;  // Struttura per l'indirizzo del server universitario

    // Creazione del socket per la connessione all'università
    pthread_mutex_lock(&university_socket_mutex);  // Lock del mutex per la sincronizzazione
    if (university_socket <= 0) {  // Verifica se il socket universitario è valido
        // Crea un nuovo socket se non esiste
        if ((university_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Errore nella creazione del socket università");  // Stampa errore se fallisce
            pthread_mutex_unlock(&university_socket_mutex);  // Sblocca il mutex
            close(client_socket);  // Chiude il socket del client
            pthread_exit(NULL);  // Termina il thread
        }

        // Inizializza l'indirizzo del server universitario
        memset(&university_addr, 0, sizeof(university_addr));  // Pulisce la struttura
        university_addr.sin_family = AF_INET;  // Imposta il family dell'indirizzo
        university_addr.sin_port = htons(12345);  // Imposta la porta del server universitario
        university_addr.sin_addr.s_addr = INADDR_ANY;  // Accetta connessioni su tutte le interfacce

        // Stabilisce la connessione al server universitario
        if (connect(university_socket, (struct sockaddr *)&university_addr, sizeof(university_addr)) < 0) {
            perror("Errore di connessione con l'università");  // Stampa errore se fallisce
            close(university_socket);  // Chiude il socket universitario
            university_socket = -1;  // Segna il socket come non valido
            pthread_mutex_unlock(&university_socket_mutex);  // Sblocca il mutex
            close(client_socket);  // Chiude il socket del client
            pthread_exit(NULL);  // Termina il thread
        }
    }
    pthread_mutex_unlock(&university_socket_mutex);  // Sblocca il mutex dopo aver creato la connessione

    // Gestisce la comunicazione con il client
    while ((n = read(client_socket, buffer, MAXLINE - 1)) > 0) {  // Legge la richiesta dal client
        buffer[n] = '\0';  // Termina la stringa
        printf("Richiesta dal client: %s\n", buffer);  // Stampa la richiesta del client

        // Invia la richiesta all'università
        pthread_mutex_lock(&university_socket_mutex);  // Lock del mutex per l'accesso al socket universitario
        if (write(university_socket, buffer, strlen(buffer)) < 0) {  // Scrive la richiesta al socket universitario
            perror("Errore in scrittura alla università");  // Stampa errore se fallisce
            pthread_mutex_unlock(&university_socket_mutex);  // Sblocca il mutex
            close(client_socket);  // Chiude il socket del client
            pthread_exit(NULL);  // Termina il thread
        }

        // Riceve la risposta dall'università
        if ((n = read(university_socket, buffer, MAXLINE - 1)) <= 0) {  // Legge la risposta dal socket universitario
            perror("Errore nella lettura dalla università");  // Stampa errore se fallisce
            pthread_mutex_unlock(&university_socket_mutex);  // Sblocca il mutex
            close(client_socket);  // Chiude il socket del client
            pthread_exit(NULL);  // Termina il thread
        }
        buffer[n] = '\0';  // Termina la stringa
        pthread_mutex_unlock(&university_socket_mutex);  // Sblocca il mutex dopo aver letto la risposta

        // Invia la risposta dall'università allo studente
        if (write(client_socket, buffer, strlen(buffer)) < 0) {  // Scrive la risposta al socket del client
            perror("Errore in scrittura allo studente");  // Stampa errore se fallisce
            close(client_socket);  // Chiude il socket del client
            pthread_exit(NULL);  // Termina il thread
        }
    }

    close(client_socket);  // Chiude il socket del client alla fine della comunicazione
    pthread_exit(NULL);  // Termina il thread
}


// Funzione per gestire l'interazione terminale per aggiungere esami
// Funzione per gestire il menu degli esami
void *manage_exams() {
    system("clear");  // Pulisce lo schermo all'inizio

    while (keep_running) {  // Ciclo principale finché il programma deve continuare
        printf("Scegliere l'opzione:\n");
        printf("1) Aggiungi Esame\n");
        printf("0) Esci\n");

        int scelta;
        if (scanf("%d", &scelta) != 1) {  // Legge la scelta dell'utente
            printf("Input non valido.\n");
            while (getchar() != '\n'); // Pulisce il buffer di input
            continue;
        }

        while (getchar() != '\n'); // Pulisce il buffer di input

        switch (scelta) {
            case 1:
                pthread_mutex_lock(&university_socket_mutex);  // Lock del mutex per il socket universitario
                if (university_socket <= 0) {  // Verifica se il socket universitario non è già attivo
                    struct sockaddr_in university_addr;
                    // Creazione del socket per la connessione all'università
                    if ((university_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                        perror("Errore nella creazione del socket università");
                        pthread_mutex_unlock(&university_socket_mutex);
                        continue;
                    }

                    // Inizializzazione dell'indirizzo del server universitario
                    memset(&university_addr, 0, sizeof(university_addr));
                    university_addr.sin_family = AF_INET;
                    university_addr.sin_port = htons(12345);  // Porta del server universitario
                    university_addr.sin_addr.s_addr = INADDR_ANY;

                    // Connessione al server universitario
                    if (connect(university_socket, (struct sockaddr *)&university_addr, sizeof(university_addr)) < 0) {
                        perror("Errore di connessione con l'università");
                        close(university_socket);
                        university_socket = -1;
                        pthread_mutex_unlock(&university_socket_mutex);
                        continue;
                    }
                }

                int should_return = 0;
                Inserisci_esame(university_socket, &should_return);  // Chiamata alla funzione per inserire un esame
                if (should_return) {  // Se dovrebbe tornare indietro, sblocca il mutex e continua
                    pthread_mutex_unlock(&university_socket_mutex);
                    continue;
                }
                break;

            case 0:
                keep_running = 0; // Imposta la variabile globale per terminare il ciclo
                pthread_mutex_lock(&university_socket_mutex);  // Lock del mutex per il socket universitario
                if (university_socket >= 0) {  // Verifica se il socket universitario è attivo
                    close(university_socket);  // Chiude il socket universitario
                    university_socket = -1;
                }
                pthread_mutex_unlock(&university_socket_mutex);  // Sblocca il mutex

                printf("Uscita dal programma.\n");
                // Chiude il socket del server per uscire dal ciclo di accettazione
                if (server_socket >= 0) {
                    close(server_socket);
                    server_socket = -1;
                }
                pthread_exit(NULL);  // Termina il thread
                break;

            default:
                printf("Opzione non valida.\n");
                break;
        }
    }

    pthread_exit(NULL);  // Termina il thread
}


int main() {
    struct sockaddr_in server_addr, client_addr;  // Strutture per l'indirizzo del server e del client
    socklen_t client_len = sizeof(client_addr);
    pthread_t thread_id, exam_thread;

    // Creazione del socket del server
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Errore nella creazione del socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));  // Inizializza la struttura a zero
    server_addr.sin_family = AF_INET;  // Utilizza IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Accetta connessioni da qualsiasi indirizzo
    server_addr.sin_port = htons(PORT);  // Imposta la porta del server

    // Associa il socket all'indirizzo e alla porta specificati
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Errore nel bind");
        close(server_socket);
        exit(1);
    }

    // Mette il server in ascolto per le connessioni in entrata
    if (listen(server_socket, 10) < 0) {
        perror("Errore nel listen");
        close(server_socket);
        exit(1);
    }

    printf("Segreteria in ascolto sulla porta %d...\n", PORT);

    // Crea il thread per gestire l'inserimento degli esami
    if (pthread_create(&exam_thread, NULL, manage_exams, NULL) != 0) {
        perror("Errore nella creazione del thread per l'inserimento degli esami");
        close(server_socket);
        exit(1);
    }
    pthread_detach(exam_thread);  // Scollega il thread per gestire la terminazione

    // Ciclo principale per accettare e gestire i client
    while (keep_running) {
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            if (!keep_running) break; // Se il server deve terminare, esci dal ciclo
            perror("Errore nell'accept");
            continue;
        }

        int *client_sock_ptr = malloc(sizeof(int));  // Alloca memoria per il socket del client
        if (client_sock_ptr == NULL) {
            perror("Errore nell'allocazione della memoria");
            close(client_socket);
            continue;
        }
        *client_sock_ptr = client_socket;  // Assegna il socket al puntatore

        // Crea un thread per gestire il client
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, client_sock_ptr) != 0) {
            perror("Errore nella creazione del thread");
            close(client_socket);
            free(client_sock_ptr);
            continue;
        }
        pthread_detach(client_thread);  // Scollega il thread per la gestione della terminazione
    }

    // Chiude il socket del server e aspetta la terminazione dei thread
    if (server_socket >= 0) {
        close(server_socket);  // Chiude il socket del server
    }

    printf("Programma terminato.\n");

    return 0;  // Termina il programma con successo
}

