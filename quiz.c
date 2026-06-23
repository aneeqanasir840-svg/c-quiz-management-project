#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#define TOTAL_QUESTIONS 21
#define QUIZ_TIME_LIMIT 300
#define NUM_SUBJECTS    5
#define NUM_DIFFICULTY  3
#define ARROW_PREFIX     224
#define LEFT_ARROW_CODE   75
#define RIGHT_ARROW_CODE  77

char allQuestions[NUM_SUBJECTS][NUM_DIFFICULTY][TOTAL_QUESTIONS][200];
char allAnswers[NUM_SUBJECTS][NUM_DIFFICULTY][TOTAL_QUESTIONS];
char diffNames[3][10] = {"EASY", "MEDIUM", "HARD"}; 
char subjectNames[NUM_SUBJECTS][20] = {"Computer", "Mathematics", "Physics", "English", "General Knowledge"};

int g_timerSeconds = QUIZ_TIME_LIMIT;
char g_currentUsername[30] = "";

void clearBuffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {       
    }
}

void printHeader()
{
    printf("======QUIZ MANAGEMENT SYSTEM======\n");
}

void printLine()
{
    printf("-------------------------------------\n");
}

char getNavigationKey()
{
    int key = getch();

    if (key == ARROW_PREFIX)
    {
        int secondKey = getch();
        if (secondKey == LEFT_ARROW_CODE)  return 'L';
        if (secondKey == RIGHT_ARROW_CODE) return 'R';
    }

    return toupper(key);
}

void readPassword(char password[], int size)
{
    int i = 0;
    char ch;

    while ((ch = getch()) != '\r' && i < size - 1)
    {
        password[i] = ch;
        i++;
        printf("*");
    }

    password[i] = '\0';
    printf("\n");
}
void saveQuestionsToFile()
{
    FILE *fp = fopen("questions.txt", "w");

    if(fp == NULL)
    {
        printf("file is not found");
        return;
    }

    int s,d,q;

    for(s=0; s<NUM_SUBJECTS; s++)
    {
        for(d=0; d<NUM_DIFFICULTY; d++)
        {
            for(q=0; q<TOTAL_QUESTIONS; q++)
            {
                fprintf(fp,"%d|%d|%d|%s|%c\n",s,d,q,allQuestions[s][d][q],allAnswers[s][d][q]);
            }
        }
    }

    fclose(fp);
}

void loadQuestionsFromFile()
{
    FILE *fp = fopen("questions.txt","r");

    if(fp == NULL)
    {
        printf("file is not found");
        return;
    }
    char line[300];

    while(fgets(line,sizeof(line),fp))
    {
        int s,d,q;
        char question[200];
        char answer;
        if(sscanf(line,"%d|%d|%d|%199[^|]|%c",&s,&d,&q,question,&answer)==5)
        {

            if(s>=0 && s<NUM_SUBJECTS &&d>=0 && d<NUM_DIFFICULTY && q>=0 && q<TOTAL_QUESTIONS)
            {

                strcpy(allQuestions[s][d][q],question);

                allAnswers[s][d][q]=answer;
            }
        }
    }
    fclose(fp);
}
void saveTimerToFile()
{
    FILE *fp = fopen("timer.txt", "w");
    if (fp == NULL)
    {
        printf("file is not found");
        return;
    }
    fprintf(fp, "%d\n", g_timerSeconds);
    fclose(fp);
}
void loadTimerFromFile()
{
    FILE *fp = fopen("timer.txt", "r");
    int seconds;
    if (fp == NULL) return;  

    fscanf(fp, "%d", &seconds);
    if (seconds > 0)
        g_timerSeconds = seconds;

    fclose(fp);
}
int secondsLeft(time_t startTime)
{
    int timepassed = (int)difftime(time(NULL), startTime);
    return g_timerSeconds - timepassed;
}
void printTimeLeft(time_t startTime)
{
    int left = secondsLeft(startTime);
    printf("  [Time Left: %02d:%02d]\n", left / 60, left % 60);
}
void viewQuestions()
{
    int s, d, q;
    for (s = 0; s < NUM_SUBJECTS; s++)
    {
        printf("\n===== %s QUESTIONS =====\n", subjectNames[s]);
        for (d = 0; d < NUM_DIFFICULTY; d++)
        {
            printf("\n  %s\n", diffNames[d]);
            for (q = 0; q < TOTAL_QUESTIONS; q++)
            {
                printf("  %s\n", allQuestions[s][d][q]);
            }
        }
    }
}

