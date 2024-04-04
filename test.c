#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

// #include "flappy_bg.h"

volatile int pixel_buffer_start; // global variable
short int Buffer1[240][512];     // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];

#define x_bird 50
#define pipe_width 50
#define pipe_color 0x07E0

typedef struct flappyBird
{
    int y_bird;
} flappyBird;

typedef struct pipe
{
    int x;
    int up_height;
    int low_height;
} pipe;

pipe pipes[100];

// Read the keyboard (pressed button)
void read_keyboard(unsigned char *pressedKey);

// Plot the pixel
void plot_pixel(int x, int y, short int line_color);
void draw_background(int x, int y, const unsigned char *flappy_bg, volatile int pixel_buffer_address);
// Draw the buffer using charbuffer
void draw_string(int x, int y, char str[]);
// Draw the char which will be used in the draw_string
void draw_char(int x, int y, char letter);
// Clear the screen with the black color
void clear_screen();
// Waiting for the buffer swapping
void wait_for_vsync();

// Muhammad Irfan //

// Draw the background
void draw_bg();
// Draw the pipe
void draw_pipe(pipe pipes);
// Random the size of the pipe for top and bottom pipe
pipe random_init();
// Increase the level
int level(int totalPipe, int *speed);
// Show the notification for the gameoever
void GameOver();
// Show the score in the screen
void proto_Score(int *curScore);
// Clear the text in the screen
void clear_all_text();
// Start the game for the first time
void startGame(unsigned char *pressedKey);
// If we press N button, restart the game
bool initializeGame(unsigned char *pressedKey);
// Display the score on the hex 4_5
void displayScore();
// If we press H button, show and update the highestscore on the hex 0_3
void displayHighScore(unsigned char *pressedKey);
// If we press P button, pause the game
void pauseGame(unsigned char *pressedKey);
// We will have a countdown before it start again
int countdown(int x, int y, int seconds);
// Draw the string on the screen of the image
void drawCenteredString(char *str);
// Change the user when the game stopped or not started yet
void changeUser(unsigned char *pressedKey);
// Add the user/player
void addUser();
// Display the user on the screen
void displayUser(int *curUser);

/// Muhammad Abdullah ///

// Draw the bird
void draw_bird(flappyBird bird);
// Control the bird using ps2 keyboard
void control_bird(flappyBird *bird, unsigned char *pressedKey);
// Check if pipes
void hitPipe(flappyBird bird);
// Audio for the background
void audio_bg();

#define MAX_PIPES 4
#define HEX4_5 ((volatile long *)0xFF200030)
#define HEX2_3 ((volatile long *)0xFF200028)
#define HEX0_1 ((volatile long *)0xFF200020)

const int hexdisplay[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67};

int delayX = 0;
int num_of_pipe = 1;
int currentScore = 0;
int speed = 3;
int highestScore = 0;
int winnerUser = 0;
int currentUser = 1;
int totalUser = 1;

int prevScore = 0;
int prevUser = 0;

bool GameStarted = false;
bool GameRetried = false;
bool GamePaused = false;
bool firstGame = true;
bool gameRestart = false;
bool gameOverFlag = false;

