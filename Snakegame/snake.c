#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>
#include <string.h>

int WIDTH = 60;
int HEIGHT = 22;
int SCREEN_WIDTH = 80;
int SCREEN_HEIGHT = 25;
int MAP_OFFSET_X = 0;
int MAP_OFFSET_Y = 0;

#define TOP_OFFSET 2
#define SPEED 140
#define FRUIT_COUNT 5
#define MAX_NAME 30
#define INTRO_SPEED 90
#define TABLE_SIZE 5000

/* ================= TEMA VISUAL ================= */

#define WALL_CHAR 219
#define BG_CHAR '.'
#define SNAKE_HEAD_CHAR 2
#define SNAKE_BODY_CHAR 'o'
#define FRUIT_CHAR '@'

#define HUD_COLOR 14
#define WALL_COLOR 11
#define HEAD_COLOR 10
#define BODY_COLOR 2
#define FRUIT_COLOR 12
#define BG_COLOR 8

#define LEGENDARY_CHANCE 1     /* 0.01%  (1 em 10000) */
#define SPECIAL_CHANCE 100     /* 1%     (100 em 10000) */
#define RANDOM_RANGE 10000

/* ================= HASH TABLE ================= */

int occupancyHash[TABLE_SIZE] = {0};

int get_hash_index(int x, int y) {
    return (y * WIDTH) + x;
}

/* ================= PROTOTIPOS ================= */

void update_console_size();
void disable_console_resize();

void reset_game();
int game_over_screen(char playerName[]);
void free_snake();
int main_menu();
void instructions_screen();
void draw_menu_option(int y, const char *text, int selected);
void draw_hud(char playerName[]);

void clearInsideArea();
void draw_center_text(int y, const char *text, int color);
void remove_newline(char *str);
void animate_snake_eating_name(const char *name);
void start_screen(char *playerName);

void draw_border();
void draw_background();
void generate_fruit(int index);
void update(char playerName[]);
void input();
void insert_head(int x, int y);
void remove_tail();
int collision_wall(int x, int y);
int collision_body(int x, int y);
int fruit_at(int x, int y);

void show_cat();
void show_skull();
void redraw_game(char playerName[]);

void setColor(int color);
void gotoxy(int x, int y);
void hideCursor();

extern HANDLE hConsole;

/* ================= ESTRUTURAS ================= */

typedef struct Node {
    int x;
    int y;
    struct Node *next;
    struct Node *prev;
} Node;

typedef struct {
    int x;
    int y;
    int type; /* 0 = normal, 1 = especial, 2 = lendaria */
} Fruit;

/* ================= VARIAVEIS GLOBAIS ================= */

Node *head = NULL;
Node *tail = NULL;
Fruit fruits[FRUIT_COUNT];

int dirX = 0;
int dirY = 0;
int score = 0;
int gameOver = 0;
int bestScore = 0;

HANDLE hConsole;

/* ================= TAMANHO TELA ================= */

void update_console_size() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);

    SCREEN_WIDTH = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    SCREEN_HEIGHT = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    if (SCREEN_WIDTH < WIDTH + 2) SCREEN_WIDTH = WIDTH + 2;
    if (SCREEN_HEIGHT < HEIGHT + 2) SCREEN_HEIGHT = HEIGHT + 2;

    MAP_OFFSET_X = (SCREEN_WIDTH - WIDTH) / 2;
    MAP_OFFSET_Y = (SCREEN_HEIGHT - HEIGHT) / 2;

    if (MAP_OFFSET_X < 0) MAP_OFFSET_X = 0;
    if (MAP_OFFSET_Y < 1) MAP_OFFSET_Y = 1;
}

void disable_console_resize() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cursorInfo);

    COORD bufferSize;
    bufferSize.X = (SHORT)WIDTH;
    bufferSize.Y = (SHORT)(HEIGHT + 5);
    SetConsoleScreenBufferSize(hOut, bufferSize);

    SMALL_RECT windowSize;
    windowSize.Left = 0;
    windowSize.Top = 0;
    windowSize.Right = (SHORT)(WIDTH - 1);
    windowSize.Bottom = (SHORT)HEIGHT;
    SetConsoleWindowInfo(hOut, TRUE, &windowSize);
}


