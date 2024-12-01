#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#define HEIGHT 25
#define WIDTH 100

#define SPACE 0
#define WALL 1
#define HIDINGSPOTS 2
#define PLAYER 3
#define DOOR 4
#define HIDDEN 5
#define ENEMY 6
#define LIGHT 7

typedef struct {
    int x;
    int y;
    float angle;    // Angle of the searchlight in radians
    float sweepDir; // Direction of sweep (1 or -1)
} Enemy;

int maze[100][100];
int playerX, playerY;
Enemy enemy;
int gameOver = 0;

// Function to check if a point is within the searchlight cone
int isInSearchlight(int x, int y, Enemy *e) {
    float dx = x - e->x;
    float dy = y - e->y;
    float pointAngle = atan2(dy, dx);
    float distance = sqrt(dx*dx + dy*dy);
    float coneAngle = 0.5;
    float maxDistance = 15.0;
    
    while (pointAngle < 0) pointAngle += 2*M_PI;
    while (e->angle < 0) e->angle += 2*M_PI;
    
    float angleDiff = fabs(pointAngle - e->angle);
    if (angleDiff > M_PI) angleDiff = 2*M_PI - angleDiff;
    
    return (angleDiff < coneAngle && distance < maxDistance);
}

void updateEnemy() {
    enemy.angle += 0.1 * enemy.sweepDir;
    
    if (enemy.angle > M_PI/2 || enemy.angle < -M_PI/2) {
        enemy.sweepDir *= -1;
    }
    
    // Clear previous light
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (maze[i][j] == LIGHT) {
                maze[i][j] = SPACE;
            }
        }
    }
    
    // Update light positions
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (maze[i][j] != WALL && maze[i][j] != ENEMY && 
                isInSearchlight(j, i, &enemy)) {
                if (maze[i][j] == PLAYER) {
                    gameOver = 1;
                }
                if (maze[i][j] == SPACE) {
                    maze[i][j] = LIGHT;
                }
            }
        }
    }
}

void createEnemy() {
    do {
        enemy.x = WIDTH/2 + rand() % (WIDTH/3);
        enemy.y = HEIGHT/3 + rand() % (HEIGHT/3);
    } while (maze[enemy.y][enemy.x] != SPACE);
    
    maze[enemy.y][enemy.x] = ENEMY;
    enemy.angle = 0;
    enemy.sweepDir = 1;
}

void createGameWindow()
{
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            if (i == 0 || j == 0 || i == HEIGHT - 1 || j == WIDTH - 1)
            {
                maze[i][j] = WALL;
            }
            else
            {
                maze[i][j] = SPACE;
            }
        }
    }
}

void createRandomHidingSpots(int numHidingSpots)
{
    int k = 0;
    while (k < numHidingSpots)
    {
        int startX, startY;

        do
        {
            startX = rand() % (WIDTH - 3);
            startY = rand() % (HEIGHT - 3);
        } while (startX == 0 || startX == WIDTH - 1 || startY == HEIGHT - 1 || startY == 0 || maze[startY][startX] != SPACE);

        maze[startY][startX] = HIDINGSPOTS;

        k++;
    }
}

void removeElements()
{
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            if (maze[i][j] == HIDINGSPOTS || maze[i][j] == DOOR || maze[i][j] == ENEMY || maze[i][j] == LIGHT)
                maze[i][j] = SPACE;
        }
    }
}

void spawnDoor()
{
    int doorX, doorY;
    do
    {
        doorX = WIDTH / 1.2 + rand() % (WIDTH / 2);
        doorY = rand() % HEIGHT;
    } while (doorX >= WIDTH || doorY >= HEIGHT || maze[doorY][doorX] != SPACE);

    maze[doorY][doorX] = DOOR;

    if (doorX + 2 < WIDTH)
    { 
        for (int i = 0; i < 2; i++)
        {
            if (maze[doorY][doorX + i] == WALL)
                continue;
            maze[doorY][doorX + i] = DOOR;
        }
    }
}

void createPlayer()
{
    playerX = 2;
    playerY = 2;
    maze[playerY][playerX] = PLAYER;
}

