#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
// CONSTANTS
#define TOTAL_QUESTIONS 20
#define QUIZ_TIME_LIMIT 300
#define NUM_SUBJECTS    5
#define NUM_DIFFICULTY  3

// QUIZ DATA
char allQuestions[NUM_SUBJECTS][NUM_DIFFICULTY][TOTAL_QUESTIONS][200];
char allAnswers[NUM_SUBJECTS][NUM_DIFFICULTY][TOTAL_QUESTIONS];
char subjectNames[5][20] = {"Computer", "Mathematics", "Physics", "English", "General Knowledge"};
// Global settings — declared early so every function below can see them
int g_timerSeconds = QUIZ_TIME_LIMIT;   // current quiz time limit, in seconds
char g_currentUsername[30] = "";        // username of the logged-in student
// HELPER: clear input buffer
void clearBuffer()
{
    int c;
    while((c = getchar()) != '\n' && c != EOF);
}
// SAFE INPUT: read a line, strip newline
void readLine(char *buf, int size)
{
    if(fgets(buf, size, stdin) != NULL)
        buf[strcspn(buf, "\n")] = '\0';
    else
        buf[0] = '\0';
}
// UTILITY
void printHeader()
{
    printf("       QUIZ MANAGEMENT SYSTEM\n"); 
}
void printLine()
{
    printf("-------------------------------------\n");
}
int mainMenu()
{
    int choice;
    printHeader();
    printf("  1. Admin\n  2. Student\n  3. Exit\n");
    printLine();
    printf("Enter Choice: ");
    scanf("%d", &choice);
    clearBuffer();
    return choice;
}
// POINTER HELPERS
char* getQuestionPtr(int subject, int diff, int idx)
{
    return allQuestions[subject][diff][idx];
}
char* getAnswerPtr(int subject, int diff, int idx)
{
    return &allAnswers[subject][diff][idx];
}
void saveQuestionsToFile()
{
    FILE *fp = fopen("questions.txt", "w");
    if(fp == NULL) { printf("  Warning: could not save questions.\n"); return; }
    int s, d, q, i;
    char lineText[200];
    for(s = 0; s < NUM_SUBJECTS; s++)
        for(d = 0; d < NUM_DIFFICULTY; d++)
            for(q = 0; q < TOTAL_QUESTIONS; q++)
            {
                char *qText  = getQuestionPtr(s, d, q);
                char  answer = *getAnswerPtr(s, d, q);
                // newlines inside the question text are stored as '~'
                // so each question still fits on ONE line in the file
                for(i = 0; qText[i] != '\0'; i++)
                    lineText[i] = (qText[i] == '\n') ? '~' : qText[i];
                lineText[i] = '\0';
                fprintf(fp, "%d|%d|%d|%s|%c\n", s, d, q, lineText, answer);
            }
    fclose(fp);
}
// Loads every question + answer from questions.txt into the arrays above
void loadQuestionsFromFile()
{
    FILE *fp = fopen("questions.txt", "r");
    if(fp == NULL)
    {
        printf("\n  WARNING: questions.txt not found!\n");
        printf("  Please make sure questions.txt is in the same folder as this program.\n");
        return;
    }
    char line[300];
    while(fgets(line, sizeof(line), fp) != NULL)
    {
        line[strcspn(line, "\n")] = '\0';
        int s, d, q, i;
        char rawText[200], answer;
        if(sscanf(line, "%d|%d|%d|%199[^|]|%c", &s, &d, &q, rawText, &answer) == 5)
        {
            if(s >= 0 && s < NUM_SUBJECTS && d >= 0 && d < NUM_DIFFICULTY && q >= 0 && q < TOTAL_QUESTIONS)
            {
                // turn the stored '~' back into a real newline
                for(i = 0; rawText[i] != '\0'; i++)
                    if(rawText[i] == '~') rawText[i] = '\n';
                strncpy(allQuestions[s][d][q], rawText, 199);
                allAnswers[s][d][q] = answer;
            }
        }
    }
    fclose(fp);
}
// TIMER HELPERS
int secondsLeft(time_t startTime)
{
    return g_timerSeconds - (int)difftime(time(NULL), startTime);
}
void printTimeLeft(time_t startTime)
{
    int left = secondsLeft(startTime);
    if(left < 0) left = 0;
    printf("  [Time Left: %02d:%02d]\n", left / 60, left % 60);
}
// VIEW ALL QUESTIONS (ADMIN) — pages through every question with N/P navigation
void viewQuestions()
{
    char *diffNames[] = {"EASY", "MEDIUM", "HARD"};
    char buf[10];
    // Flatten (subject, difficulty, question) into one big list of
    // NUM_SUBJECTS * NUM_DIFFICULTY * TOTAL_QUESTIONS slots, indexed by pos.
    int totalSlots = NUM_SUBJECTS * NUM_DIFFICULTY * TOTAL_QUESTIONS;
    int pos = 0;
    printf("\n  How to navigate: N = Next | P = Previous | B = Back to Admin Menu\n");
    while(1)
    {
        int s = pos / (NUM_DIFFICULTY * TOTAL_QUESTIONS);
        int d = (pos / TOTAL_QUESTIONS) % NUM_DIFFICULTY;
        int q = pos % TOTAL_QUESTIONS;
        printf("\n  ----------------------------------------\n");
        printf("  %s | %s | Question %d of %d   (Item %d of %d)\n",
               subjectNames[s], diffNames[d], q + 1, TOTAL_QUESTIONS, pos + 1, totalSlots);
        printf("  %s\n", getQuestionPtr(s, d, q));
        printf("  Answer: %c\n", *getAnswerPtr(s, d, q));
        printf("  ----------------------------------------\n");
        printf("  [N = Next | P = Previous | B = Back to Admin Menu]\n");
        printf("  Choice: ");
        readLine(buf, sizeof(buf));
        char nav = toupper(buf[0]);
        if(nav == 'N')
        {
            if(pos < totalSlots - 1) pos++;
            else printf("  Already at the last question!\n");
        }
        else if(nav == 'P')
        {
            if(pos > 0) pos--;
            else printf("  Already at the first question!\n");
        }
        else if(nav == 'B')
        {
            printf("  Returning to Admin Menu...\n");
            return;
        }
        else
        {
            printf("  Invalid! Use N, P, or B.\n");
        }
    }
}
// SEARCH QUESTIONS (ADMIN) — pages through matches with N/P navigation
void searchQuestions()
{
    char keyword[100];
    int s, d, q, found = 0;
    char *diffNames[] = {"EASY", "MEDIUM", "HARD"};
    printf("\nEnter keyword to search: ");
    readLine(keyword, sizeof(keyword));
    char kwLower[100];
    int k;
    for(k = 0; keyword[k]; k++)
        kwLower[k] = tolower((unsigned char)keyword[k]);
    kwLower[k] = '\0';
    // Collect every matching slot's (s, d, q) so we can page through them.
    int maxMatches = NUM_SUBJECTS * NUM_DIFFICULTY * TOTAL_QUESTIONS;
    int matchS[maxMatches], matchD[maxMatches], matchQ[maxMatches];
    for(s = 0; s < NUM_SUBJECTS; s++)
        for(d = 0; d < NUM_DIFFICULTY; d++)
            for(q = 0; q < TOTAL_QUESTIONS; q++)
            {
                char *qText = getQuestionPtr(s, d, q);
                char qLower[200];
                int j;
                for(j = 0; qText[j]; j++)
                    qLower[j] = tolower((unsigned char)qText[j]);
                qLower[j] = '\0';
                if(strstr(qLower, kwLower) != NULL)
                {
                    matchS[found] = s;
                    matchD[found] = d;
                    matchQ[found] = q;
                    found++;
                }
            }
    if(!found)
    {
        printf("  No questions found containing \"%s\".\n", keyword);
        return;
    }
    printf("\n  Found %d result(s) for \"%s\".\n", found, keyword);
    printf("  How to navigate: N = Next | P = Previous | B = Back to Admin Menu\n");
    char buf[10];
    int pos = 0;
    while(1)
    {
        int s2 = matchS[pos], d2 = matchD[pos], q2 = matchQ[pos];
        printf("\n  ----------------------------------------\n");
        printf("  Result %d of %d\n", pos + 1, found);
        printf("  [%s | %s | Q%d]\n", subjectNames[s2], diffNames[d2], q2 + 1);
        printf("  %s\n", getQuestionPtr(s2, d2, q2));
        printf("  Answer: %c\n", *getAnswerPtr(s2, d2, q2));
        printf("  ----------------------------------------\n");
        printf("  [N = Next | P = Previous | B = Back to Admin Menu]\n");
        printf("  Choice: ");
        readLine(buf, sizeof(buf));
        char nav = toupper(buf[0]);
        if(nav == 'N')
        {
            if(pos < found - 1) pos++;
            else printf("  Already at the last result!\n");
        }
        else if(nav == 'P')
        {
            if(pos > 0) pos--;
            else printf("  Already at the first result!\n");
        }
        else if(nav == 'B')
        {
            printf("  Returning to Admin Menu...\n");
            return;
        }
        else
        {
            printf("  Invalid! Use N, P, or B.\n");
        }
    }
}
// ADD QUESTION (ADMIN) — supports N/P navigation across question numbers
void addQuestion()
{
    int subChoice, diffChoice, qNum;
    char qText[200], answer;
    char *diffNames[] = {"Easy", "Medium", "Hard"};
    char buf[10];
    printf("\n===== ADD / REPLACE QUESTION =====\n");
    printf("  Select Subject:\n  1.Computer  2.Math  3.Physics  4.English  5.GK\n  Choice: ");
    scanf("%d", &subChoice);
    clearBuffer();
    if(subChoice < 1 || subChoice > 5) { printf("  Invalid subject.\n"); return; }
    printf("  Select Difficulty:\n  1.Easy  2.Medium  3.Hard\n  Choice: ");
    scanf("%d", &diffChoice);
    clearBuffer();
    if(diffChoice < 1 || diffChoice > 3) { printf("  Invalid difficulty.\n"); return; }
    int s = subChoice - 1;
    int d = diffChoice - 1;
    qNum = 1;   // start at question 1, admin can navigate from here
    printf("\n  How to navigate: N = Next | P = Previous | B = Back to Admin Menu\n");
    while(1)
    {
        int q = qNum - 1;
        printf("\n  ----------------------------------------\n");
        printf("  %s | %s | Question %d of %d\n", subjectNames[s], diffNames[d], qNum, TOTAL_QUESTIONS);
        printf("  Current text  : %s\n", getQuestionPtr(s, d, q));
        printf("  Current answer: %c\n", *getAnswerPtr(s, d, q));
        printf("  ----------------------------------------\n");
        printf("  [E = Edit this question | N = Next | P = Previous | B = Back to Admin Menu]\n");
        printf("  Choice: ");
        readLine(buf, sizeof(buf));
        char nav = toupper(buf[0]);
        if(nav == 'N')
        {
            if(qNum < TOTAL_QUESTIONS) qNum++;
            else printf("  Already at the last question!\n");
        }
        else if(nav == 'P')
        {
            if(qNum > 1) qNum--;
            else printf("  Already at the first question!\n");
        }
        else if(nav == 'B')
        {
            printf("  Returning to Admin Menu...\n");
            return;
        }
        else if(nav == 'E')
        {
            printf("  Enter the question text:\n  > ");
            readLine(qText, sizeof(qText));
            printf("  Enter correct answer (A/B/C/D): ");
            readLine(buf, sizeof(buf));
            answer = toupper(buf[0]);
            if(answer < 'A' || answer > 'D')
            {
                printf("  Invalid answer. Question not saved.\n");
                continue;
            }
            strncpy(allQuestions[s][d][q], qText, 199);
            allAnswers[s][d][q] = answer;
            saveQuestionsToFile();   // write the change to questions.txt right away
            printf("  Question saved! Subject: %s | Difficulty: %s | Q%d | Answer: %c\n",
                   subjectNames[s], diffNames[d], qNum, answer);
            printf("  It will now appear in the quiz.\n");
        }
        else
        {
            printf("  Invalid! Use E, N, P, or B.\n");
        }
    }
}
// DELETE QUESTION (ADMIN) — supports N/P navigation across question numbers
void deleteQuestion()
{
    int subChoice, diffChoice, qNum;
    char *diffNames[] = {"Easy", "Medium", "Hard"};
    char buf[10];
    printf("\n===== DELETE QUESTION =====\n");
    printf("  Select Subject (1-5): ");
    scanf("%d", &subChoice);
    clearBuffer();
    if(subChoice < 1 || subChoice > 5) { printf("  Invalid.\n"); return; }
    printf("  Select Difficulty (1=Easy 2=Medium 3=Hard): ");
    scanf("%d", &diffChoice);
    clearBuffer();
    if(diffChoice < 1 || diffChoice > 3) { printf("  Invalid.\n"); return; }
    int s = subChoice - 1;
    int d = diffChoice - 1;
    qNum = 1;
    printf("\n  How to navigate: N = Next | P = Previous | B = Back to Admin Menu\n");
    while(1)
    {
        int q = qNum - 1;
        char *qPtr = getQuestionPtr(s, d, q);
        char  ans  = *getAnswerPtr(s, d, q);
        printf("\n  ----------------------------------------\n");
        printf("  %s | %s | Question %d of %d\n", subjectNames[s], diffNames[d], qNum, TOTAL_QUESTIONS);
        printf("  %s\n", qPtr);
        printf("  Answer: %c\n", ans);
        printf("  ----------------------------------------\n");
        printf("  [D = Delete this question | N = Next | P = Previous | B = Back to Admin Menu]\n");
        printf("  Choice: ");
        readLine(buf, sizeof(buf));
        char nav = toupper(buf[0]);
        if(nav == 'N')
        {
            if(qNum < TOTAL_QUESTIONS) qNum++;
            else printf("  Already at the last question!\n");
        }
        else if(nav == 'P')
        {
            if(qNum > 1) qNum--;
            else printf("  Already at the first question!\n");
        }
        else if(nav == 'B')
        {
            printf("  Returning to Admin Menu...\n");
            return;
        }
        else if(nav == 'D')
        {
            printf("  Confirm delete? (Y/N): ");
            readLine(buf, sizeof(buf));
            if(toupper(buf[0]) != 'Y')
            {
                printf("  Delete cancelled.\n");
                continue;
            }
            strcpy(qPtr, "[DELETED]");
            *getAnswerPtr(s, d, q) = '?';
            saveQuestionsToFile();
            printf("  Question %d deleted from %s [%s].\n", qNum, subjectNames[s], diffNames[d]);
        }
        else
        {
            printf("  Invalid! Use D, N, P, or B.\n");
        }
    }
}
// EDIT QUESTION (ADMIN) — supports N/P navigation across question numbers
void editQuestion()
{
    int subChoice, diffChoice, qNum;
    char newText[200], navBuf[10];
    char *diffNames[] = {"Easy", "Medium", "Hard"};
    printf("\n===== EDIT QUESTION =====\n");
    printf("  Select Subject (1-5): "); 
    scanf("%d", &subChoice);
    clearBuffer();
    if(subChoice < 1 || subChoice > 5) { printf("  Invalid.\n"); return; }
    printf("  Select Difficulty (1=Easy 2=Medium 3=Hard): ");
    scanf("%d", &diffChoice);
    clearBuffer();
    if(diffChoice < 1 || diffChoice > 3) { printf("  Invalid.\n"); return; }
    int s = subChoice - 1;
    int d = diffChoice - 1;
    qNum = 1;
    printf("\n  How to navigate: N = Next | P = Previous | B = Back to Admin Menu\n");
    while(1)
    {
        int q = qNum - 1;
        char *qPtr = getQuestionPtr(s, d, q);
        char *aPtr = getAnswerPtr(s, d, q);
        printf("\n  ----------------------------------------\n");
        printf("  %s | %s | Question %d of %d\n", subjectNames[s], diffNames[d], qNum, TOTAL_QUESTIONS);
        printf("  Current Question: %s\n", qPtr);
        printf("  Current Answer  : %c\n", *aPtr);
        printf("  ----------------------------------------\n");
        printf("  [E = Edit this question | N = Next | P = Previous | B = Back to Admin Menu]\n");
        printf("  Choice: ");
        readLine(navBuf, sizeof(navBuf));
        char nav = toupper(navBuf[0]);
        if(nav == 'N')
        {
            if(qNum < TOTAL_QUESTIONS) qNum++;
            else printf("  Already at the last question!\n");
        }
        else if(nav == 'P')
        {
            if(qNum > 1) qNum--;
            else printf("  Already at the first question!\n");
        }
        else if(nav == 'B')
        {
            printf("  Returning to Admin Menu...\n");
            return;
        }
        else if(nav == 'E')
        {
            printf("\n  Enter new question text (press ENTER to keep current):\n  > ");
            readLine(newText, sizeof(newText));
            if(strlen(newText) > 0)
                strncpy(qPtr, newText, 199);
            printf("  Enter new answer (A/B/C/D, press ENTER to keep current): ");
            readLine(navBuf, sizeof(navBuf));
            if(strlen(navBuf) > 0)
            {
                char newAnswer = toupper(navBuf[0]);
                if(newAnswer >= 'A' && newAnswer <= 'D') *aPtr = newAnswer;
                else printf("  Invalid answer key — answer not changed.\n");
            }
            saveQuestionsToFile();
            printf("  Question updated and saved.\n");
        }
        else
        {
            printf("  Invalid! Use E, N, P, or B.\n");
        }
    }
}
// SET QUIZ TIMER (ADMIN)
// Saves the timer setting so it stays the same next time the program runs
void saveTimerToFile()
{
    FILE *fp = fopen("timer.txt", "w");
    if(fp == NULL) { printf("  Warning: could not save timer setting.\n"); return; }
    fprintf(fp, "%d\n", g_timerSeconds);
    fclose(fp);
}
// Loads the saved timer setting on startup (if the file exists)
void loadTimerFromFile()
{
    FILE *fp = fopen("timer.txt", "r");
    if(fp == NULL) return;   // no saved setting yet, just keep the default
    int seconds;
    if(fscanf(fp, "%d", &seconds) == 1 && seconds > 0)
        g_timerSeconds = seconds;
    fclose(fp);
}
void setTimerSetting()
{
    int mins;
    printf("\n===== SET QUIZ TIMER =====\n");
    printf("  Current limit: %d minutes\n", g_timerSeconds / 60);
    printf("  Enter new time limit in minutes (1-60): ");
    scanf("%d", &mins);
    clearBuffer();
    if(mins < 1 || mins > 60) { printf("  Invalid. Keeping current.\n"); return; }
    g_timerSeconds = mins * 60;
    saveTimerToFile();
    printf("  Timer set to %d minutes.\n", mins);
}
// ADMIN SECTION
void adminSection()
{
    char enteredUsername[30], enteredPassword[30];
    int adminChoice;
    printf("\n===== ADMIN LOGIN =====\n");
    printf("Username: ");
    readLine(enteredUsername, sizeof(enteredUsername));
    printf("Password: ");
    readLine(enteredPassword, sizeof(enteredPassword));
    if(strcmp(enteredUsername, "admin") != 0 || strcmp(enteredPassword, "1234") != 0)
    {
        printf("\nInvalid Username or Password!\n");
        return;
    }
    printf("\nLogin Successful!\n");
    do
    {
        printf("\n===== ADMIN PANEL =====\n");
        printf("  1. Add / Replace Question\n");
        printf("  2. Delete Question\n");
        printf("  3. Search Questions\n");
        printf("  4. Edit Question\n");
        printf("  5. View All Questions\n");
        printf("  6. Set Quiz Timer\n");
        printf("  7. Back to Main Menu\n");
        printLine();
        printf("Enter Choice: ");
        scanf("%d", &adminChoice);
        clearBuffer();
        if(adminChoice == 1)      addQuestion();
        else if(adminChoice == 2) deleteQuestion();
        else if(adminChoice == 3) searchQuestions();
        else if(adminChoice == 4) editQuestion();
        else if(adminChoice == 5) viewQuestions();
        else if(adminChoice == 6) setTimerSetting();
        else if(adminChoice == 7) printf("\nReturning to Main Menu...\n");
        else                      printf("\nInvalid Choice!\n");
    } while(adminChoice != 7);
}
// SUBJECT / DIFFICULTY MENUS
int subjectMenu()
{
    int choice;
    printf("\n===== SELECT SUBJECT =====\n");
    printf("  1. Computer\n  2. Mathematics\n  3. Physics\n  4. English\n  5. General Knowledge\n  6. Exit\n");
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
    printf("  1. Easy\n  2. Medium\n  3. Hard\n  4. Back\n");
    printLine();
    printf("Enter Choice: ");
    scanf("%d", &choice);
    clearBuffer();
    return choice;
}
// STUDENT FILE HANDLING
int findStudent(char *username, char *outPassword, char *outFullName)
{
    FILE *fp = fopen("students.txt", "r");
    if(fp == NULL) return 0;
    char line[150], fileUser[30], filePass[30], fileName[60];
    while(fgets(line, sizeof(line), fp) != NULL)
    {
        line[strcspn(line, "\n")] = '\0';
        if(sscanf(line, "%29[^|]|%29[^|]|%59[^\n]", fileUser, filePass, fileName) == 3)
        {
            if(strcmp(fileUser, username) == 0)
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
    char dummyPass[30], dummyName[60];
    printf("\n===== STUDENT SIGN UP =====\n");
    printf("Choose a username: ");
    readLine(username, sizeof(username));
    if(strlen(username) == 0) { printf("  Username cannot be empty.\n"); return; }
    if(findStudent(username, dummyPass, dummyName))
    {
        printf("  That username is already taken.\n");
        return;
    }
    printf("Choose a password: ");
    readLine(password, sizeof(password));
    if(strlen(password) == 0) 
	{ printf("  Password cannot be empty.\n"); 
	return; 
	}
    printf("Enter your full name: ");
    readLine(fullname, sizeof(fullname));
    if(strlen(fullname) == 0) { printf("  Name cannot be empty.\n"); return; }
    FILE *fp = fopen("students.txt", "a");
    if(fp == NULL) { printf("  Error: could not save your account.\n"); return; }
    fprintf(fp, "%s|%s|%s\n", username, password, fullname);
    fclose(fp);
    printf("\n  Account created! Please login now.\n");
}
int studentLogin()
{
    char username[30], password[30];
    char storedPass[30], storedName[60];
    printf("\n===== STUDENT LOGIN =====\n");
    printf("Username: ");
    readLine(username, sizeof(username));
    printf("Password: ");
    readLine(password, sizeof(password));
    if(!findStudent(username, storedPass, storedName))
    {
        printf("\n  No account found with that username. Please Sign Up first.\n");
        return 0;
    }
    if(strcmp(password, storedPass) != 0)
    {
        printf("\n  Incorrect password!\n");
        return 0;
    }
    strcpy(g_currentUsername, username);
    printf("\n  Login Successful! Welcome, %s.\n", storedName);
    return 1;
}
void saveScore(char *username, char *subject, char *difficulty, int score, int timeTaken)
{
    FILE *fp = fopen("scores.txt", "a");
    if(fp == NULL) { printf("  Warning: could not save score.\n"); return; }
    fprintf(fp, "%s|%s|%s|%d|%d|%d\n", username, subject, difficulty, score, TOTAL_QUESTIONS, timeTaken);
    fclose(fp);
}
void viewMyScores()
{
    FILE *fp = fopen("scores.txt", "r");
    if(fp == NULL) { printf("\n  No score history yet.\n"); return; }
    char line[150], fileUser[30], subject[30], difficulty[20];
    int score, total, timeTaken, count = 0;
    printf("\n===== YOUR SCORE HISTORY (%s) =====\n", g_currentUsername);
    while(fgets(line, sizeof(line), fp) != NULL)
    {
        line[strcspn(line, "\n")] = '\0';
        if(sscanf(line, "%29[^|]|%29[^|]|%19[^|]|%d|%d|%d",
                  fileUser, subject, difficulty, &score, &total, &timeTaken) == 6)
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
    if(count == 0) printf("  No attempts yet.\n");
    printLine();
}
// FEEDBACK
void showFeedback(int score, int timeTaken)
{
    printf("\n=====================================\n");
    printf("              RESULT\n");
    printf("=====================================\n");
    printf("  Your Score  : %d / %d\n", score, TOTAL_QUESTIONS);
    printf("  Time Taken  : %02d:%02d\n", timeTaken/60, timeTaken%60);
    printf("  Time Limit  : %02d:%02d\n", g_timerSeconds/60, g_timerSeconds%60);
    printLine();
    if(score == TOTAL_QUESTIONS)      printf("  PERFECT! Outstanding!\n");
    else if(score >= 16)              printf("  Excellent! Very well done!\n");
    else if(score >= 12)              printf("  Good Job! Keep it up!\n");
    else if(score >= 8)               printf("  Fair! A bit more practice needed.\n");
    else if(score >= 4)               printf("  Keep Practicing!\n");
    else                              printf("  Don't give up! Try again!\n");
    printf("=====================================\n");
}
// RUN QUIZ
void runQuiz(int subject, int difficulty)
{
    char studentAnswers[TOTAL_QUESTIONS];
    int  answered[TOTAL_QUESTIONS];
    int  i, score = 0;
    char input[10];
    char *diffNames[] = {"Easy", "Medium", "Hard"};
    // subject here is 1-based (from subjectMenu), convert to 0-based index
    int subIdx = subject - 1;
    // Read straight from the shared arrays — same ones admin edits/adds to,
    // so any change made by the admin shows up here right away.
    char (*questions)[200] = allQuestions[subIdx][difficulty];
    char *answers = allAnswers[subIdx][difficulty];
    for(i = 0; i < TOTAL_QUESTIONS; i++) answered[i] = 0;
    time_t startTime = time(NULL);
    printf("\n===== %s QUIZ =====\n", subjectNames[subIdx]);
    printf("  Time Limit : %d minutes\n", g_timerSeconds / 60);
    printf("  [A/B/C/D = Answer | N = Next | P = Previous | Q = Quit & Submit]\n");
    printLine();
    i = 0;
    while(1)
    {
        int left = secondsLeft(startTime);
        if(left <= 0)
        {
            printf("\n  *** TIME'S UP! Quiz auto-submitted. ***\n");
            break;
        }
        printf("\nQuestion %d of %d:\n", i + 1, TOTAL_QUESTIONS);
        printTimeLeft(startTime);
        printf("%s\n", questions[i]);
        if(answered[i])
            printf("  (Saved answer: %c)\n", studentAnswers[i]);
        printf("  Your input: ");
        readLine(input, sizeof(input));
        if(secondsLeft(startTime) <= 0)
        {
            printf("\n  *** TIME'S UP! Quiz auto-submitted. ***\n");
            break;
        }
        char nav = toupper(input[0]);
        if(nav == 'N')
        {
            if(i < TOTAL_QUESTIONS - 1) i++;
            else printf("  This is the last question!\n");
        }
        else if(nav == 'P')
        {
            if(i > 0) i--;
            else printf("  This is the first question!\n");
        }
        else if(nav == 'Q')
        {
            printf("  Quit and submit? (Y/N): ");
            readLine(input, sizeof(input));
            if(toupper(input[0]) == 'Y') break;
        }
        else if(nav == 'A' || nav == 'B' || nav == 'C' || nav == 'D')
        {
            studentAnswers[i] = nav;
            answered[i] = 1;
            printf("  Answer saved: %c\n", nav);
            if(i < TOTAL_QUESTIONS - 1)
            {
                i++;
            }
            else
            {
                int allDone = 1, j;
                for(j = 0; j < TOTAL_QUESTIONS; j++)
                    if(!answered[j]) { allDone = 0; break; }
                if(allDone)
                {
                    printf("\n  All questions answered! N = Submit | P = Go Back: ");
                    readLine(input, sizeof(input));
                    nav = toupper(input[0]);
                    if(nav == 'N') break;
                    else if(nav == 'P' && i > 0) i--;
                }
            }
        }
        else
        {
            printf("  Invalid! Use A/B/C/D, N, P, or Q.\n");
        }
    }
    int timeTaken = (int)difftime(time(NULL), startTime);
    if(timeTaken > g_timerSeconds) timeTaken = g_timerSeconds;
    score = 0;
    for(i = 0; i < TOTAL_QUESTIONS; i++)
        if(answered[i] && studentAnswers[i] == answers[i])
            score++;
    printf("\n===== ANSWER REVIEW =====\n");
    for(i = 0; i < TOTAL_QUESTIONS; i++)
    {
        char given   = answered[i] ? studentAnswers[i] : '-';
        char correct = answers[i];
        printf("  Q%-2d  Your: %c   Correct: %c   [%s]\n",
               i+1, given, correct, (given == correct) ? "Correct" : "Incorrect");
    }
    showFeedback(score, timeTaken);
    saveScore(g_currentUsername, subjectNames[subIdx], diffNames[difficulty], score, timeTaken);
}
// STUDENT SECTION
void studentSection()
{
    int loggedIn = 0;
    while(!loggedIn)
    {
        int choice;
        printf("\n===== STUDENT SECTION =====\n");
        printf("  1. Login\n  2. Sign Up\n  3. Back\n");
        printLine();
        printf("Enter Choice: ");
        scanf("%d", &choice);
        clearBuffer();
        if(choice == 1)      loggedIn = studentLogin();
        else if(choice == 2) studentSignUp();
        else if(choice == 3) return;
        else                 printf("\nInvalid Choice!\n");
    }
    int subjectChoice, difficultyChoice, afterChoice;
    do
    {
        subjectChoice = subjectMenu();
        if(subjectChoice == 6) { printf("\nReturning to Main Menu...\n"); return; }
        if(subjectChoice < 1 || subjectChoice > 5) { printf("\nInvalid Choice!\n"); continue; }
        difficultyChoice = difficultyMenu();
        if(difficultyChoice == 4) { printf("\nGoing back...\n"); continue; }
        if(difficultyChoice < 1 || difficultyChoice > 3) { printf("\nInvalid Choice!\n"); continue; }
        runQuiz(subjectChoice, difficultyChoice - 1);
        printf("\n===== WHAT NEXT? =====\n");
        printf("  1. Choose Another Subject\n");
        printf("  2. View My Score History\n");
        printf("  3. Exit to Main Menu\n");
        printLine();
        printf("Enter Choice: ");
        scanf("%d", &afterChoice);
        clearBuffer();
        if(afterChoice == 2)      viewMyScores();
        else if(afterChoice == 3) { printf("\nReturning to Main Menu...\n"); return; }
    } while(1);
}
// MAIN
int main()
{
    int choice;
    loadQuestionsFromFile();   // load all 500 questions from questions.txt into memory
    loadTimerFromFile();       // load the admin's saved timer setting, if any
    do
    {
        choice = mainMenu();
        if(choice == 1)      adminSection();
        else if(choice == 2) studentSection();
        else if(choice == 3) printf("\nGoodbye!\n");
        else                 printf("\nInvalid Choice!\n");
    } while(choice != 3);
    return 0;
}