/* ================= UTIL ================= */

void setColor(int color) {
    SetConsoleTextAttribute(hConsole, color);
}

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = (SHORT)x;
    coord.Y = (SHORT)y;
    SetConsoleCursorPosition(hConsole, coord);
}

void gotoxy_map(int x, int y) {
    COORD coord;
    coord.X = (SHORT)(x + MAP_OFFSET_X);
    coord.Y = (SHORT)(y + MAP_OFFSET_Y);
    SetConsoleCursorPosition(hConsole, coord);
}

void hideCursor() {
    CONSOLE_CURSOR_INFO ci;
    ci.dwSize = 1;
    ci.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &ci);
}

/* ================= SNAKE ================= */

void insert_head(int x, int y) {
    int idx = get_hash_index(x, y);

    if (idx < 0 || idx >= TABLE_SIZE) {
        gameOver = 1;
        return;
    }

    Node *newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        gameOver = 1;
        return;
    }

    newNode->x = x;
    newNode->y = y;
    newNode->next = head;
    newNode->prev = NULL;

    if (head != NULL)
        head->prev = newNode;

    head = newNode;

    if (tail == NULL)
        tail = newNode;

    occupancyHash[idx] = 1;
}

void remove_tail() {
    if (tail == NULL)
        return;

    int idx = get_hash_index(tail->x, tail->y);
    if (idx >= 0 && idx < TABLE_SIZE)
        occupancyHash[idx] = 0;

    Node *temp = tail;

    if (tail->prev != NULL) {
        tail = tail->prev;
        tail->next = NULL;
    } else {
        head = NULL;
        tail = NULL;
    }

    gotoxy_map(temp->x, temp->y);
    setColor(BG_COLOR);
    printf("%c", BG_CHAR);
    setColor(7);

    free(temp);
}

/* ================= COLISAO ================= */

int collision_wall(int x, int y) {
    if (x <= 0 || x >= WIDTH - 1 || y <= TOP_OFFSET || y >= HEIGHT - 1)
        return 1;
    return 0;
}

int collision_body(int x, int y) {
    int idx = get_hash_index(x, y);

    if (idx < 0 || idx >= TABLE_SIZE)
        return 1;

    if (occupancyHash[idx] == 1)
        return 1;

    return 0;
}

int fruit_at(int x, int y) {
    for (int i = 0; i < FRUIT_COUNT; i++) {
        if (fruits[i].x == x && fruits[i].y == y)
            return i;
    }
    return -1;
}

/* ================= GAME ================= */

void draw_border() {
    setColor(WALL_COLOR);

    for (int i = 0; i < WIDTH; i++) {
        gotoxy_map(i, TOP_OFFSET);
        printf("%c", WALL_CHAR);

        gotoxy_map(i, HEIGHT - 1);
        printf("%c", WALL_CHAR);
    }

    for (int i = TOP_OFFSET; i < HEIGHT; i++) {
        gotoxy_map(0, i);
        printf("%c", WALL_CHAR);

        gotoxy_map(WIDTH - 1, i);
        printf("%c", WALL_CHAR);
    }

    setColor(7);
}

void draw_background() {
    setColor(BG_COLOR);

    for (int y = TOP_OFFSET + 1; y < HEIGHT - 1; y++) {
        for (int x = 1; x < WIDTH - 1; x++) {
            gotoxy_map(x, y);
            printf("%c", BG_CHAR);
        }
    }

    setColor(7);
}

