#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>
#include <unistd.h>

#define MAX_LINE 256
#define MAX_QUESTIONS 20
#define PROGRESS_BAR_LENGTH 20

typedef struct {
    char question[MAX_LINE];
    int points[2]; // [0] - ekstrawertyk, [1] - introwertyk
} YesNoQuestion;

typedef struct {
    char question[MAX_LINE];
    char answers[4][MAX_LINE];
    int points[4][2]; // [4 odpowiedzi][2 kategorie: optymista, pesymista]
} MultiChoiceQuestion;

// Prototypy funkcji
void runYesNoQuiz(const char *nickname);
void runMultiChoiceQuiz(const char *nickname);
void showResults();
void clearResults();
void displayProgressBar(const char *label, int percentage);
void displayQuestionProgressBar(int current, int total);
void saveResults(const char *nickname, const char *quizType, int extrovertPercent, int introvertPercent);


void enableANSIColors() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

// Funkcja do wczytywania pytań tak/nie z pliku
void loadYesNoQuestions(const char *filename, YesNoQuestion questions[], int *questionCount) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Nie mozna otworzyc pliku");
        exit(1);
    }

    char line[MAX_LINE];
    int qIndex = 0;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Usuwanie znaku nowej linii

        char *token = strtok(line, "|");
        strcpy(questions[qIndex].question, token);
        questions[qIndex].points[0] = atoi(strtok(NULL, "|")); // Punkty dla ekstrawertyka
        questions[qIndex].points[1] = atoi(strtok(NULL, "|")); // Punkty dla introwertyka

        qIndex++;
    }

    *questionCount = qIndex;
    fclose(file);
}

// Funkcja do wczytywania pytań wielokrotnego wyboru z pliku
void loadMultiChoiceQuestions(const char *filename, MultiChoiceQuestion questions[], int *questionCount) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Nie mozna otworzyc pliku");
        exit(1);
    }

    char line[MAX_LINE];
    int qIndex = -1;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Usuwanie znaku nowej linii

        if (strncmp(line, "Pytanie", 7) == 0) {
            qIndex++;
            strcpy(questions[qIndex].question, strchr(line, '|') + 1);
        } else {
            char *token = strtok(line, "|");
            int aIndex = token[0] - 'A';
            strcpy(questions[qIndex].answers[aIndex], strtok(NULL, "|"));
            questions[qIndex].points[aIndex][0] = atoi(strtok(NULL, "|"));
            questions[qIndex].points[aIndex][1] = atoi(strtok(NULL, "|"));
        }
    }

    *questionCount = qIndex + 1;
    fclose(file);
}

// Funkcja zapisywania wyników
void saveResults(const char *nickname, const char *quizType, int extrovertPercent, int introvertPercent) {
    FILE *file = fopen("wyniki.txt", "a");
    if (!file) {
        perror("Nie mozna otworzyc pliku wynikow");
        return;
    }
    fprintf(file, "Nick: %s | Quiz: %s | Ekstrawertyk: %d%% | Introwertyk: %d%%\n",
            nickname, quizType, extrovertPercent, introvertPercent);
    fclose(file);
}

// Funkcja wyświetlania wyników
void showResults() {
    FILE *file = fopen("wyniki.txt", "r");
    if (!file) {
        perror("Nie mozna otworzyc pliku wynikow");
        return;
    }

    char line[MAX_LINE];
    int results = 0;

    printf("\n=== Wyniki ===\n");

    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
        results = 1;
    }
    fclose(file);

    if (!results) {
        printf("Brak wynikow do wyswietlenia.\n");
    }
}

// Funkcja czyszczenia wyników
void clearResults() {
    FILE *file = fopen("wyniki.txt", "w");
    if (!file) {
        perror("Nie mozna otworzyc pliku wynikow");
        return;
    }
    fclose(file);
    printf("Wyniki zostaly wyczyszczone.\n");
}

// Funkcja wyświetlania paska postępu
void displayProgressBar(const char *label, int percentage) {

    const char *colorStart = "\033[35m";  // Fioletowy (magenta)
    const char *colorEnd = "\033[0m";

    printf("%s: %d%% [", label, percentage);
    int filled = (percentage * PROGRESS_BAR_LENGTH) / 100;

    for (int i = 0; i < PROGRESS_BAR_LENGTH; i++) {
        if (i < filled)
            printf("%s#%s", colorStart, colorEnd);
        else
            printf(" ");
    }
    printf("]\n");
}

void displayQuestionProgressBar(int current, int total) {
    int percentage = (current * 100) / total;
    printf("Pytanie %d/%d ", current, total);
    displayProgressBar("", percentage);
}