int main(void)
{
    volatile int *ps2_ctrl_ptr = (int *)0xFF200100;
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    /* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;

    unsigned char pressedKey = 0;
    read_keyboard(&pressedKey);

    flappyBird bird = {120};
    draw_bird(bird);

    /* set front pixel buffer to Buffer 1 */
    *(pixel_ctrl_ptr + 1) = (int)&Buffer1; // first store the address in the  back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer

    draw_bg();

    /* set back pixel buffer to Buffer 2 */
    *(pixel_ctrl_ptr + 1) = (int)&Buffer2;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer

    pipes[0] = random_init();
    proto_Score(&currentScore);
    displayUser(&currentUser);

    draw_string(30, 30, "Start the game");

    while (1)
    {
        pressedKey = 0;
        if (GameRetried)
        {
            read_keyboard(&pressedKey);
            if (pressedKey == 0x29)
            {
                countdown(40, 30, 3);
                gameOverFlag = false;
                GameRetried = false;
                GameStarted = true;
            }
        }

        if (!GameStarted)
        {
            if (pressedKey == 0x1C)
            {
                addUser();
                pressedKey = 0;
            }

            changeUser(&pressedKey);
        }
        read_keyboard(&pressedKey);

        initializeGame(&pressedKey); // If you press N, it will restart the game

        pauseGame(&pressedKey); // If you press N, it will pause the game, click s to continue

        startGame(&pressedKey); // Start the game for the first time

        displayScore(); // Display the score on the hex 4_5

        displayHighScore(&pressedKey); // Display the HighScore and the user who get it on the hex 0_3

        if (GameStarted && !GamePaused)
        {
            clear_all_text();           // Clear the text on the screen
            proto_Score(&currentScore); // Show the score on the screen
            displayUser(&currentUser);  // Show the user on the screen
            draw_bg();                  // Draw the background

            draw_bird(bird); // Draw the bird

            control_bird(&bird, &pressedKey); // Control the bird using space button
            hitPipe(bird);                    // Check if the bird hitting the pipe

            level(num_of_pipe, &speed); // Increase the difficulty

            // Update the game state
            for (int i = 0; i < num_of_pipe; ++i)
            {
                pipes[i].x -= speed; // Move the pipe left
                // Check if the pipe is off the screen
                if (pipes[i].x < -pipe_width)
                {
                    if (num_of_pipe >= MAX_PIPES)
                    {
                        pipes[i] = random_init();
                        pipes[i].x = 320; // Move the reused pipe to the right
                        currentScore += 1;
                    }
                }
            }

            // Draw the game state
            for (int i = 0; i < num_of_pipe; ++i)
            {
                draw_pipe(pipes[i]);
            }

            // Add new pipes if necessary
            if (delayX >= 30)
            {
                if (num_of_pipe < MAX_PIPES)
                {
                    num_of_pipe++;
                    pipes[num_of_pipe - 1] = random_init();
                    currentScore += 1;
                }
                delayX = 0; // Reset the delay counter regardless of whether a new pipe was added or not
            }
            else
            {
                delayX++; // Increment the delay counter
            }

            // If we press N button, it will stop the game before someone press space button
            // to start the game again
            if (gameRestart)
            {
                GameRetried = true;
                GameStarted = false;
                gameRestart = false;
            }

            wait_for_vsync();                           // swap front and back buffers on VGA vertical sync
            pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
        }
    }
}

///////////////////////// Muhammad Irfan ////////////////////////////

void drawCenteredString(char *str)
{

    int screenWidth = 320;
    int screenHeight = 240;
    int charsPerRow = 8;
    int charsPerColumn = 8;

    int charWidth = screenWidth / charsPerRow;
    int charHeight = screenHeight / charsPerColumn;

    int stringLengthPixels = strlen(str) * charWidth;
    int x = (screenWidth - stringLengthPixels) / 2;
    int y = (screenHeight - charHeight) / 2;

    // Now use x and y as the starting point to draw your string
    draw_string(x, y, str);
}

void pauseGame(unsigned char *pressedKey)
{
    if (*pressedKey == 0x4D && GamePaused == false)
    {
        GamePaused = true;
    }
    if (*pressedKey == 0x1B && GamePaused == true)
    {
        GamePaused = false;
        countdown(40, 30, 3);
    }
}

void addUser()
{
    totalUser = totalUser + 1;
    char new_String[30];
    sprintf(new_String, "Add Player %d", totalUser);
    draw_string(30, 30, new_String);
    return;
}

void changeUser(unsigned char *pressedKey)
{
    if (*pressedKey == 0x3C)
    {
        if (currentUser == totalUser)
        {
            currentUser = 1;
        }
        else
        {
            currentUser += 1;
        }
        char new_string[30];
        sprintf(new_string, "Change player to Player %d", currentUser);
        draw_string(30, 30, new_string);
    }
}

void displayHighScore(unsigned char *pressedKey)
{
    if (highestScore < currentScore)
    {
        winnerUser = currentUser;
        highestScore = currentScore;
    }
    if (*pressedKey == 0x33)
    {
        int tens = highestScore / 10;
        int ones = highestScore % 10;

        int tensUser = winnerUser / 10;
        int onesUser = winnerUser % 10;

        int displayHighestAndUserScore = (hexdisplay[tensUser] << 24) | (hexdisplay[onesUser] << 16) | (hexdisplay[tens] << 8) | hexdisplay[ones];

        *(HEX0_1) = displayHighestAndUserScore;
    }
}