void generate_fruit(int index) {
    int x, y, valid;

    do {
        valid = 1;

        x = rand() % (WIDTH - 2) + 1;
        y = rand() % (HEIGHT - TOP_OFFSET - 2) + TOP_OFFSET + 1;

        int idx = get_hash_index(x, y);

        if (idx < 0 || idx >= TABLE_SIZE)
            valid = 0;

        if (valid && occupancyHash[idx] == 1)
            valid = 0;

        for (int i = 0; i < FRUIT_COUNT; i++) {
            if (i != index && fruits[i].x == x && fruits[i].y == y) {
                valid = 0;
                break;
            }
        }
    } while (!valid);

    fruits[index].x = x;
    fruits[index].y = y;

    int r = rand() % RANDOM_RANGE;

    if (r < LEGENDARY_CHANCE)
        fruits[index].type = 2;
    else if (r < LEGENDARY_CHANCE + SPECIAL_CHANCE)
        fruits[index].type = 1;
    else
        fruits[index].type = 0;

    gotoxy_map(x, y);

    if (fruits[index].type == 2) {
        setColor(12);
        printf("$");
    } else if (fruits[index].type == 1) {
        setColor(13);
        printf("&");
    } else {
        setColor(FRUIT_COLOR);
        printf("%c", FRUIT_CHAR);
    }

    setColor(7);
}

void update(char playerName[]) {
    if ((dirX == 0 && dirY == 0) || head == NULL)
        return;

    int newX = head->x + dirX;
    int newY = head->y + dirY;

    if (collision_wall(newX, newY)) {
        gameOver = 1;
        return;
    }

    int fruitIndex = fruit_at(newX, newY);
    int ateFruit = (fruitIndex != -1);

    if (!ateFruit && tail != NULL && newX == tail->x && newY == tail->y) {
        /* permitido */
    } else if (collision_body(newX, newY)) {
        gameOver = 1;
        return;
    }

    setColor(BODY_COLOR);
    gotoxy_map(head->x, head->y);
    printf("%c", SNAKE_BODY_CHAR);

    insert_head(newX, newY);

    setColor(HEAD_COLOR);
    gotoxy_map(newX, newY);

    char headChar = 'O';
    if (dirX == 1) headChar = '>';
    if (dirX == -1) headChar = '<';
    if (dirY == 1) headChar = 'v';
    if (dirY == -1) headChar = '^';

    printf("%c", headChar);
    setColor(7);

    if (ateFruit) {
        int type = fruits[fruitIndex].type;

        if (type == 2) {
            score += 50;
            show_skull();
            generate_fruit(fruitIndex);
            redraw_game(playerName);
        } else if (type == 1) {
            score += 20;
            show_cat();
            generate_fruit(fruitIndex);
            redraw_game(playerName);
        } else {
            score += 10;
            generate_fruit(fruitIndex);
        }
    } else {
        remove_tail();
    }
}

void input() {
    if (_kbhit()) {
        int key = _getch();

        if (key == 0 || key == 224) {
            key = _getch();

            switch (key) {
                case 75:
                    if (dirX != 1) {
                        dirX = -1;
                        dirY = 0;
                    }
                    break;

                case 77:
                    if (dirX != -1) {
                        dirX = 1;
                        dirY = 0;
                    }
                    break;

                case 72:
                    if (dirY != 1) {
                        dirX = 0;
                        dirY = -1;
                    }
                    break;

                case 80:
                    if (dirY != -1) {
                        dirX = 0;
                        dirY = 1;
                    }
                    break;
            }
        } else {
            switch (key) {
                case 'a':
                case 'A':
                    if (dirX != 1) {
                        dirX = -1;
                        dirY = 0;
                    }
                    break;

                case 'd':
                case 'D':
                    if (dirX != -1) {
                        dirX = 1;
                        dirY = 0;
                    }
                    break;

                case 'w':
                case 'W':
                    if (dirY != 1) {
                        dirX = 0;
                        dirY = -1;
                    }
                    break;

                case 's':
                case 'S':
                    if (dirY != -1) {
                        dirX = 0;
                        dirY = 1;
                    }
                    break;
            }
        }
    }
}

/* ================= MENU RETRO ================= */

void draw_menu_option(int y, const char *text, int selected) {
    int len = (int)strlen(text);
    int x = (SCREEN_WIDTH - len) / 2;

    gotoxy(x - 2, y);

    if (selected) {
        setColor(10);
        printf("> %s", text);
    } else {
        setColor(7);
        printf("  %s", text);
    }

    setColor(7);
}

