#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// #include "flappy_bg.h"

volatile int pixel_buffer_start; // global variable
short int Buffer1[240][512];     // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];

#define pipe_width 50
#define pipe_color 0x07E0

typedef struct bird
{
    int life;
    int x;
    int y;
} bird;

typedef struct pipe
{
    int x;
    int up_height;
    int low_height;
} pipe;

pipe pipes[5];

void plot_pixel(int x, int y, short int line_color);
void draw_background(int x, int y, const unsigned char *flappy_bg, volatile int pixel_buffer_address);
void draw_string(int x, int y, char str[]);
void draw_char(int x, int y, char letter);
void clear_screen();
void wait_for_vsync();

// Muhammad Irfan //
void draw_image();
void draw_pipe(pipe pipes);
pipe random_init();
void shiftPipesToLeft(pipe pipes[], int *num_of_pipe);
int level(int totalPipe, int *speed);
void GameOver();
int score(bird *flappy, int *curScore, const int *num_pipe, pipe *pipes);

/// Muhammad Abdullah ///
void draw_bird();    // Draw the bird
void control_bird(); // Control the bird using ps2 keyboard
bool hitPipe();      // Check if pipes
void audio_bg();     // Audio for the background
void startGame();    // Start the game controller using ps2 keyboard

#define MAX_PIPES 4

int main(void)
{
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    /* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;

    int delayX = 0;
    int num_of_pipe = 1;
    int *currentScore = 0;
    int *speed = 3;
    int *total_pipe = 0;

    pipes[0] = random_init();

    /* set front pixel buffer to Buffer 1 */
    *(pixel_ctrl_ptr + 1) = (int)&Buffer1; // first store the address in the  back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer

    // draw_image();

    /* set back pixel buffer to Buffer 2 */
    *(pixel_ctrl_ptr + 1) = (int)&Buffer2;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer

    int currentPipeIndex = 0; // Index of the next pipe to be moved or reinitialized

    while (1)
    {
        clear_screen();

        // Update the game state
        for (int i = 0; i < num_of_pipe; ++i)
        {
            pipes[i].x -= *speed; // Move the pipe left
            // Check if the pipe is off the screen
            if (pipes[i].x < -pipe_width)
            {
                if (num_of_pipe >= MAX_PIPES)
                {
                    pipes[i] = random_init();
                    pipes[i].x = 320; // Move the reused pipe to the right
                }
                else
                {
                    // The condition where a pipe goes off-screen and we have fewer than max pipes should not happen
                    // But if it does, handle it by initializing a new pipe at the end of the array
                    // This could be a place to throw an error or reset the game state if it's unexpected behavior
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
                pipes[num_of_pipe + 1] = random_init();
            }
            delayX = 0; // Reset the delay counter regardless of whether a new pipe was added or not
        }
        else
        {
            delayX++; // Increment the delay counter
        }

        wait_for_vsync();                           // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }
}

///////////////////////// Muhammad Irfan ////////////////////////////

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

void draw_image()
{
    // for (int k = 0; k < 153600; k += 2)
    // {
    //     int red = ((flappy_bg[k + 1] & 0xF8) >> 3) << 11;
    //     int green = (((flappy_bg[k] & 0xE0) >> 5)) | ((flappy_bg[k + 1] & 0x7) << 3);
    //     int blue = (flappy_bg[k] & 0x1f);

    //     short int p = red | ((green << 5) | blue);
    //     plot_pixel((k / 2) % 320, (k / 2) / 320, p);
    // }
}

void draw_pipe(pipe cur_pipe)
{
    // for (int k = 0; k < 2400; k+=2) {
    // 	int red = ((Mario_pipe[k + 1] & 0xF8) >> 3) << 11;
    // 	int green = (((Mario_pipe[k] & 0xE0) >> 5)) | ((Mario_pipe[k+1] & 0x7) << 3) ;
    // 	int blue = (Mario_pipe[k] & 0x1f);

    // 	short int p = red | ( (green << 5) | blue);
    // 	plot_pixel(50 + (k/2)%40, (k/2)/40, p);
    // }

    for (int i = 0; i < pipe_width; i++)
    {
        for (int j = 0; j < cur_pipe.low_height; j++)
        {
            plot_pixel(cur_pipe.x - i, j, pipe_color);
        }
        for (int j = 0; j < cur_pipe.up_height; j++)
        {
            plot_pixel(cur_pipe.x - i, 240 - j, pipe_color);
        }
        // cur_pipe.x += 1;
    }
}

void GameOver()
{
}

int score(bird flappy, int *curScore, const int *num_pipe, pipe *pipes)
{
    for (int i = 0; i < num_pipe; i++)
    {
        if (flappy.x > pipes[i].x)
        {
            *curScore += 1;
        }
    }
}

int level(int totalPipe, int *speed)
{
    if (totalPipe > 30)
    {
        *speed += 3;
    }
}

pipe random_init()
{
    int height = 240;
    int low_height = 20 + rand() % (height - 20 - 20 - 1);
    int gap = 80;
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