void searchQuestions()
{
    char search[100];
    int s, d, q, found = 0;

    printf("\nEnter keyword to search: ");
    fgets(search, sizeof(search), stdin);
    search[strcspn(search, "\n")] = '\0';

    printf("\n--- Search Results for %s ---\n", search);

    for (s = 0; s < NUM_SUBJECTS; s++)
    {
        for (d = 0; d < NUM_DIFFICULTY; d++)
        {
            for (q = 0; q < TOTAL_QUESTIONS; q++)
            {
            
                if (strstr(allQuestions[s][d][q], search) != NULL)
                {
                    printf("\n[%s | %s | Q%d]\n%s\n",
                           subjectNames[s], diffNames[d], q + 1, allQuestions[s][d][q]);
                    found++;
                }
            }
        }
    }

    if (found == 0)
        printf("  No questions found containing %s.\n", search);
    else
        printf("\n  Total found: %d\n", found);
}
void addQuestion()
{
    int subChoice, diffChoice, qNum;
    char newquestion[200], answerLine[10];
    char answer;

    printf("\n===== ADD / REPLACE QUESTION =====\n");
    printf("  Select Subject:\n  1.Computer  2.Math  3.Physics  4.English  5.GK\n  Choice: ");
    scanf("%d", &subChoice);
    clearBuffer();
    if (subChoice < 1 || subChoice > 5)
    {
        printf("  Invalid subject.\n");
        return;
    }

    printf("  Select Difficulty:\n  1.Easy  2.Medium  3.Hard\n  Choice: ");
    scanf("%d", &diffChoice);
    clearBuffer();
    if (diffChoice < 1 || diffChoice > 3)
    {
        printf("  Invalid difficulty.\n");
        return;
    }

    printf("  Enter Question Number to set (1-%d): ", TOTAL_QUESTIONS);
    scanf("%d", &qNum);
    clearBuffer();
    if (qNum < 1 || qNum > TOTAL_QUESTIONS)
    {
        printf("  Invalid question number.\n");
        return;
    }

    printf("  Enter the question text:\n  > ");
    fgets(newquestion, sizeof(newquestion), stdin);
    newquestion[strcspn(newquestion, "\n")] = '\0';

    printf("  Enter correct answer (A/B/C/D): ");
    fgets(answerLine, sizeof(answerLine), stdin);
    answer = toupper(answerLine[0]);
    if (answer < 'A' || answer > 'D')
    {
        printf("  Invalid answer.\n");
        return;
    }

    int s = subChoice - 1;
    int d = diffChoice - 1;
    int q = qNum - 1;

    strncpy(allQuestions[s][d][q], newquestion, 199);
    allAnswers[s][d][q] = answer;

    saveQuestionsToFile();

    printf("Question saved! Subject: %s | Q%d | Answer: %c\n",
           subjectNames[s], qNum, answer);
    printf("It will now appear in the quiz.\n");
}
void deleteQuestion()
{
    int subChoice, diffChoice, qNum;
    char confirmLine[10];

    printf("\n===== DELETE QUESTION =====\n");
    printf("Select Subject (1-5): ");
    scanf("%d", &subChoice);
    clearBuffer();
    if (subChoice < 1 || subChoice > 5)
    {
        printf("Invalid.\n");
        return;
    }

    printf("Select Difficulty (1=Easy 2=Medium 3=Hard): ");
    scanf("%d", &diffChoice);
    clearBuffer();
    if (diffChoice < 1 || diffChoice > 3)
    {
        printf("  Invalid.\n");
        return;
    }

    printf("Enter Question Number (1-%d): ", TOTAL_QUESTIONS);
    scanf("%d", &qNum);
    clearBuffer();
    if (qNum < 1 || qNum > TOTAL_QUESTIONS)
    {
        printf("Invalid question number.\n");
        return;
    }

    int s = subChoice - 1;
    int d = diffChoice - 1;
    int q = qNum - 1;

    printf("\nQuestion to delete:\n  %s\n  Answer: %c\n",
           allQuestions[s][d][q], allAnswers[s][d][q]);

    printf("\nConfirm delete? (Y/N): ");
    fgets(confirmLine, sizeof(confirmLine), stdin);

    if (toupper(confirmLine[0]) != 'Y')
    {
        printf("Delete cancelled.\n");
        return;
    }
    strcpy(allQuestions[s][d][q],"[DELETED]");
    allAnswers[s][d][q] = '?';

    saveQuestionsToFile();
    printf("Question %d deleted from %s.\n", qNum, subjectNames[s]);
}
void editQuestion()
{
    int subChoice, diffChoice, qNum;
    char newText[200], ansLine[10];

    printf("\n===== EDIT QUESTION =====\n");
    printf("  Select Subject (1-5): ");
    scanf("%d", &subChoice);
    clearBuffer();
    if (subChoice < 1 || subChoice > 5)
    {
        printf("Invalid.\n");
        return;
    }
    printf("Select Difficulty (1=Easy 2=Medium 3=Hard): ");
    scanf("%d", &diffChoice);
    clearBuffer();
    if (diffChoice < 1 || diffChoice > 3)
    {
        printf("Invalid.\n");
        return;
    }

    printf("Enter Question Number (1-%d): ", TOTAL_QUESTIONS);
    scanf("%d", &qNum);
    clearBuffer();
    if (qNum < 1 || qNum > TOTAL_QUESTIONS)
    {
        printf("Invalid question number.\n");
        return;
    }

    int s = subChoice - 1;
    int d = diffChoice - 1;
    int q = qNum - 1;

    printf("\nCurrent Question:\n  %s\n", allQuestions[s][d][q]);

    printf("\nEnter new question text (press ENTER to keep current):\n  > ");
    fgets(newText, sizeof(newText), stdin);
    newText[strcspn(newText, "\n")] = '\0';
    if (strlen(newText) > 0)
        strncpy(allQuestions[s][d][q], newText, 199);

    printf("Enter new answer (A/B/C/D, press ENTER to keep current): ");
    fgets(ansLine, sizeof(ansLine), stdin);
    ansLine[strcspn(ansLine, "\n")] = '\0';
    if (strlen(ansLine) > 0)
    {
        char newAnswer = toupper(ansLine[0]);
        if (newAnswer >= 'A' && newAnswer <= 'D')
            allAnswers[s][d][q] = newAnswer;
        else
            printf("Invalid answer key - answer not changed.\n");
    }

    saveQuestionsToFile();
    printf("Question updated and saved.\n");
}
void setTimerSetting()
{
    int mins;

    printf("\n===== SET QUIZ TIMER =====\n");
    printf("  Current limit: %d minutes\n", g_timerSeconds / 60);
    printf("  Enter new time limit in minutes (1-60): ");
    scanf("%d", &mins);
    clearBuffer();

    if (mins < 1 || mins > 60)
    {
        printf("Invalid. Keeping current.\n");
        return;
    }

    g_timerSeconds = mins * 60;
    saveTimerToFile();
    printf("Timer set to %d minutes.\n", mins);
}

