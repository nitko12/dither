#include <stdio.h>
#include <stdlib.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define LOADBMP_IMPLEMENTATION
#include "loadbmp.h"

// Floyd Steinberg
// int coef = 16;
// int kernelX = 1;
// unsigned char kernel[3][4] = {
//     {0, 0, 7},
//     {3, 5, 1},
// };

// J F Jarvis, C N Judice, and W H Ninke "Minimized Average Error"
// int coef = 48;
// int kernelX = 2;
// unsigned char kernel[3][5] = {
//     {0, 0, 0, 7, 5},
//     {3, 5, 7, 5, 3},
//     {1, 3, 5, 3, 1},
// };

// Atkinson
int coef = 8;
int kernelX = 1;
unsigned char kernel[3][4] = {
    {0, 0, 1, 1},
    {1, 1, 1, 0},
    {0, 1, 0, 0},
};

// Burkes
// int coef = 32;
// int kernelX = 2;
// unsigned char kernel[3][5] = {
//     {0, 0, 0, 8, 4},
//     {2, 4, 8, 4, 2},
//     {0, 0, 0, 0, 0},
// };

// Stucki
// int coef = 42;
// int kernelX = 2;
// unsigned char kernel[3][5] = {
//     {0, 0, 0, 8, 4},
//     {2, 4, 8, 4, 2},
//     {1, 2, 4, 2, 1},
// };

// Sierra lite
// int coef = 4;
// int kernelX = 1;
// unsigned char kernel[3][3] = {
//     {0, 0, 2},
//     {1, 1, 0},
//     {0, 0, 0},
// };

int kernelWidth = sizeof kernel[0] / sizeof kernel[0][0];
int kernelHeight = sizeof kernel / sizeof kernel[0];

unsigned int pallete[] = {
    0x000000,
    0xFFFFFF,
    0x00FF00,
    0x0000FF,
    0xFF0000,
    0xFFFF00,
    0xFF8000,
};

int main()
{
    unsigned char *pixelsIn = NULL;
    unsigned int width, height;
    unsigned int err = loadbmp_decode_file("in3.bmp", &pixelsIn, &width, &height, LOADBMP_RGBA);

    printf("%d %d\n", width, height);

    if (err)
        return printf("LoadBMP Load Error: %u\n", err), 0;

    unsigned char *pixels = (unsigned char *)malloc(width * height * 4);

    int(*rBuf)[width] = malloc(sizeof(int[kernelHeight][width]));
    int(*gBuf)[width] = malloc(sizeof(int[kernelHeight][width]));
    int(*bBuf)[width] = malloc(sizeof(int[kernelHeight][width]));

    memset(rBuf, 0, sizeof(int[kernelHeight][width]));
    memset(gBuf, 0, sizeof(int[kernelHeight][width]));
    memset(bBuf, 0, sizeof(int[kernelHeight][width]));

    unsigned char *ptr = pixelsIn;
    unsigned char *ptrOut = pixels;
    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            int r = *(ptr++) + rBuf[i % kernelHeight][j];
            int g = *(ptr++) + gBuf[i % kernelHeight][j];
            int b = *(ptr++) + bBuf[i % kernelHeight][j];
            ptr++;

            rBuf[i % kernelHeight][j] = gBuf[i % kernelHeight][j] = bBuf[i % kernelHeight][j] = 0;

            r = MAX(0, MIN(255, r));
            g = MAX(0, MIN(255, g));
            b = MAX(0, MIN(255, b));

            int md = 1 << 30,
                mi = 0;
            for (int k = 0; k < (sizeof pallete / sizeof pallete[0]); ++k)
            {
                int rp = (int)(pallete[k] >> 16 & 0xFF);
                int gp = (int)(pallete[k] >> 8 & 0xFF);
                int bp = (int)(pallete[k] >> 0 & 0xFF);

                int d = (rp - r) * (rp - r) + (gp - g) * (gp - g) + (bp - b) * (bp - b);

                if (d < md)
                    md = d, mi = k;
            }

            int rErr = r - (int)(*ptrOut++ = pallete[mi] >> 16 & 0xFF);
            int gErr = g - (int)(*ptrOut++ = pallete[mi] >> 8 & 0xFF);
            int bErr = b - (int)(*ptrOut++ = pallete[mi] >> 0 & 0xFF);
            *ptrOut++ = 0xFF;

            for (int k = 0; k < kernelHeight; ++k)
            {
                for (int l = -kernelX; l < kernelWidth - kernelX; ++l)
                {
                    if (!(0 <= j + l && j + l < width))
                        continue;
                    rBuf[(i + k) % kernelHeight][j + l] += (kernel[k][l] * rErr) / coef;
                    gBuf[(i + k) % kernelHeight][j + l] += (kernel[k][l] * gErr) / coef;
                    bBuf[(i + k) % kernelHeight][j + l] += (kernel[k][l] * bErr) / coef;
                }
            }
        }
    }

    err = loadbmp_encode_file("out3.bmp", pixels, width, height, LOADBMP_RGBA);
    if (err)
        return printf("LoadBMP Load Error: %u\n", err), 0;

    free(rBuf);
    free(gBuf);
    free(bBuf);
    free(pixels);
    free(pixelsIn);
    return 0;
}