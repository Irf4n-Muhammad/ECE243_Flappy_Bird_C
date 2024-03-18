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
#define pipe_color 0x00FF00

typedef struct flappyBird
{
    int life;
    int x;
    int y;
    int dx;
    int dy;
} flappyBird;

typedef struct pipe
{
    int x;
    int up_height;
    int low_height;
} pipe;

pipe pipes[4];

void plot_pixel(int x, int y, short int line_color);
void draw_background(int x, int y, const unsigned char *flappy_bg, volatile int pixel_buffer_address);
void draw_string(int x, int y, char str[]);
void draw_char(int x, int y, char letter);
void clear_screen();
void draw_image();
void draw_pipe(pipe pipes);
pipe random_init();
void wait_for_vsync();

int main(void)
{
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    /* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;

    clear_screen();
    // draw_image();
    int delayX = 0;
    int num_of_pipe = 1;

    pipes[0] = random_init();
    draw_pipe(pipes[0]);

    /* set front pixel buffer to Buffer 1 */
    *(pixel_ctrl_ptr + 1) = (int)&Buffer1; // first store the address in the  back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer

    /* set back pixel buffer to Buffer 2 */
    *(pixel_ctrl_ptr + 1) = (int)&Buffer2;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer

    while (1)
    {
        // draw_background(0, 0, flappy_bg, pixel_buffer_start);
        clear_screen();
        pipes[0].x -= 5;
        draw_pipe(pipes[0]);
        for (int i = 0; i < num_of_pipe; ++i)
        {
            pipes[i].x -= 5;
            draw_pipe(pipes[i]);
        }

        // if (delayX == 10)
        // {
        //     pipes[num_of_pipe - 1] = random_init();
        //     num_of_pipe++;
        //     delayX = 0;
        // }
        // else
        // {
        //     delayX++;
        // }
        // draw_string(20, 20, "Start the game");
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

    // We can adjust the resolution
    for (int x = 0; x < 320; x++)
    {
        for (int y = 0; y < 240; y++)
        {
            // Plot the pixel with black color
            plot_pixel(x, y, 0x0000);
        }
    }
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