int PlayerMovement(int ch)
{
    int newPlayerX = playerX;
    int newPlayerY = playerY;
    switch (ch)
    {
    case 'w':
        newPlayerY--;
        break;
    case 's':
        newPlayerY++;
        break;
    case 'a':
        newPlayerX--;
        break;
    case 'd':
        newPlayerX++;
        break;
    }

    if(maze[newPlayerY][newPlayerX] == HIDINGSPOTS){
        maze[playerY][playerX] = SPACE;
        maze[newPlayerY][newPlayerX] = HIDDEN;
        playerX = newPlayerX;
        playerY = newPlayerY;
    }
    else if(maze[playerY][playerX] == HIDDEN){
        maze[playerY][playerX] = HIDINGSPOTS;
        maze[newPlayerY][newPlayerX] = PLAYER;
        playerX = newPlayerX;
        playerY = newPlayerY;
    }
    else if (maze[newPlayerY][newPlayerX] == SPACE) 
    {
        maze[playerY][playerX] = SPACE;
        maze[newPlayerY][newPlayerX] = PLAYER;
        playerX = newPlayerX;
        playerY = newPlayerY;
    }
    else if(maze[newPlayerY][newPlayerX] == DOOR){
        maze[playerY][playerX] = SPACE;
        return 1;
    }

    return 0;
}

void displayGameWindow()
{
    int originX = abs(COLS - WIDTH) / 2;
    int originY = abs(LINES - HEIGHT) / 2;
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            move(i + originY, j + originX);
            if (maze[i][j] == SPACE)
            {
                addch(' ');
            }
            else if (maze[i][j] == WALL)
            {
                attron(COLOR_PAIR(2));
                addch(' ');
                attroff(COLOR_PAIR(2));
            }
            else if (maze[i][j] == HIDINGSPOTS)
            {
                attron(COLOR_PAIR(1));
                addch(' ');
                attroff(COLOR_PAIR(1));
            }
            else if (maze[i][j] == DOOR)
            {
                attron(COLOR_PAIR(3));
                addch(' ');
                attroff(COLOR_PAIR(3));
            }
            else if (maze[i][j] == PLAYER)
            {
                attron(COLOR_PAIR(4));
                addch(' ');
                attroff(COLOR_PAIR(4));
            }
            else if(maze[i][j] == HIDDEN)
            {
                attron(COLOR_PAIR(5));
                addch('H');
                attroff(COLOR_PAIR(5));
            }
            else if(maze[i][j] == ENEMY)
            {
                attron(COLOR_PAIR(6));
                addch('E');
                attroff(COLOR_PAIR(6));
            }
            else if(maze[i][j] == LIGHT)
            {
                attron(COLOR_PAIR(7));
                addch('.');
                attroff(COLOR_PAIR(7));
            }
        }
    }

    if (gameOver) {
        mvprintw(LINES/2, COLS/2 - 5, "GAME OVER!");
    }
}

int main()
{
    initscr();
    noecho();
    cbreak();
    curs_set(0);

    createGameWindow();
    removeElements();
    createRandomHidingSpots(30);
    spawnDoor();
    createPlayer();
    createEnemy();
    
    if (has_colors())
    {
        start_color();
        init_pair(1, COLOR_BLACK, COLOR_WHITE);   // SPACE (black on white)
        init_pair(2, COLOR_YELLOW, COLOR_YELLOW); // WALL
        init_pair(3, COLOR_MAGENTA, COLOR_MAGENTA); // door
        init_pair(4, COLOR_GREEN, COLOR_GREEN); // player
        init_pair(5, COLOR_WHITE, COLOR_BLUE); // Hidden
        init_pair(6, COLOR_RED, COLOR_BLACK);  // Enemy
        init_pair(7, COLOR_RED, COLOR_BLACK); // Searchlight
    }

    while (!gameOver)
    {
        displayGameWindow();
        refresh();
        updateEnemy();
        
        int ch = getch();
        int win = PlayerMovement(ch);
        if(win){
            removeElements();
            createRandomHidingSpots(30);
            spawnDoor();
            createPlayer();
            createEnemy();
        }
        if (ch == 'q')
        {
            break;
        }
    }

    if (gameOver) {
        getch();
    }

    endwin();
    return 0;
}