void adminSection()
{
    char enteredUsername[30], enteredPassword[30];
    int adminChoice;

    printf("\n===== ADMIN LOGIN =====\n");
    printf("Username: ");
    fgets(enteredUsername, sizeof(enteredUsername), stdin);
    enteredUsername[strcspn(enteredUsername, "\n")] = '\0';

    printf("Password: ");
    readPassword(enteredPassword, sizeof(enteredPassword));

    if (strcmp(enteredUsername, "admin") != 0 || strcmp(enteredPassword, "1234") != 0)
    {
        printf("\nInvalid Username or Password!\n");
        return;
    }

    printf("\nLogin Successful!\n");

    do
    {
        printf("\n===== ADMIN PANEL =====\n");
        printf(" 1. Add / Replace Question\n");
        printf(" 2. Delete Question\n");
        printf(" 3. Search Questions\n");
        printf(" 4. Edit Question\n");
        printf(" 5. View All Questions\n");
        printf(" 6. Set Quiz Timer\n");
        printf(" 7. Back to Main Menu\n");
        printLine();
        printf("Enter Choice: ");
        scanf("%d", &adminChoice);
        clearBuffer();

        if (adminChoice == 1)      addQuestion();
        else if (adminChoice == 2) deleteQuestion();
        else if (adminChoice == 3) searchQuestions();
        else if (adminChoice == 4) editQuestion();
        else if (adminChoice == 5) viewQuestions();
        else if (adminChoice == 6) setTimerSetting();
        else if (adminChoice == 7) printf("\nReturning to Main Menu...\n");
        else                       printf("\nInvalid Choice!\n");

    } while (adminChoice != 7);
}