void instructions_screen() {
    system("cls");
    update_console_size();

    draw_center_text(4, "=========== INSTRUCOES ===========", 11);
    draw_center_text(7, "Use as SETAS ou WASD para mover", 14);
    draw_center_text(9, "Coma as frutas para crescer", 10);
    draw_center_text(11, "Nao bata na parede ou no corpo", 12);
    draw_center_text(13, "Fruta & = gato secreto", 13);
    draw_center_text(14, "Fruta $ = caveira lendaria", 12);
    draw_center_text(17, "Pressione qualquer tecla para voltar", 7);

    while (!_kbhit()) {
        Sleep(50);
    }

    _getch();
}

int main_menu() {
    int selected = 0;
    const int totalOptions = 3;
    int needRedraw = 1;

    system("cls");
    update_console_size();

    draw_center_text(4, "#############################", 10);
    draw_center_text(5, "#        SNAKE GAME         #", 10);
    draw_center_text(6, "#############################", 10);
    draw_center_text(8, "===== MENU PRINCIPAL =====", 14);
    draw_center_text(SCREEN_HEIGHT - 3, "Use as setas e ENTER", 8);

    while (1) {
        if (needRedraw) {
            draw_menu_option(11, "Jogar", selected == 0);
            draw_menu_option(12, "Instrucoes", selected == 1);
            draw_menu_option(13, "Sair", selected == 2);
            needRedraw = 0;
        }

        if (_kbhit()) {
            int key = _getch();

            if (key == 0 || key == 224) {
                key = _getch();

                if (key == 72) {
                    selected--;
                    if (selected < 0)
                        selected = totalOptions - 1;
                    needRedraw = 1;
                } else if (key == 80) {
                    selected++;
                    if (selected >= totalOptions)
                        selected = 0;
                    needRedraw = 1;
                }
            } else if (key == 13) {
                return selected;
            }
        }

        Sleep(20);
    }
}

/* ================= INTRO ================= */

void clearInsideArea() {
    for (int y = TOP_OFFSET + 1; y < HEIGHT - 1; y++) {
        gotoxy(1, y);
        setColor(BG_COLOR);
        for (int x = 1; x < WIDTH - 1; x++) {
            printf("%c", BG_CHAR);
        }
    }
    setColor(7);
}

void draw_center_text(int y, const char *text, int color) {
    int len = (int)strlen(text);
    int x = (SCREEN_WIDTH - len) / 2;

    if (x < 1)
        x = 1;

    setColor(color);
    gotoxy(x, y);
    printf("%s", text);
    setColor(7);
}

