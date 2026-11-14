/* ===========================================================
   TREASURE HUNT: DUNGEON ESCAPE
   Uses only <stdio.h>

   Collect all treasures (T), avoid traps (X), then reach the
   exit (E). Try Torch Mode: you can only see nearby cells and
   your score is DOUBLED!
   =========================================================== */
#include <stdio.h>

/* ---------- Structure ---------- */
struct Player {
    char name[20];
    int score;
    int moves;
    int level;
};

/* ---------- Global data ---------- */
struct Player players[50];     /* leaderboard: array of structs (max 50) */
int playerCount = 0;
char maze[10][10];             /* 2D array: 10 rows x 10 columns */

/* ---------- Function prototypes ---------- */
void clearBuffer(void);
int getNumber(int low, int high);
int absValue(int x);
int sameString(char a[], char b[]);
void copyString(char dest[], char src[]);
void loadLevel(int level, int *playerRow, int *playerCol);
void printMaze(int pr, int pc, int lives, int treasures, int moves, int mode);
int countTreasures(int row, int col);          /* recursive */
int nearestTreasure(int row, int col);
int calculateScore(int collected, int moves, int won, int mode);
void showRank(int score);
void playGame(int level, int mode);
void addScore(char name[], int score, int moves, int level);
void sortScores(void);
void showScores(void);
void searchScore(void);
void saveScores(void);
void loadScores(void);
void showBanner(void);
void showInstructions(void);
void playMenu(void);
void leaderboardMenu(void);

/* ---------- Input helpers ---------- */
void clearBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

/* Keeps asking until the user enters a number between low and high */
int getNumber(int low, int high) {
    int value;
    while (1) {
        printf("Enter choice (%d-%d): ", low, high);
        if (scanf("%d", &value) != 1) {
            printf("Invalid input! Please enter a number.\n");
            clearBuffer();
        } else if (value < low || value > high) {
            printf("Out of range! Try again.\n");
            clearBuffer();
        } else {
            clearBuffer();
            return value;
        }
    }
}

int absValue(int x) {
    if (x < 0)
        return -x;
    return x;
}

/* ---------- String helpers (no string.h needed) ---------- */
int sameString(char a[], char b[]) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        char x = a[i];
        char y = b[i];
        /* make both lowercase so search is not case-sensitive */
        if (x >= 'A' && x <= 'Z') x = x + 32;
        if (y >= 'A' && y <= 'Z') y = y + 32;
        if (x != y) return 0;
        i++;
    }
    return a[i] == b[i];   /* both must end together */
}