// Funkcja uruchamiania quizu tak/nie
void runYesNoQuiz(const char *nickname) {
    YesNoQuestion questions[MAX_QUESTIONS];
    int questionCount = 0;
    int choice;

    while (1) {
        printf("\nWybierz quiz tak/nie:\n");
        printf("1. Ekstrawertyk czy introwertyk?\n");
        printf("2. Czy jestes optymista?\n");
        printf("0. Wroc do menu glownego\n");
        printf("Twoj wybor: ");
        scanf("%d", &choice);

        if (choice == 0) {
            break;
        }

        const char *filename;
        switch (choice) {
            case 1:
                filename = "quiz_yesno1.txt";
                break;
            case 2:
                filename = "quiz_yesno2.txt";
                break;
            default:
                printf("Nieprawidlowy wybor, sprobuj ponownie.\n");
                continue;
        }

        loadYesNoQuestions(filename, questions, &questionCount);

        int totalPoints[2] = {0, 0}; // [0] - ekstrawertyk, [1] - introwertyk

        for (int i = 0; i < questionCount; i++) {
            displayQuestionProgressBar(i + 1, questionCount);

            char answer;
            while (1) {
                printf("%d. %s (T/N): lub 0 by wyjsc", i + 1, questions[i].question);
                scanf(" %c", &answer);

                if (answer == '0') {
                    printf("Wyjscie z quizu \n");
                    return;
                }

                answer = toupper(answer);
                if (answer == 'T' || answer == 'N') break;
                printf("Nieprawidlowa odpowiedz. Wpisz T lub N. lub 0 by wyjsc \n");
            }

            if (answer == 'T') {
                totalPoints[0] += questions[i].points[0]; // Ekstrawertyk
            } else {
                totalPoints[1] += questions[i].points[1]; // Introwertyk
            }
        }

        int total = totalPoints[0] + totalPoints[1];
        int extroPercent = (totalPoints[0] * 100) / total;
        int introPercent = (totalPoints[1] * 100) / total;

        printf("\nWynik quizu tak/nie:\n");
        displayProgressBar("Ekstrawertyk", extroPercent);
        displayProgressBar("Introwertyk", introPercent);

        saveResults(nickname, "Quiz Tak/Nie", extroPercent, introPercent);
    }
}

// Funkcja uruchamiania quizu wielokrotnego wyboru
void runMultiChoiceQuiz(const char *nickname) {
    MultiChoiceQuestion questions[MAX_QUESTIONS];
    int questionCount = 0;
    int choice;

    while (1) {
        printf("\nWybierz quiz wielokrotnego wyboru:\n");
        printf("1. Quiz 1\n");
        printf("2. Quiz 2\n");
        printf("3. Quiz 3\n");
        printf("0. Wroc do menu glownego\n");
        printf("Twoj wybor: ");
        scanf("%d", &choice);

        if (choice == 0) {
            break;
        }

        const char *filename;
        switch (choice) {
            case 1:
                filename = "quiz_multi1.txt";
                break;
            case 2:
                filename = "quiz_multi2.txt";
                break;
            case 3:
                filename = "quiz_multi3.txt";
                break;
            default:
                printf("Nieprawidlowy wybor, sprobuj ponownie.\n");
                continue;
        }

        loadMultiChoiceQuestions(filename, questions, &questionCount);

        int totalPoints[2] = {0, 0}; // [0] - optymista, [1] - pesymista

        for (int i = 0; i < questionCount; i++) {
            displayQuestionProgressBar(i + 1, questionCount);

            printf("\t %d. %s\n", i + 1, questions[i].question);
            for (int j = 0; j < 4; j++) {
                printf("\t \t %c. %s\n", 'A' + j, questions[i].answers[j]);
            }

            char answer;

            while (1) {
                printf("Twoja odpowiedz (A-D) lub 0 aby wyjsc: ");
                scanf(" %c", &answer);

                if (answer == '0') {
                    printf("Wyjscie z quizu \n");
                    return;
                }

                answer = toupper(answer);
                if (answer >= 'A' && answer <= 'D') break; // poprawna odpowiedź
                printf("Nieprawidlowa odpowiedz. Wpisz A, B, C lub D. lub 0, aby wyjsc. \n"); // Dodano komunikat
            }

            int aIndex = answer - 'A';
            totalPoints[0] += questions[i].points[aIndex][0];
            totalPoints[1] += questions[i].points[aIndex][1];
        }

        int total = totalPoints[0] + totalPoints[1];
        int optPercent = (totalPoints[0] * 100) / total;
        int pesPercent = (totalPoints[1] * 100) / total;

        printf("\nWynik quizu wielokrotnego wyboru:\n");
        displayProgressBar("Optymista", optPercent);
        displayProgressBar("Pesymista", pesPercent);

        saveResults(nickname, "Quiz Wielokrotnego Wyboru", optPercent, pesPercent);
    }
}

//eo
int main() {
    enableANSIColors(); // Aktywuj obsługę kolorów
    char nickname[MAX_LINE];
    int choice;

    printf("Podaj swoj nick: ");
    scanf("%s", nickname);

    while (1) {
        printf("\n=== Menu glowne ===\n");
        printf("1. Quiz tak/nie\n");
        printf("2. Quiz wielokrotnego wyboru\n");
        printf("3. Pokaz wyniki\n");
        printf("4. Wyczysc wyniki\n");
        printf("5. Wyjdz\n");
        printf("Twoj wybor: ");

        if (scanf("%d", &choice) != 1 || choice < 1 || choice > 5) {
            printf("Nieprawidlowy wybor, sprobuj ponownie.\n");
            while (getchar() != '\n'); // Czyszczenie bufora
            continue;
        }

        switch (choice) {
            case 1:
                runYesNoQuiz(nickname);
                break;
            case 2:
                runMultiChoiceQuiz(nickname);
                break;
            case 3:
                showResults();
                break;
            case 4:
                clearResults();
                break;
            case 5:
                printf("Zamykanie programu. Do widzenia, %s!\n", nickname);
                return 0;
            default:
                printf("Nieprawidlowy wybor, sprobuj ponownie.\n");
        }
    }
}
