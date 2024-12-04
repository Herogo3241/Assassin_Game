#include <ncurses.h>
#include <math.h>
#include <stdlib.h>

// Define screen size
#define screenWidth 100
#define screenHeight 30

// Define the map (1 = wall, 0 = empty space)
int map[5][5] = {
    {1, 1, 1, 1, 1},
    {1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1},
    {1, 1, 1, 1, 1}
};

// Player's position and direction
float playerX = 2.5, playerY = 2.5;  // Initial position
float dirX = -1, dirY = 0;           // Facing left initially
float planeX = 0, planeY = 0.66;     // Camera plane for FOV

// Function to render the frame using raycasting
void renderFrame() {
    clear(); // Clear the screen

    for (int x = 0; x < screenWidth; x++) {
        // Calculate ray position and direction
        float cameraX = 2 * x / (float)screenWidth - 1; // X in camera space
        float rayDirX = dirX + planeX * cameraX;
        float rayDirY = dirY + planeY * cameraX;

        // Which box of the map we're in
        int mapX = (int)playerX;
        int mapY = (int)playerY;

        // Length of ray from one x or y-side to next x or y-side
        float deltaDistX = fabs(1 / rayDirX);
        float deltaDistY = fabs(1 / rayDirY);

        float sideDistX, sideDistY; // Length to the next x or y-side

        // Step direction and initial side distance
        int stepX, stepY;
        int hit = 0; // Was a wall hit?
        int side;    // Was the wall hit a vertical or horizontal one?

        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (playerX - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0 - playerX) * deltaDistX;
        }

        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (playerY - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0 - playerY) * deltaDistY;
        }

        // Perform DDA
        while (!hit) {
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
            if (map[mapX][mapY] > 0) hit = 1;
        }

        // Calculate distance to the wall
        float perpWallDist;
        if (side == 0)
            perpWallDist = (mapX - playerX + (1 - stepX) / 2) / rayDirX;
        else
            perpWallDist = (mapY - playerY + (1 - stepY) / 2) / rayDirY;

        // Calculate height of the line to draw on the screen
        int lineHeight = (int)(screenHeight / perpWallDist);

        // Calculate start and end points of the wall slice
        int drawStart = -lineHeight / 2 + screenHeight / 2;
        if (drawStart < 0) drawStart = 0;
        int drawEnd = lineHeight / 2 + screenHeight / 2;
        if (drawEnd >= screenHeight) drawEnd = screenHeight - 1;

        // Choose wall character based on side
        char wallChar = (side == 1) ? '|' : '#';

        // Draw the wall slice
        for (int y = drawStart; y < drawEnd; y++) {
            mvaddch(y, x, wallChar);
        }
    }

    refresh(); // Refresh the screen
}

// Main function
int main() {
    // Initialize ncurses
    initscr();
    noecho();
    curs_set(FALSE);

    // Game loop
    while (1) {
        renderFrame();

        // Get user input
        int ch = getch();
        if (ch == 'q') break; // Quit the game

        // Movement
        if (ch == 'w') { // Move forward
            if (map[(int)(playerX + dirX * 0.1)][(int)playerY] == 0) playerX += dirX * 0.1;
            if (map[(int)playerX][(int)(playerY + dirY * 0.1)] == 0) playerY += dirY * 0.1;
        }
        if (ch == 's') { // Move backward
            if (map[(int)(playerX - dirX * 0.1)][(int)playerY] == 0) playerX -= dirX * 0.1;
            if (map[(int)playerX][(int)(playerY - dirY * 0.1)] == 0) playerY -= dirY * 0.1;
        }
        if (ch == 'a') { // Rotate left
            float oldDirX = dirX;
            dirX = dirX * cos(-0.1) - dirY * sin(-0.1);
            dirY = oldDirX * sin(-0.1) + dirY * cos(-0.1);
            float oldPlaneX = planeX;
            planeX = planeX * cos(-0.1) - planeY * sin(-0.1);
            planeY = oldPlaneX * sin(-0.1) + planeY * cos(-0.1);
        }
        if (ch == 'd') { // Rotate right
            float oldDirX = dirX;
            dirX = dirX * cos(0.1) - dirY * sin(0.1);
            dirY = oldDirX * sin(0.1) + dirY * cos(0.1);
            float oldPlaneX = planeX;
            planeX = planeX * cos(0.1) - planeY * sin(0.1);
            planeY = oldPlaneX * sin(0.1) + planeY * cos(0.1);
        }
    }

    // Cleanup ncurses
    endwin();
    return 0;
}