void copyString(char dest[], char src[]) {
    int i = 0;
    while (src[i] != '\0' && i < 19) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

/* ---------- Maze ---------- */
/* # = wall, . = path, T = treasure, X = trap, E = exit, S = start */
void loadLevel(int level, int *playerRow, int *playerCol) {
    char level1[10][11] = {
        "##########",
        "#S...#..T#",
        "#.##.#.#.#",
        "#.#..#.#.#",
        "#.#.##.#.#",
        "#.#..X.#.#",
        "#.##.###.#",
        "#T.#...#.#",
        "#.....T#E#",
        "##########"
    };
    char level2[10][11] = {
        "##########",
        "#S.#....T#",
        "#..#.##..#",
        "#X.#.#T#.#",
        "#.##.#.#.#",
        "#....#.#.#",
        "#.####.#X#",
        "#.#T...#.#",
        "#...##.#.E",
        "##########"
    };
    char level3[10][11] = {
        "##########",
        "#S.X...#T#",
        "#.##.#.#.#",
        "#.#T.#...#",
        "#.#.####.#",
        "#.#.X..#.#",
        "#.####.#.#",
        "#T.....#X#",
        "#.#####..E",
        "##########"
    };
    int r, c;

    for (r = 0; r < 10; r++) {
        for (c = 0; c < 10; c++) {
            switch (level) {
                case 1: maze[r][c] = level1[r][c]; break;
                case 2: maze[r][c] = level2[r][c]; break;
                default: maze[r][c] = level3[r][c]; break;
            }
            if (maze[r][c] == 'S') {
                *playerRow = r;     /* pointers: give back 2 values */
                *playerCol = c;
                maze[r][c] = '.';
            }
        }
    }
}

/* mode 1 = normal, mode 2 = torch (you only see 3 steps around you) */
void printMaze(int pr, int pc, int lives, int treasures, int moves, int mode) {
    int r, c, i;
    printf("\n  Lives: ");
    for (i = 0; i < lives; i++)
        printf("<3 ");
    printf("  Treasures left: %d   Moves: %d\n", treasures, moves);
    if (mode == 2)
        printf("  [TORCH MODE - double points]\n");
    printf("\n  +---------------------+\n");
    for (r = 0; r < 10; r++) {
        printf("  | ");
        for (c = 0; c < 10; c++) {
            if (r == pr && c == pc)
                printf("P ");
            else if (mode == 2 && absValue(r - pr) + absValue(c - pc) > 3)
                printf("? ");               /* too dark to see */
            else
                printf("%c ", maze[r][c]);
        }
        printf("|\n");
    }
    printf("  +---------------------+\n\n");
}

/* RECURSIVE: counts treasures by checking one cell, then calling
   itself for the next cell. Start with countTreasures(0, 0). */
int countTreasures(int row, int col) {
    if (row == 10)              /* base case: went past the last row */
        return 0;
    if (col == 10)              /* end of row: go to the next row */
        return countTreasures(row + 1, 0);
    if (maze[row][col] == 'T')
        return 1 + countTreasures(row, col + 1);
    return countTreasures(row, col + 1);
}

/* Returns how many steps away the closest treasure is (-1 if none) */
int nearestTreasure(int row, int col) {
    int r, c, d, best = -1;
    for (r = 0; r < 10; r++) {
        for (c = 0; c < 10; c++) {
            if (maze[r][c] == 'T') {
                d = absValue(r - row) + absValue(c - col);
                if (best == -1 || d < best)
                    best = d;
            }
        }
    }
    return best;
}

int calculateScore(int collected, int moves, int won, int mode) {
    int score = collected * 20;
    if (won) {
        score = score + 50;
        if (moves < 50)
            score = score + (50 - moves);   /* bonus for fewer moves */
    }
    if (mode == 2)
        score = score * 2;                  /* torch mode bonus */
    return score;
}

void showRank(int score) {
    if (score >= 200)
        printf("Rank: LEGENDARY TREASURE HUNTER!\n");
    else if (score >= 130)
        printf("Rank: Master Explorer\n");
    else if (score >= 60)
        printf("Rank: Brave Adventurer\n");
    else
        printf("Rank: Rookie (keep practicing!)\n");
}

/* ---------- Gameplay ---------- */
void playGame(int level, int mode) {
    int row, col, newRow, newCol, hint;
    int lives = 3, moves = 0, collected = 0;
    int treasuresLeft, won = 0, score;
    char move, name[20];

    loadLevel(level, &row, &col);
    treasuresLeft = countTreasures(0, 0);

    printf("\n=== LEVEL %d ===\n", level);
    printf("W/A/S/D = move   H = hint (costs 3 moves)   Q = quit\n");

    while (lives > 0) {
        printMaze(row, col, lives, treasuresLeft, moves, mode);
        printf("Move: ");
        scanf(" %c", &move);
        clearBuffer();

        newRow = row;
        newCol = col;

        switch (move) {
            case 'w': case 'W': newRow--; break;
            case 's': case 'S': newRow++; break;
            case 'a': case 'A': newCol--; break;
            case 'd': case 'D': newCol++; break;
            case 'h': case 'H':
                hint = nearestTreasure(row, col);
                if (hint == -1)
                    printf("No treasure left! Head for the exit (E).\n");
                else
                    printf("HINT: the nearest treasure is %d steps away.\n", hint);
                moves = moves + 3;
                continue;
            case 'q': case 'Q': lives = 0; continue;
            default:
                printf("Invalid key! Use W, A, S, D, H or Q.\n");
                continue;
        }

        /* boundary and wall check */
        if (newRow < 0 || newRow > 9 || newCol < 0 || newCol > 9 ||
            maze[newRow][newCol] == '#') {
            printf("Ouch! You hit a wall.\n");
            continue;
        }

        row = newRow;
        col = newCol;
        moves++;

        if (maze[row][col] == 'T') {
            collected++;
            treasuresLeft--;
            maze[row][col] = '.';
            printf("** You found a treasure! (+20) **\n");
        } else if (maze[row][col] == 'X') {
            lives--;
            maze[row][col] = '.';
            printf("!! TRAP! You lost a life. !!\n");
        } else if (maze[row][col] == 'E') {
            if (treasuresLeft == 0) {
                won = 1;
                break;
            } else {
                printf("The exit is locked! %d treasure(s) still hidden.\n", treasuresLeft);
            }
        }
    }

    if (won)
        printf("\n*** YOU ESCAPED THE DUNGEON! ***\n");
    else
        printf("\nGAME OVER.\n");

    score = calculateScore(collected, moves, won, mode);
    printf("Treasures: %d   Moves: %d   Score: %d\n", collected, moves, score);
    showRank(score);

    printf("Enter your name (no spaces): ");
    scanf("%19s", name);
    clearBuffer();
    addScore(name, score, moves, level);
    saveScores();
}

/* ---------- Leaderboard ---------- */
void addScore(char name[], int score, int moves, int level) {
    int i;
    if (playerCount == 50) {
        printf("Leaderboard full! Replacing the lowest score.\n");
        sortScores();
        i = playerCount - 1;
    } else {
        i = playerCount;
        playerCount++;
    }
    copyString(players[i].name, name);
    players[i].score = score;
    players[i].moves = moves;
    players[i].level = level;
}

/* Bubble sort: highest score first */
void sortScores(void) {
    int i, j;
    struct Player temp;
    for (i = 0; i < playerCount - 1; i++) {
        for (j = 0; j < playerCount - i - 1; j++) {
            if (players[j].score < players[j + 1].score) {
                temp = players[j];
                players[j] = players[j + 1];
                players[j + 1] = temp;
            }
        }
    }
}

void showScores(void) {
    int i;
    if (playerCount == 0) {
        printf("\nNo scores yet. Play a game first!\n");
        return;
    }
    sortScores();
    printf("\n%-5s %-20s %-8s %-8s %-6s\n", "Rank", "Name", "Score", "Moves", "Level");
    printf("-------------------------------------------------------\n");
    for (i = 0; i < playerCount; i++) {
        printf("%-5d %-20s %-8d %-8d %-6d\n", i + 1, players[i].name,
               players[i].score, players[i].moves, players[i].level);
    }
}

/* Linear search by name (not case-sensitive) */
void searchScore(void) {
    char name[20];
    int i, found = 0;
    printf("Enter name to search: ");
    scanf("%19s", name);
    clearBuffer();
    for (i = 0; i < playerCount; i++) {
        if (sameString(name, players[i].name)) {
            printf("Found: %s  Score: %d  Moves: %d  Level: %d\n",
                   players[i].name, players[i].score,
                   players[i].moves, players[i].level);
            found = 1;
        }
    }
    if (!found)
        printf("No player named %s found.\n", name);
}

/* ---------- File handling ---------- */
void saveScores(void) {
    FILE *fp = fopen("scores.txt", "w");
    int i;
    if (fp == NULL) {
        printf("Error: could not save scores.\n");
        return;
    }
    fprintf(fp, "%d\n", playerCount);
    for (i = 0; i < playerCount; i++) {
        fprintf(fp, "%s %d %d %d\n", players[i].name, players[i].score,
                players[i].moves, players[i].level);
    }
    fclose(fp);
    printf("Scores saved.\n");
}

void loadScores(void) {
    FILE *fp = fopen("scores.txt", "r");
    int i, count;
    if (fp == NULL) {
        printf("No saved scores found (starting fresh).\n");
        return;
    }
    if (fscanf(fp, "%d", &count) != 1 || count < 0 || count > 50) {
        printf("Score file is corrupt. Ignoring it.\n");
        fclose(fp);
        return;
    }
    playerCount = 0;
    for (i = 0; i < count; i++) {
        if (fscanf(fp, "%19s %d %d %d", players[i].name, &players[i].score,
                   &players[i].moves, &players[i].level) != 4) {
            printf("Score file is damaged. Loaded %d records.\n", playerCount);
            break;
        }
        playerCount++;
    }
    fclose(fp);
    printf("Loaded %d saved scores.\n", playerCount);
}

/* ---------- Menus ---------- */
void showBanner(void) {
    printf("\n");
    printf("  ==================================\n");
    printf("  |     TREASURE HUNT              |\n");
    printf("  |        DUNGEON ESCAPE          |\n");
    printf("  |                                |\n");
    printf("  |   T = gold     X = trap        |\n");
    printf("  ==================================\n");
}

void showInstructions(void) {
    printf("\n--- HOW TO PLAY ---\n");
    printf("P = you      # = wall      . = path\n");
    printf("T = treasure X = trap      E = exit\n");
    printf("1. Collect ALL treasures (T). The exit only opens after that.\n");
    printf("2. Each trap (X) costs 1 life. You start with 3 lives.\n");
    printf("3. Fewer moves means a higher score.\n");
    printf("4. Torch Mode: you only see 3 steps around you, but score x2.\n");
    printf("5. Press H for a hint (distance to nearest treasure, costs 3 moves).\n");
    printf("Controls: W A S D to move, H for hint, Q to quit.\n");
}

void playMenu(void) {
    int level, mode;
    printf("\n--- SELECT LEVEL ---\n");
    printf("1. Level 1 (Easy)\n2. Level 2 (Medium)\n3. Level 3 (Hard)\n4. Back\n");
    level = getNumber(1, 4);
    if (level == 4)
        return;

    printf("\n--- SELECT MODE ---\n");
    printf("1. Normal\n2. Torch Mode (fog, double points)\n");
    mode = getNumber(1, 2);

    playGame(level, mode);
}

void leaderboardMenu(void) {
    int choice;
    do {
        printf("\n--- LEADERBOARD ---\n");
        printf("1. Show scores\n2. Search by name\n3. Save scores\n4. Load scores\n5. Back\n");
        choice = getNumber(1, 5);
        switch (choice) {
            case 1: showScores(); break;
            case 2: searchScore(); break;
            case 3: saveScores(); break;
            case 4: loadScores(); break;
            case 5: break;
        }
    } while (choice != 5);
}

/* ---------- Main ---------- */
int main(void) {
    int choice;

    showBanner();
    loadScores();

    do {
        printf("\n--- MAIN MENU ---\n");
        printf("1. Play\n2. Leaderboard\n3. Instructions\n4. Exit\n");
        choice = getNumber(1, 4);
        switch (choice) {
            case 1: playMenu(); break;
            case 2: leaderboardMenu(); break;
            case 3: showInstructions(); break;
            case 4: saveScores(); printf("Goodbye, adventurer!\n"); break;
        }
    } while (choice != 4);

    return 0;
}