void displayScore()
{
    int tens = currentScore / 10;
    int ones = currentScore % 10;

    int displayHighScore = (hexdisplay[tens] << 8) | hexdisplay[ones];

    *(HEX4_5) = displayHighScore;
}

bool initializeGame(unsigned char *pressedKey)
{
    if (*pressedKey == 0x31)
    {
        memset(pipes, 0, sizeof(pipes));
        delayX = 0;
        num_of_pipe = 1;
        currentScore = 0;
        speed = 3;
        gameRestart = true;
        pipes[0] = random_init();

        return true;
    }
    return false;
}

void startGame(unsigned char *pressedKey)
{
    if (firstGame == true)
    {
        if (*pressedKey == 0x29)
        {
            GameStarted = true;
            firstGame == false;
        }
    }
}

void read_keyboard(unsigned char *pressedKey)
{
    static unsigned char lastKey = 0; // Store the last key pressed
    static int keyReleasePending = 0; // Flag to indicate waiting for a key release code
    volatile int *PS2_ptr = (int *)0xFF200100;
    int data = *PS2_ptr;

    if (data & 0x8000)
    { // If there's input
        *pressedKey = data & 0xFF;
        if (*pressedKey == 0xF0)
        { // Key release signal
            keyReleasePending = 1;
        }
        else if (keyReleasePending)
        {
            if (*pressedKey == lastKey)
            { // The key that was pressed has been released
                keyReleasePending = 0;
                lastKey = 0;     // Reset lastKey to allow a new key press
                *pressedKey = 0; // Ignore this as it's a release code
            }
        }
        else if (*pressedKey != lastKey)
        {
            lastKey = *pressedKey; // Update lastKey to the new press
        }
        else
        {
            *pressedKey = 0; // No new key press or still waiting for release
        }
    }
    else
    {
        *pressedKey = 0; // No input detected
    }
}

void clear_all_text()
{
    for (int x = 0; x < 80; x++)
    {
        for (int y = 0; y < 60; y++)
        {
            draw_char(x, y, ' ');
        }
    }
}

void draw_string(int x, int y, char str[])
{
    for (int i = 0; i < strlen(str); i++)
    {
        draw_char(x + i, y, str[i]);
    }
}

void draw_char(int x, int y, char letter)
{
    volatile int charBuffer = 0x09000000;
    *(char *)(charBuffer + (y << 7) + x) = letter;
}

void draw_bg()
{
    for (int k = 0; k < 153600; k += 2)
    {
        int red = ((flappy_bg[k + 1] & 0xF8) >> 3) << 11;
        int green = (((flappy_bg[k] & 0xE0) >> 5)) | ((flappy_bg[k + 1] & 0x7) << 3);
        int blue = (flappy_bg[k] & 0x1f);

        short int p = red | ((green << 5) | blue);
        plot_pixel((k / 2) % 320, (k / 2) / 320, p);
    }

    // for (int x = 0; x < 320; x++)
    // {
    //     for (int y = 0; y < 240; y++)
    //     {
    //         plot_pixel(x, y, 0x0xae19);
    //     }
    // }
}

void draw_pipe(pipe cur_pipe)
{
    int pipe_height = 240;
    int gap = 80; // Gap between the upper and lower pipes

    // Draw the lower pipe, assuming a gap of 80 pixels
    int startY = cur_pipe.up_height + gap; // Start drawing the lower pipe after the gap

    for (int y = startY; y < pipe_height; y++)
    {
        for (int x = 0; x < pipe_width; x++)
        {
            int index = ((y - startY) * pipe_width + x) * 2; // Correct the index for the lower pipe bitmap
            int red = ((lower_pipe[index + 1] & 0xF8) >> 3) << 11;
            int green = (((lower_pipe[index] & 0xE0) >> 5)) | ((lower_pipe[index + 1] & 0x7) << 3);
            int blue = (lower_pipe[index] & 0x1f);

            short int color = red | (green << 5) | blue;
            plot_pixel(cur_pipe.x + x, y, color);
        }
    }

    // Draw the upper pipe

    // change the y = 0 to be 240
    for (int y = cur_pipe.low_height + gap; y < 240; y++)
    {
        for (int x = 0; x < pipe_width; x++)
        {
            int index = (y * pipe_width + x) * 2; // Assuming 2 bytes per pixel
            int red = ((upper_pipe[index + 1] & 0xF8) >> 3) << 11;
            int green = (((upper_pipe[index] & 0xE0) >> 5)) | ((upper_pipe[index + 1] & 0x7) << 3);
            int blue = (upper_pipe[index] & 0x1f);

            short int color = red | (green << 5) | blue;
            plot_pixel(cur_pipe.x + x, y - cur_pipe.low_height - gap, color);
        }
    }
}

