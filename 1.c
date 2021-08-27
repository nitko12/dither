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
// int coef = 8;
// int kernelX = 1;
// unsigned char kernel[3][4] = {
//     {0, 0, 1, 1},
//     {1, 1, 1, 0},
//     {0, 1, 0, 0},
// };

// Burkes
// int coef = 32;
// int kernelX = 2;
// unsigned char kernel[3][5] = {
//     {0, 0, 0, 8, 4},
//     {2, 4, 8, 4, 2},
//     {0, 0, 0, 0, 0},
// };

// Stucki
int coef = 42;
int kernelX = 2;
unsigned char kernel[3][5] = {
    {0, 0, 0, 8, 4},
    {2, 4, 8, 4, 2},
    {1, 2, 4, 2, 1},
};

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

unsigned int pallete[10][10] = {
    {
        0xFF0000,
        0xef3434,
        0xe56363,
        0xe08d8d,
        0xe0b1b1,
        0xe5d0d0,
        0xefeaea,
        0xFFFFFF,
    },
    {
        0x00FF00,
        0x3aaa0d,
        0x83cb22,
        0xbed053,
        0xd0c68a,
        0xd8cbb9,
        0xe8e2e0,
        0xFFFFFF,
    },
    {
        0x0000FF,
        0x349fef,
        0x63e5d2,
        0x8de0a4,
        0xbee0b1,
        0xe2e5d0,
        0xefedea,
        0xFFFFFF,
    },
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
    printf("uint8_t img_buf = {");
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

            for (int k = 0; k < 8; ++k)
            {
                int rp = (int)(pallete[(i % 3 + j) % 3][k] >> 16 & 0xFF);
                int gp = (int)(pallete[(i % 3 + j) % 3][k] >> 8 & 0xFF);
                int bp = (int)(pallete[(i % 3 + j) % 3][k] >> 0 & 0xFF);

                int d = (rp - r) * (rp - r) + (gp - g) * (gp - g) + (bp - b) * (bp - b);

                if (d < md)
                    md = d, mi = k;
            }

            int rErr = r - (int)(*ptrOut++ = pallete[(i % 3 + j) % 3][mi] >> 16 & 0xFF);
            int gErr = g - (int)(*ptrOut++ = pallete[(i % 3 + j) % 3][mi] >> 8 & 0xFF);
            int bErr = b - (int)(*ptrOut++ = pallete[(i % 3 + j) % 3][mi] >> 0 & 0xFF);
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

            printf("%d,", 7 - mi);
        }
        printf("\n");
    }
    printf("};\n");

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