int mainMenu()
{
    int choice;
    printHeader();

    printf("1. Admin\n  2. Student\n  3. Exit\n");
    printf("Enter Choice: ");
    scanf("%d", &choice);
    clearBuffer();
    return choice;
}

int subjectMenu()
{
    int choice;
    printf("\n===== SELECT SUBJECT =====\n");
    printf("1. Computer\n  2. Mathematics\n  3. Physics\n  4. English\n  5. General Knowledge\n  6. Exit\n");
    printLine();
    printf("Enter Choice: ");
    scanf("%d", &choice);
    clearBuffer();
    return choice;
}

int difficultyMenu()
{
    int choice;
    printf("\n===== SELECT DIFFICULTY =====\n");
    printf("1. Easy\n  2. Medium\n  3. Hard\n  4. Back\n");
    printLine();
    printf("Enter Choice: ");
    scanf("%d", &choice);
    clearBuffer();
    return choice;
}
int findStudent(char username[], char outPassword[], char outFullName[])
{
    FILE *fp = fopen("students.txt", "r");
    if (fp == NULL) return 0;

    char line[150];
    char fileUser[50], filePass[50], fileName[50];

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        line[strcspn(line, "\n")] = '\0';

        if (sscanf(line, "%49[^|]|%49[^|]|%49[^\n]",
                   fileUser, filePass, fileName) == 3)
        {
            if (strcmp(fileUser, username) == 0)
            {
                strcpy(outPassword, filePass);
				strcpy(outFullName, fileName);
                fclose(fp);
                return 1;
            }
        }
    }

    fclose(fp);
    return 0;
}
void studentSignUp()
{
    char username[30], password[30], fullname[60];
    char pass[30], Name[60];

    printf("\n===== STUDENT SIGN UP =====\n");
    printf("Choose a username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';

    if (strlen(username) == 0)
    {
        printf("Username cannot be empty.\n");
        return;
    }
    if (findStudent(username, pass, Name))
    {
        printf("That username is already taken.\n");
        return;
    }
    printf("Choose a password: ");
    readPassword(password, sizeof(password));
    if (strlen(password) == 0)
    {
        printf("Password cannot be empty.\n");
        return;
    }
    printf("Enter your full name: ");
    fgets(fullname, sizeof(fullname), stdin);
    fullname[strcspn(fullname, "\n")] = '\0';
    if (strlen(fullname) == 0)
    {
        printf("Name cannot be empty.\n");
        return;
    }

    FILE *fp = fopen("students.txt", "a");
    if (fp == NULL)
    {
        printf("file is not found");
        return;
    }
    fprintf(fp,"%s|%s|%s\n",username, password, fullname);
    fclose(fp);

    printf("\nAccount created! Please login now.\n");
}
int studentLogin()
{
    char username[30], password[30];
    char storedPass[30], storedName[60];

    printf("\n===== STUDENT LOGIN =====\n");
    printf("Username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';

    printf("Password: ");
    readPassword(password, sizeof(password));

    if (!findStudent(username, storedPass, storedName))
    {
        printf("\nNo account found with that username. Please Sign Up first.\n");
        return 0;
    }

    if (strcmp(password, storedPass) != 0)
    {
        printf("\nIncorrect password!\n");
        return 0;
    }
    strcpy(g_currentUsername, username);
    printf("\nLogin Successful! Welcome, %s.\n", storedName);
    return 1;
}
void saveScore(char username[], char subject[], char difficulty[], int score, int timeTaken)
{
    FILE *fp = fopen("scores.txt", "a");
    if (fp == NULL)
    {
        printf("  Warning: could not save score.\n");
        return;
    }
    fprintf(fp, "%s|%s|%s|%d|%d|%d\n", username, subject, difficulty, score, TOTAL_QUESTIONS, timeTaken);
    fclose(fp);
}
void viewMyScores()
{
    FILE *fp = fopen("scores.txt", "r");
    if(fp == NULL) { printf("not found"); return; }

    char line[150], fileUser[30], subject[30], difficulty[20];
    int score, total, timeTaken, count = 0;

    printf("\n===== YOUR SCORE HISTORY (%s) =====\n", g_currentUsername);
    while(fgets(line, sizeof(line), fp) != NULL)
    {
        line[strcspn(line, "\n")] = '\0';
        if(sscanf(line, "%29[^|]|%29[^|]|%19[^|]|%d|%d|%d", fileUser, subject, difficulty, &score, &total, &timeTaken) == 6)
        {
            if(strcmp(fileUser, g_currentUsername) == 0)
            {
                count++;
                printf("  [%d] %-18s (%-6s)  Score: %d/%d   Time: %02d:%02d\n",
                       count, subject, difficulty, score, total, timeTaken/60, timeTaken%60);
            }
        }
    }
    fclose(fp);
    if(count == 0) printf("No attempts yet.\n");
    printLine();
}
void showFeedback(int score, int timeTaken)
{
    printf("\n=====================================\n");
    printf("              RESULT\n");
    printf("=====================================\n");
    printf("Your Score  : %d / %d\n", score, TOTAL_QUESTIONS);
    printf("Time Taken  : %02d:%02d\n", timeTaken / 60, timeTaken % 60);
    printf("Time Limit  : %02d:%02d\n", g_timerSeconds / 60, g_timerSeconds % 60);
    printLine();
    if (score == TOTAL_QUESTIONS)      printf("PERFECT! Outstanding!\n");
    else if (score >= 16)              printf("excellent! Very well done!\n");
    else if (score >= 12)              printf("Good Job! Keep it up!\n");
    else if (score >= 8)               printf("Fair! A bit more practice needed.\n");
    else if (score >= 4)               printf("Keep Practicing!\n");
    else                                printf("Don't give up! Try again!\n");
}
void runQuiz(int subject, int difficulty)
{
    char studentAnswers[TOTAL_QUESTIONS];
    int  answered[TOTAL_QUESTIONS];
    int  i, score;
    char confirmLine[10];
    int subindex = subject - 1;  
    for (i = 0; i < TOTAL_QUESTIONS; i++)
        answered[i] = 0;

    time_t startTime = time(NULL);

    printf("\n===== %s QUIZ =====\n", subjectNames[subindex]);
    printf("Time Limit : %d minutes\n", g_timerSeconds / 60);
    printf("[A/B/C/D = Answer | LEFT ARROW = Back | RIGHT ARROW = Forth | Q = Quit & Submit]\n");
    printLine();
    i = 0;
    while (1)
    {
        int left = secondsLeft(startTime);
        if (left <= 0)
        {
            printf("\n*** TIME'S UP! Quiz auto-submitted.***\n");
            break;
        }
        printf("\nQuestion %d of %d:\n", i + 1, TOTAL_QUESTIONS);
        printTimeLeft(startTime);
        printf("%s\n", allQuestions[subindex][difficulty][i]);

        if (answered[i])
            printf("(Saved answer: %c)\n", studentAnswers[i]);

        printf("Your input: ");
        char key = getNavigationKey();
        printf("%c\n", key);  

        if (secondsLeft(startTime) <= 0)
        {
            printf("\n*** TIME'S UP! Quiz auto-submitted.***\n");
            break;
        }

        if (key == 'R')  
        {
            if (i < TOTAL_QUESTIONS - 1)
                i++;
            else
                printf("This is the last question!\n");
        }
        else if (key == 'L')  
        {
            if (i > 0)
                i--;
            else
                printf("This is the first question!\n");
        }
        else if (key == 'Q')
        {
            printf("Quit and submit? (Y/N): ");
            fgets(confirmLine, sizeof(confirmLine), stdin);
            if (toupper(confirmLine[0]) == 'Y') break;
        }
        else if (key == 'A' || key == 'B' || key == 'C' || key == 'D')
        {
            studentAnswers[i] = key;
            answered[i] = 1;
            printf("Answer saved: %c\n", key);

            if (i < TOTAL_QUESTIONS - 1)
            {
                i++;
            }
            else
            {
                int allDone = 1, j;
                for (j = 0; j < TOTAL_QUESTIONS; j++)
                {
                    if (!answered[j])
                    {
                        allDone = 0;
                        break;
                    }
                }

                if (allDone)
                {
                    printf("\n  All questions answered! Press RIGHT ARROW to Submit, LEFT ARROW to go back: ");
                    char finalKey = getNavigationKey();
                    printf("\n");
                    if (finalKey == 'R') break;
                    else if (finalKey == 'L' && i > 0) i--;
                }
            }
        }
        else
        {
            printf("Invalid! Use A/B/C/D, LEFT ARROW, RIGHT ARROW, or Q.\n");
        }
    }

    int timeTaken = (int)difftime(time(NULL), startTime);
    if (timeTaken > g_timerSeconds) timeTaken = g_timerSeconds;

    score = 0;
    for (i = 0; i < TOTAL_QUESTIONS; i++)
    {
        if (answered[i] && studentAnswers[i] == allAnswers[subindex][difficulty][i])
            score++;
    }

    printf("\n===== ANSWER REVIEW =====\n");
    for (i = 0; i < TOTAL_QUESTIONS; i++)
    {
        char given   = answered[i] ? studentAnswers[i] : '-';
        char correct = allAnswers[subindex][difficulty][i];
        printf("  Q%-2d  Your: %c   Correct: %c   [%s]\n",
               i + 1, given, correct, (given == correct) ? "Correct" : "Incorrect");
    }

    showFeedback(score, timeTaken);
    saveScore(g_currentUsername, subjectNames[subindex], diffNames[difficulty], score, timeTaken);
}
void studentSection()
{
    int loggedIn = 0;
    while (!loggedIn)
    {
        int choice;
        printf("\n===== STUDENT SECTION =====\n");
        printf("  1. Login\n  2. Sign Up\n  3. Back\n");
        printLine();
        printf("Enter Choice: ");
        scanf("%d", &choice);
        clearBuffer();

        if (choice == 1)      
		loggedIn = studentLogin();
        else if (choice == 2) 
		studentSignUp();
        else if (choice == 3) 
		return;
        else                  
		printf("\nInvalid Choice!\n");
    }
    int subjectChoice, difficultyChoice, afterChoice;

    do
    {
        subjectChoice = subjectMenu();
        if (subjectChoice == 6)
        {
            printf("\nReturning to Main Menu...\n");
            return;
        }
        if (subjectChoice < 1 || subjectChoice > 5)
        {
            printf("\nInvalid Choice!\n");
            continue;
        }

        difficultyChoice = difficultyMenu();
        if (difficultyChoice == 4)
        {
            printf("\nGoing back...\n");
            continue;
        }
        if (difficultyChoice < 1 || difficultyChoice > 3)
        {
            printf("\nInvalid Choice!\n");
            continue;
        }

        runQuiz(subjectChoice, difficultyChoice - 1);

        printf("\n===== WHAT NEXT? =====\n");
        printf("1. Choose Another Subject\n");
        printf("2. View My Score History\n");
        printf("3. Exit to Main Menu\n");
        printLine();
        printf("Enter Choice: ");
        scanf("%d", &afterChoice);
        clearBuffer();

        if (afterChoice == 2)      viewMyScores();
        else if (afterChoice == 3)
        {
            printf("\nReturning to Main Menu...\n");
            return;
        }

    } while (1);
}
int main()
{
    int choice;

    loadQuestionsFromFile();
    loadTimerFromFile();
    do
    {
        choice = mainMenu();
        if (choice == 1)      adminSection();
        else if (choice == 2) studentSection();
        else if (choice == 3) printf("\nGoodbye!\n");
        else                  printf("\nInvalid Choice!\n");
    } while (choice != 3);

    return 0;
}