void remove_newline(char *str) {
    int len = (int)strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

void animate_snake_eating_name(const char *name) {
    int len = (int)strlen(name);

    if (len == 0) return;

    int spacing = 2;
    int totalWidth = len + (len - 1) * spacing;
    int startX = (WIDTH - totalWidth) / 2;
    int nameY = HEIGHT / 2;

    int letterX[MAX_NAME];
    int eaten[MAX_NAME] = {0};

    int snakeX[200];
    int snakeY[200];
    int prevSnakeX[200];
    int prevSnakeY[200];
    int snakeLen = 4;

    int headStartX = startX - 6;

    draw_center_text(4, "A COBRA ESTA COMENDO SEU NOME...", 14);

    for (int i = 0; i < len; i++) {
        letterX[i] = startX + i * (spacing + 1);
        setColor(11);
        gotoxy_map(letterX[i], nameY);
        printf("%c", name[i]);
    }
    setColor(7);

    for (int i = 0; i < snakeLen; i++) {
        snakeX[i] = headStartX - i;
        snakeY[i] = nameY;
        prevSnakeX[i] = snakeX[i];
        prevSnakeY[i] = snakeY[i];
    }

    int finished = 0;

    while (!finished) {
        finished = 1;

        for (int i = 0; i < snakeLen; i++) {
            prevSnakeX[i] = snakeX[i];
            prevSnakeY[i] = snakeY[i];
        }

        for (int i = snakeLen - 1; i > 0; i--) {
            snakeX[i] = snakeX[i - 1];
            snakeY[i] = snakeY[i - 1];
        }

        snakeX[0]++;

        for (int i = 0; i < len; i++) {
            if (!eaten[i] && snakeX[0] >= letterX[i]) {
                eaten[i] = 1;

                gotoxy_map(letterX[i], nameY);
                setColor(BG_COLOR);
                printf("%c", BG_CHAR);
                setColor(7);

                if (snakeLen < 199) {
                    snakeX[snakeLen] = snakeX[snakeLen - 1];
                    snakeY[snakeLen] = snakeY[snakeLen - 1];
                    prevSnakeX[snakeLen] = snakeX[snakeLen];
                    prevSnakeY[snakeLen] = snakeY[snakeLen];
                    snakeLen++;
                }
            }
        }

        for (int i = 0; i < len; i++) {
            if (!eaten[i]) {
                finished = 0;
                break;
            }
        }

        for (int i = 0; i < snakeLen; i++) {
            if (prevSnakeX[i] > 0 && prevSnakeX[i] < WIDTH - 1 &&
                prevSnakeY[i] > TOP_OFFSET && prevSnakeY[i] < HEIGHT - 1) {
                gotoxy_map(prevSnakeX[i], prevSnakeY[i]);
                setColor(BG_COLOR);
                printf("%c", BG_CHAR);
            }
        }

        for (int i = snakeLen - 1; i >= 0; i--) {
            if (snakeX[i] > 0 && snakeX[i] < WIDTH - 1 &&
                snakeY[i] > TOP_OFFSET && snakeY[i] < HEIGHT - 1) {
                gotoxy_map(snakeX[i], snakeY[i]);

                if (i == 0) {
                    setColor(HEAD_COLOR);
                    printf("%c", SNAKE_HEAD_CHAR);
                } else {
                    setColor(BODY_COLOR);
                    printf("%c", SNAKE_BODY_CHAR);
                }
            }
        }

        setColor(7);
        Sleep(INTRO_SPEED);
    }

    Sleep(700);
}

void start_screen(char *playerName) {
    system("cls");
    update_console_size();

    draw_center_text(4, "=============================", 11);
    draw_center_text(5, "      JOGO DA COBRINHA       ", 10);
    draw_center_text(6, "=============================", 11);

    draw_center_text(9, "Digite seu nome:", 14);

    setColor(15);
    gotoxy((SCREEN_WIDTH / 2) - 10, 11);

    fgets(playerName, MAX_NAME, stdin);
    remove_newline(playerName);

    if (strlen(playerName) == 0) {
        strcpy(playerName, "Jogador");
    }

    system("cls");
    update_console_size();
    draw_border();
    draw_background();
    animate_snake_eating_name(playerName);
    Sleep(300);

    system("cls");
}

/* ================= RESET / GAME OVER ================= */

void free_snake() {
    while (tail != NULL) {
        remove_tail();
    }
}

void reset_game() {
    free_snake();

    for (int i = 0; i < TABLE_SIZE; i++) {
        occupancyHash[i] = 0;
    }

    score = 0;
    gameOver = 0;
    dirX = 0;
    dirY = 0;

    int startX = WIDTH / 2;
    int startY = HEIGHT / 2;

    insert_head(startX, startY);

    setColor(HEAD_COLOR);
    gotoxy_map(startX, startY);
    printf("%c", SNAKE_HEAD_CHAR);
    setColor(7);

    for (int i = 0; i < FRUIT_COUNT; i++) {
        generate_fruit(i);
    }
}

int game_over_screen(char playerName[]) {
    if (score > bestScore) {
        bestScore = score;
    }

    system("cls");
    update_console_size();

    draw_center_text(6, "========================", 12);
    draw_center_text(7, "       GAME OVER        ", 12);
    draw_center_text(8, "========================", 12);

    char scoreText[50];
    sprintf(scoreText, "Pontuacao final: %d", score);
    draw_center_text(10, scoreText, 14);

    char bestText[50];
    sprintf(bestText, "Melhor pontuacao: %d", bestScore);
    draw_center_text(11, bestText, 10);

    char playerText[80];
    sprintf(playerText, "Jogador: %s", playerName);
    draw_center_text(12, playerText, 11);

    draw_center_text(15, "[ R ] Reiniciar", 10);
    draw_center_text(16, "[ M ] Menu Principal", 14);
    draw_center_text(17, "[ ESC ] Sair", 7);

    while (1) {
        if (_kbhit()) {
            int key = _getch();

            if (key == 'r' || key == 'R')
                return 1;

            if (key == 'm' || key == 'M')
                return 2;

            if (key == 27)
                return 0;
        }
        Sleep(50);
    }
}

void draw_hud(char playerName[]) {
    gotoxy(2, 0);
    setColor(HUD_COLOR);
    printf("SCORE: %d   ", score);

    gotoxy((SCREEN_WIDTH / 2) - 10, 0);
    setColor(11);
    printf("PLAYER: %s   ", playerName);

    gotoxy(SCREEN_WIDTH - 18, 0);
    setColor(10);
    printf("RECORD: %d   ", bestScore);

    setColor(7);
}

/* ================= EASTER EGG ================= */

void redraw_game(char playerName[]) {
    system("cls");
    update_console_size();
    draw_border();
    draw_background();
    draw_hud(playerName);

    for (int i = 0; i < FRUIT_COUNT; i++) {
        gotoxy_map(fruits[i].x, fruits[i].y);

        if (fruits[i].type == 2) {
            setColor(12);
            printf("$");
        } else if (fruits[i].type == 1) {
            setColor(13);
            printf("&");
        } else {
            setColor(FRUIT_COLOR);
            printf("%c", FRUIT_CHAR);
        }
    }

    Node *current = tail;
    while (current != NULL) {
        gotoxy_map(current->x, current->y);

        if (current == head) {
            char headChar = 'O';
            if (dirX == 1) headChar = '>';
            if (dirX == -1) headChar = '<';
            if (dirY == 1) headChar = 'v';
            if (dirY == -1) headChar = '^';

            setColor(HEAD_COLOR);
            printf("%c", headChar);
        } else {
            setColor(BODY_COLOR);
            printf("%c", SNAKE_BODY_CHAR);
        }

        current = current->prev;
    }

    setColor(7);
}

void show_cat() {
    system("cls");
    update_console_size();

    draw_center_text(3, "EASTER EGG!", 14);
    draw_center_text(5, "Voce encontrou o gato secreto!", 11);

    draw_center_text(8,  " /\\_/\\\\ ", 10);
    draw_center_text(9,  "( o.o )", 10);
    draw_center_text(10, " > ^ < ", 10);

    draw_center_text(13, "Miau!", 13);

    Sleep(2500);
}

void show_skull() {
    system("cls");
    update_console_size();

    draw_center_text(3, "FRUTA LENDARIA!", 12);
    draw_center_text(5, "Voce despertou a caveira...", 12);

    draw_center_text(8,  "  _____ ", 8);
    draw_center_text(9,  " /     \\", 8);
    draw_center_text(10, "| () () |", 8);
    draw_center_text(11, " \\  ^  /", 8);
    draw_center_text(12, "  |||||", 8);
    draw_center_text(13, "  |||||", 8);

    draw_center_text(15, "CUIDADO...", 12);

    Sleep(3000);
}

/* ================= MAIN ================= */

int main() {
    char playerName[MAX_NAME];
    int running = 1;

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    disable_console_resize();
    srand((unsigned int)time(NULL));
    hideCursor();
    update_console_size();

    while (running) {
        int option = main_menu();

        if (option == 0) {
            start_screen(playerName);

            while (1) {
                system("cls");
                update_console_size();
                draw_border();
                draw_background();
                reset_game();
                draw_hud(playerName);

                while (!gameOver) {
                    input();
                    update(playerName);
                    draw_hud(playerName);
                    Sleep(SPEED);
                }

                int result = game_over_screen(playerName);

                if (result == 1) {
                    continue;
                } else if (result == 2) {
                    break;
                } else {
                    running = 0;
                    break;
                }
            }
        } else if (option == 1) {
            instructions_screen();
        } else if (option == 2) {
            running = 0;
        }
    }

    system("cls");
    setColor(7);
    printf("Obrigado por jogar!\n");

    free_snake();
    return 0;
}