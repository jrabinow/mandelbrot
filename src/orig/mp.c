/*  Copyright (C) 2012  P.D. Buchan (pdbuchan@yahoo.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */

// Mandelbrot Generator

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char** argv)
{
	int i, j, index, ncolor, *c;
	unsigned int n, dum, x, y, xres, yres, maxit, pad;
	unsigned int *tr, *tg, *tb, buf, r, g, b, filesize;
	double rlo, rhi, ilo, ihi, u, v, stepu, stepv;
	double r1, r2, i1, i2;
	FILE *fp, *fo;

	// Determine how many colors in color palette.
	fp = fopen ("palette", "r");
	if (fp == NULL) {
		printf ("Can't open palette file.\n");
		exit (EXIT_FAILURE);
	}
	i = 0;
	while ((n = fgetc (fp)) != EOF) {
		if (n == '\n') {
			i++;
		}
	}
	fclose (fp);
	ncolor = i;

	// Allocate memory for color palette.
	tr = (unsigned int *) malloc (ncolor * sizeof (unsigned int));
	memset (tr, 0, ncolor * sizeof (unsigned int));
	tg = (unsigned int *) malloc (ncolor * sizeof (unsigned int));
	memset (tg, 0, ncolor * sizeof (unsigned int));
	tb = (unsigned int *) malloc (ncolor * sizeof (unsigned int));
	memset (tb, 0, ncolor * sizeof (unsigned int));

	// Read in color palette.
	fp = fopen ("palette", "r");
	for (i=0; i<=ncolor-1; i++) {
		fscanf (fp, "%i %i %i %i", &dum, &tr[i], &tg[i], &tb[i]);
	}
	fclose (fp);

	// Ask for image dimensions (px).
	printf ("What is the horizontal dimension (px) ?\n");
	scanf ("%u", &xres);
	printf ("What is the vertical dimension (px) ?\n");
	scanf ("%u", &yres);

	// Allocate memory for the array containing iterations.
	c = (int *) malloc (xres * yres * sizeof (int));
	memset (c, 0, xres * yres * sizeof (int));

	// Ask for range on real scale
	printf ("What is the lowest real value ?\n");
	scanf ("%lf", &rlo);
	printf ("What is the highest real value ?\n");
	scanf ("%lf", &rhi);

	// Ask for range on imaginary scale.
	printf ("What is the lowest imaginary value ?\n");
	scanf ("%lf", &ilo);
	printf ("What is the highest imaginary value ?\n");
	scanf ("%lf", &ihi);

	// Ask for maximum allowable number of iterations.
	printf ("What is the maximum allowable number of iterations ?\n");
	scanf ("%i", &maxit);

	// Open output file.
	fo = fopen ("output.bmp", "wb");
	if (fo == NULL) {
		printf ("Can't open new bitmap file.\n");
		exit (EXIT_FAILURE);
	}

	// Determine step-sizes in (u,v) plane.
	stepu = (rhi - rlo) / xres;
	stepv = (ihi - ilo) / yres;

	// File is for Windows O/S.
	buf = 66u;
	fputc (buf, fo);
	buf = 77u;
	fputc (buf, fo);

	// File size. Pad to nearest 4-bytes. Add header length.
	filesize = (xres * yres * 3u) + 54u;
	pad = 0;
	if ((filesize % 4u) > 0) {
		filesize++;
		pad++;
	}
	if ((filesize % 4u) > 0) {
		filesize++;
		pad++;
	}
	if ((filesize % 4u) > 0) {
		filesize++;
		pad++;
	}
	fputc (filesize & 255u, fo);
	fputc ((filesize >> 8u) & 255u, fo);
	fputc ((filesize >> 16u) & 255u, fo);
	fputc ((filesize >> 24u) & 255u, fo);

	// Reserved for future use.
	buf = 0;
	fputc (buf, fo);
	fputc (buf, fo);
	fputc (buf, fo);
	fputc (buf, fo);

	// Offset to BMP data.
	buf = 54u;
	fputc (buf, fo);
	buf = 0;
	fputc (buf, fo);
	fputc (buf, fo);
	fputc (buf, fo);

	// Header is for Windows O/S.
	buf = 40u;
	fputc (buf, fo);
	buf = 0;
	fputc (buf, fo);
	fputc (buf, fo);
	fputc (buf, fo);

	// Image width (px).
	fputc (xres & 255u, fo);
	fputc ((xres >> 8u) & 255u, fo);
	fputc ((xres >> 16u) & 255u, fo);
	fputc ((xres >> 24u) & 255u, fo);

	// Image height (px).
	fputc (yres & 255u, fo);
	fputc ((yres >> 8u) & 255u, fo);
	fputc ((yres >> 16u) & 255u, fo);
	fputc ((yres >> 24u) & 255u, fo);

	// Number of planes.
	buf = 1u;
	fputc (buf, fo);
	buf = 0;
	fputc (buf, fo);

	// Bit depth of image.
	buf = 24u;
	fputc (buf, fo);
	buf = 0;
	fputc (buf, fo);

	// No compression.
	buf = 0;
	fputc (buf, fo);
	fputc (buf, fo);
	fputc (buf, fo);
	fputc (buf, fo);

	// BMP data size.
	fputc ((filesize - 54u) & 255u, fo);
	fputc (((filesize - 54u) >> 8u) & 255u, fo);
	fputc (((filesize - 54u) >> 16u) & 255u, fo);
	fputc (((filesize - 54u) >> 24u) & 255u, fo);

	// Horizontal resolution 72 dpi.
	buf = 18u;
	fputc (buf, fo);
	buf = 11u;
	fputc (buf, fo);
	buf = 0;
	fputc (buf, fo);
	fputc (buf, fo);

	// Vertical resolution 72 dpi.
	buf = 18u;
	fputc (buf, fo);
	buf = 11u;
	fputc (buf, fo);
	buf = 0;
	fputc (buf, fo);
	fputc (buf, fo);

	// Use maximum number of colors.
	buf = 0;
	fputc (buf, fo);
	fputc (buf, fo);
	fputc (buf, fo);
	fputc (buf, fo);

	// All colors are important.
	buf = 0;
	fputc (buf, fo);
	fputc (buf, fo);
	fputc (buf, fo);
	fputc (buf, fo);

	// Plot selected area, iterating on each point.
	// Pixels are in (x,y) plane.
	i = 0;
	v = ilo;
	printf("y = %4d\tv = %f\n", 1, v);
	for (y=1; y<=yres; y++) {
		u = rlo;
		for (x=1; x<=xres; x++) {
			r1 = u;
			i1 = v;

			// Iterate until either: maxit is reached, or abs value > 2.0).
			// Variable c counts iterations.
			c[i] = 0;
			r2 = 0.0;
			i2 = 0.0;
			while ((((r2*r2)+(i2*i2)) < 4.0) && (c[i] <= maxit)) {
				r2 = (r1 * r1) - (i1 * i1) + u;
				i2 = (2.0 * i1 * r1) + v;
				c[i]++;
				r1 = r2;
				i1 = i2;
			}
			i++;
			u += stepu;
		}
		v += stepv;
	}

	// Find minimum number of iterations taken.
	// in order to scale color range.
	i = 0;
	j = maxit;
	for (y=1; y<=yres; y++) {
		for (x=1; x<=xres; x++) {
			if (c[i] < j) {
				j = c[i];
			}
			i++;
		}
	}

	// Assign a color to each pixel.
	i = 0;
	for (y=1; y<=yres; y++) {
		for (x=1; x<=xres; x++) {
			if (c[i] > maxit) {  
				r = 0;
				g = 0;
				b = 0;
			} else {
				index = c[i] % ncolor;
				r = tr[index];
				g = tg[index];
				b = tb[index];
			}
			i++;

			// Write pixel to file (RGB).
			buf = b;
			fputc (buf, fo);
			buf = g;
			fputc (buf, fo);
			buf = r;
			fputc (buf, fo);
		}
	}

	// Add file padding to reach 4-byte boundary.
	buf = 0;
	for (i=0; i<pad; i++) {
		fputc (buf, fo);
	}

	// Close file descriptor.
	fclose (fo);

	// Free allocated memory.
	free (tr);
	free (tg);
	free (tb);
	free (c);

	return (EXIT_SUCCESS);
}