void GameOver()
{
}

int countdown(int x, int y, int seconds)
{
    int fps = 60;
    char buffer[32];

    for (int i = seconds; i > 0; i--)
    {
        draw_string(x, y, "   ");
        sprintf(buffer, "%d", i);
        draw_string(x, y, buffer);

        for (int j = 0; j < 50000000; j++)
        {
        }
    }
    draw_string(x, y, "   ");
}

void proto_Score(int *curScore)
{
    char scoreStr[20];
    sprintf(scoreStr, "Score: %d", *curScore); // Combine "Score: " with the integer score value into scoreStr.
    prevScore = *curScore;

    draw_string(5, 8, scoreStr); // Display the concatenated score string on the screen.
}

void displayUser(int *curUser)
{
    char player[20];

    sprintf(player, "Player %d", *curUser);
    prevUser = *curUser;

    draw_string(5, 5, player); // Display the concatenated player string on the screen.
}

int level(int totalPipe, int *speed)
{
    // Problem: We need to know the relation or the ratio between the speed and the delayX
    if (totalPipe > 30)
    {
        *speed += 2;
        totalPipe += 30;
    }
}

pipe random_init()
{
    int height = 240;
    int gap = 80;
    int min_height = 40;
    int low_height = 40 + rand() % (height - min_height - min_height - 1);
    int up_height = height - low_height - gap;

    pipe new_pipe = {320, low_height, up_height};

    return new_pipe;
}

void wait_for_vsync()
{
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    volatile int *status = (int *)0xFF20302C;

    *pixel_ctrl_ptr = 1;

    while ((*status & 0x01) != 0)
    {
        status = status; // keep reading status;
    }

    // exit when S is 1
    return;
}

void clear_screen()
{
    for (int x = 0; x < 320; x++)
    {
        for (int y = 0; y < 240; y++)
        {
            plot_pixel(x, y, 0x0000);
        }
    }
}

void plot_pixel(int x, int y, short int line_color)
{
    // Plot the pixel
    *(volatile short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

///////////////////////// Muhammad Abdullah ////////////////////////////

void draw_bird(flappyBird bird)
{
    for (int y = 0; y < 50; y++)
    {
        for (int x = 0; x < 50; x++)
        {
            int index = ((y * 50) + x) * 2; // adjusts the index position of the bird
            int red = ((flappy_bd[index + 1] & 0xF8) >> 3) << 11;
            int green = (((flappy_bd[index] & 0xE0) >> 5)) | ((flappy_bd[index + 1] & 0x7) << 3);
            int blue = (flappy_bd[index] & 0x1f);

            short int color = red | (green << 5) | blue;

            plot_pixel(x, bird.y_bird + y, color);
        }
    }
}

void hitPipe(flappyBird bird)
{
    int first_pipe_onScreen;
    if (num_of_pipe > 4)
    {
        first_pipe_onScreen = num_of_pipe - 4;
    }
    else
    {
        first_pipe_onScreen = 0;
    }
    for (int i = first_pipe_onScreen; i < num_of_pipe; i++)
    {
        if ((x_bird >= pipes[i].x - pipe_width && x_bird <= pipes[i].x) && ((bird.y_bird >= 0 & bird.y_bird <= pipes[i].up_height) && bird.y_bird <= 320 && bird.y_bird >= (pipes[i].up_height + 80)))
        {
            draw_string(40, 30, "Game Over");
            gameOverFlag = true;
        }
    }
}

void control_bird(flappyBird *bird, unsigned char *pressedKey)
{
    if (bird->y_bird > 1 && bird->y_bird < 319)
    {
        bird->y_bird += 4;
        if (*pressedKey == 0x29)
        {
            bird->y_bird -= 20;
        }
    }
    else if (bird->y_bird == 1)
    {
        bird->y_bird += 4;
    }
    else if (bird->y_bird == 319)
    {
        if (*pressedKey == 0x29)
        {
            bird->y_bird -= 20;
        }
    }
}
