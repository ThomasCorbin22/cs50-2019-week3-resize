// Copies a BMP file

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bmp.h"

void iterateWidth(int INbiWidth, int INpadding, int OUTpadding, FILE *inptr, FILE *outptr, float val);

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./resize f infile outfile\n");
        return 1;
    }

    // get float from input
    float val;

    val = atof(argv[1]);

    // remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];

    // open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    printf("INBiSizeImage: %i\n", bi.biSizeImage);
    printf("INBfSize: %i\n", bf.bfSize);

    // Define old and new BITMAPINFOHEADER width and height
    int INbiWidth = abs(bi.biWidth);
    printf("INwidth: %i\n", INbiWidth);
    int INbiHeight = abs(bi.biWidth);
    printf("Inheight: %i\n", INbiHeight);
    int OUTbiWidth = fabs(bi.biWidth * val);
    printf("Outwidth: %i\n", OUTbiWidth);
    int OUTbiHeight = fabs(bi.biWidth * val);
    printf("Outheight: %i\n", OUTbiHeight);

    // write outfile's new width and height
    bi.biWidth = OUTbiWidth;
    bi.biHeight = OUTbiHeight * -1;

    // determine padding for scanlines
    int INpadding = (4 - (INbiWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    printf("INpadding: %i\n", INpadding);
    int OUTpadding = (4 - (OUTbiWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    printf("Outpadding: %i\n", OUTpadding);

    // determine seek amount for scanlines
    int SEEKAmount = (INbiWidth * sizeof(RGBTRIPLE) + INpadding) * -1;

    // change BITMAPINFOHEADER size
    bi.biSizeImage = ((sizeof(RGBTRIPLE) * OUTbiWidth) + OUTpadding) * OUTbiHeight;
    printf("OUTBiSizeImage: %i\n", bi.biSizeImage);

    // change BITMAPFILEHEADER size
    bf.bfSize = bi.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    printf("OUTBfSize: %i\n", bf.bfSize);

    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);
    if (val < 1)
    {
        float j = val;

        // iterate over infile's scanlines
        for (int i = 0; i < INbiHeight; i++)
        {
            if (j >= 1)
            {
                iterateWidth(INbiWidth, INpadding, OUTpadding, inptr, outptr, val);

                j = j - 1 + val;

            }
            else
            {
                fseek(inptr, (SEEKAmount * - 1), SEEK_CUR);

                j = j + val;

            }
        }
    }

    else
    {
        // iterate over infile's scanlines
        for (int i = 0; i < INbiHeight; i++)
        {
            float k = val - floor(val);

            // iterate over outfile's scanlines
            for (int j = 0; j < floor(val); j++)
            {
                // iterate over pixels in scanline
                iterateWidth(INbiWidth, INpadding, OUTpadding, inptr, outptr, val);

                if (k >= 1)
                {
                    fseek(inptr, SEEKAmount, SEEK_CUR);
                    iterateWidth(INbiWidth, INpadding, OUTpadding, inptr, outptr, val);

                    k = k - 1 + (val - floor(val));
                }

                else
                {
                    k = k + (val - floor(val));
                }

                // resets the pointer up until final iteration
                if (j < floor(val) - 1)
                {
                    fseek(inptr, SEEKAmount, SEEK_CUR);
                }
            }
        }
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}

// iterate over pixels in the scanline
void iterateWidth(int INbiWidth, int INpadding, int OUTpadding, FILE *inptr, FILE *outptr, float val)

{
    if (val < 1)
    {
        float j = val;

        for (int i = 0; i < INbiWidth; i++)
        {
            // temporary storage
            RGBTRIPLE triple;
            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

            if (j >= 1)
            {
                fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);

                j = j - 1 + val;
            }
            else
            {
                j = j + val;
            }

        }

    }
    else
    {
        float k = val - floor(val);

        for (int i = 0; i < INbiWidth; i++)
        {
            // temporary storage
            RGBTRIPLE triple;

            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

            // write RGB triple to outfile
            for (int j = 0; j < val; j++)
            {
                fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
            }
            if (k >= 1)
            {
                fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);

                k = k - 1 + (val - floor(val));
            }
            else
            {
                k = k + (val - floor(val));
            }
        }
    }

    // skip over padding, if any
    fseek(inptr, INpadding, SEEK_CUR);

    // then add padding back (to demonstrate how)
    for (int i = 0; i < OUTpadding; i++)
    {
        fputc(0x00, outptr);
    }
}