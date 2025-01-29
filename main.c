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
    int points[2];
} YesNoQuestion;

typedef struct {
    char question[MAX_LINE];
    char answers[4][MAX_LINE];
    int points[4][2];
} MultiChoiceQuestion;

// Prototypy funkcji
void runYesNoQuiz(const char *nickname);
void runMultiChoiceQuiz(const char *nickname);
void showResults();
void clearResults();
void displayProgressBar(const char *label, int percentage);
void displayQuestionProgressBar(int current, int total);
void saveResults(const char *nickname, const char *quizType, const char *category1, const char *category2, int percent1, int percent2);


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
        line[strcspn(line, "\n")] = 0;

        char *token = strtok(line, "|");
        strcpy(questions[qIndex].question, token);
        questions[qIndex].points[0] = atoi(strtok(NULL, "|"));
        questions[qIndex].points[1] = atoi(strtok(NULL, "|"));

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
void saveResults(const char *nickname, const char *quizType, const char *category1, const char *category2, int percent1, int percent2) {
    FILE *file = fopen("wyniki.txt", "a");
    if (!file) {
        perror("Nie mozna otworzyc pliku wynikow");
        return;
    }
    fprintf(file, "Nick: %s | Quiz: %s | %s: %d%% | %s: %d%%\n",
            nickname, quizType, category1, percent1, category2, percent2);
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

    printf("\n\033[36m=== Wyniki ===\033[0m\n");

    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
        results = 1;
    }
    fclose(file);

    if (!results) {
        printf("\033[31mBrak wynikow do wyswietlenia.\033[0m\n");
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
    printf("\033[33mWyniki zostaly wyczyszczone.\033[0m\n");
}

// Funkcja wyświetlania paska postępu
void displayProgressBar(const char *label, int percentage) {

    const char *colorStart = "\033[35m";
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

void runYesNoQuiz(const char *nickname) {
    YesNoQuestion questions[MAX_QUESTIONS];
    int questionCount = 0;
    int choice;

    while (1) {
        printf("\n\033[32mWybierz quiz tak/nie:\033[0m\n");
        printf("1. Ekstrawertyk czy introwertyk?\n");
        printf("2. Czy jestes optymista?\n");
        printf("0. Wroc do menu glownego\n");
        printf("Twoj wybor: ");

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("\033[31mNieprawidlowy wybor, sprobuj ponownie.\033[0m\n");
            continue;
        }

        if (choice < 0 || choice > 2) {
            printf("\033[31mNieprawidlowy wybor, sprobuj ponownie.\033[0m\n");
            continue;
        }

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
                printf("\033[31mNieprawidlowy wybor, sprobuj ponownie.\033[0m\n");
                continue;
        }

        loadYesNoQuestions(filename, questions, &questionCount);

        int pointsA = 0;  // Punkty dla pierwszej kategorii
        int pointsB = 0;  // Punkty dla drugiej kategorii

        for (int i = 0; i < questionCount; i++) {
            displayQuestionProgressBar(i + 1, questionCount);

            char answer;
            while (1) {
                printf("%d. %s (T/N): lub 0 by wyjsc: ", i + 1, questions[i].question);
                if (scanf(" %c", &answer) != 1) {
                    printf("\033[31mNieprawidlowa odpowiedz. Wpisz T, N lub 0.\033[0m\n");
                    while (getchar() != '\n');
                    continue;
                }

                answer = toupper(answer);

                while ( getchar() != '\n' );

                if (answer == '0') {
                    printf("\033[94mWyjscie z quizu.\033[0m\n");
                    return;
                }

                if (answer == 'T' || answer == 'N') break;
                printf("\033[31mNieprawidlowa odpowiedz. Wpisz T, N lub 0.\033[0m\n");
            }

            // Dodanie punktow na podstawie odpowiedzi
            if (answer == 'T') {
                pointsA += questions[i].points[0];  // T -> pierwsza kategoria
                pointsB += questions[i].points[1];  // T -> druga kategoria
            } else if (answer == 'N') {
                pointsA += questions[i].points[1];  // N -> pierwsza kategoria
                pointsB += questions[i].points[0];  // N -> druga kategoria
            }
        }

        // Obliczanie procentów
        int percentageA = (pointsA * 100 + pointsA + pointsB / 2) / (pointsA + pointsB);
        int percentageB = 100 - percentageA;


        // Wyswietlanie wynikow w zaleznosci od wybranego quizu
        if (choice == 1) {
            printf("\033[92m\nWynik quizu tak/nie:\033[0m\n");
            displayProgressBar("Ekstrawertyk", (int)percentageA);
            displayProgressBar("Introwertyk", (int)percentageB);
        } else if (choice == 2) {
            printf("\033[92m\nWynik quizu tak/nie:\033[0m\n");
            displayProgressBar("Optymista", (int)percentageA);
            displayProgressBar("Pesymista", (int)percentageB);
        }

        const char *category1, *category2;
        if (choice == 1) {
            category1 = "Ekstrawertyk";
            category2 = "Introwertyk";
        } else {
            category1 = "Optymista";
            category2 = "Pesymista";
        }
        // Zapis wynikow
        saveResults(nickname, (choice == 1) ? "Ekstrawertyk czy introwertyk?" : "Czy jestes optymista?",
            (choice == 1) ? "Ekstrawertyk" : "Optymista",
            (choice == 1) ? "Introwertyk" : "Pesymista",
            (int)percentageA, (int)percentageB);

    }
}



void runMultiChoiceQuiz(const char *nickname) {
    MultiChoiceQuestion questions[MAX_QUESTIONS];
    int questionCount = 0;
    int choice;

    while (1) {
        printf("\033[32m\nWybierz quiz wielokrotnego wyboru:\033[0m\n");
        printf("1. Jak reagujesz na stresujace sytuacje?\n");
        printf("2. Kiedy masz do wyboru rozne opcje, czy decydujesz się na najbezpieczniejsza czy najbardziej ryzykowna?\n");
        printf("3. Jakim jestes rodzajem jedzenia?\n");
        printf("0. Wroc do menu glownego\n");
        printf("Twoj wybor: ");

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("\033[31mNieprawidlowy wybor, sprobuj ponownie.\033[0m\n");
            continue;
        }

        if (choice < 0 || choice > 3) {
            printf("\033[31mNieprawidlowy wybor, sprobuj ponownie.\033[0m\n");
            continue;
        }

        if (choice == 0) {
            printf("Przejscie do menu");
            return;
        }

        const char *filename;
        const char *quizType;
        const char *category1, *category2;

        switch (choice) {
            case 1:
                filename = "quiz_multi1.txt";
            quizType = "Jak reagujesz na stres?";
            category1 = "Pozytywnie";
            category2 = "Negatywnie";
            break;
            case 2:
                filename = "quiz_multi2.txt";
            quizType = "Twoje decyzje - bezpieczne czy ryzykowne?";
            category1 = "Bezpiecznie";
            category2 = "Ryzykownie";
            break;
            case 3:
                filename = "quiz_multi3.txt";
            quizType = "Jakim jestes rodzajem jedzenia?";
            category1 = "Hamburger";
            category2 = "Salatka";
            break;
            default:
                printf("\033[31mNieprawidlowy wybor, sprobuj ponownie.\033[0m\n");
            continue;
        }

        loadMultiChoiceQuestions(filename, questions, &questionCount);

        int totalPoints[2] = {0, 0};
        int maxPoints[2] = {0, 0};

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
                if (answer >= 'A' && answer <= 'D') break;
                printf("\033[31mNieprawidlowa odpowiedz. Wpisz A, B, C lub D. lub 0, aby wyjsc.\033[0m\n");

            }

            int aIndex = answer - 'A';
            totalPoints[0] += questions[i].points[aIndex][0];
            totalPoints[1] += questions[i].points[aIndex][1];

            maxPoints[0] += questions[i].points[aIndex][0];
            maxPoints[1] += questions[i].points[aIndex][1];
        }

        int totalMaxPoints = maxPoints[0] + maxPoints[1];
        int percent1 = (totalMaxPoints > 0) ? (totalPoints[0] * 100 + totalMaxPoints / 2) / totalMaxPoints : 0;
        int percent2 = 100 - percent1;

        printf("\033[32m\nWynik quizu wielokrotnego wyboru:\033[0m\n");

        displayProgressBar(category1, percent1);
        displayProgressBar(category2, percent2);


        saveResults(nickname, quizType, category1, category2, percent1, percent2);

        break;
    }
}

int main() {
    enableANSIColors();
    char nickname[MAX_LINE];
    int choice;

    printf("\033[32mPodaj swoj nick: \033[0m");
    scanf("%s", nickname);

    while (1) {
        printf("\033[35m\n=== Menu glowne ===\033[0m\n");
        printf("1. Quiz tak/nie\n");
        printf("2. Quiz wielokrotnego wyboru\n");
        printf("3. Pokaz wyniki\n");
        printf("4. Wyczysc wyniki\n");
        printf("5. Wyjdz\n");
        printf("\033[95mTwoj wybor: \033[0m");


        if (scanf("%d", &choice) != 1 || choice < 1 || choice > 5) {
            printf("\033[31mNieprawidlowy wybor, sprobuj ponownie.\033[0m\n");
            while (getchar() != '\n');
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
                printf("\033[31mNieprawidlowy wybor, sprobuj ponownie.\033[0m\n");
        }
    }
 // Agnieszka Mazur
}
