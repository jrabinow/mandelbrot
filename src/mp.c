/*  Copyright(C) 2012  P.D. Buchan(pdbuchan@yahoo.com)

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

#include <float.h>
#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __APPLE__

#include <libkern/OSByteOrder.h>

#define htobe16(x) OSSwapHostToBigInt16(x)
#define htole16(x) OSSwapHostToLittleInt16(x)
#define be16toh(x) OSSwapBigToHostInt16(x)
#define le16toh(x) OSSwapLittleToHostInt16(x)

#define htobe32(x) OSSwapHostToBigInt32(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)

#define htobe64(x) OSSwapHostToBigInt64(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)

#endif

typedef struct {
  unsigned char *framebuffer;
  double rlo, ilo, stepu, stepv;
  unsigned int xres;
  int *c, maxit, ncolor;
  unsigned int *tr, *tg, *tb;
} Global_var;

typedef struct {
  Global_var gv;
  int ymin, ymax;
  int id, j;
  pthread_t th;
} Thread_arg;

void write_BMP_header(FILE *stream, unsigned int filesize, unsigned int xres,
                      unsigned int yres);

void *threaded_mp(void *arg) {
  Thread_arg *ta = (Thread_arg *)arg;
  int *c = ta->gv.c, ncolor = ta->gv.ncolor, maxit = ta->gv.maxit, i, j, index;
  unsigned int x, y, xres = ta->gv.xres, ymin = ta->ymin, ymax = ta->ymax, r, g,
                     b, *tr = ta->gv.tr, *tg = ta->gv.tg, *tb = ta->gv.tb;
  double u, v, rlo = ta->gv.rlo, r1, i1, r2, i2, stepu = ta->gv.stepu,
               stepv = ta->gv.stepv;
  unsigned char *framebuffer = ta->gv.framebuffer;

  v = ta->gv.ilo + ta->id * stepv * (ymax - ymin);
  i = ymin * xres;
  j = maxit;

  for (y = ymin; y < ymax; y++) {
    u = rlo;
    for (x = 0; x < xres; x++) {
      r1 = u;
      i1 = v;

      // Iterate until either maxit is reached, or abs value > 2.0.
      // c array counts iterations.
      c[i] = 0;
      r2 = 0.0;
      i2 = 0.0;
      while (r2 * r2 + i2 * i2 < 4.0 && c[i] <= maxit) {
        r2 = r1 * r1 - i1 * i1 + u;
        i2 = 2.0 * i1 * r1 + v;
        c[i]++;
        r1 = r2;
        i1 = i2;
      }
      u += stepu;
      // Find minimum number of iterations taken.
      // in order to scale color range.
      j = c[i] < j ? c[i] : j;
      i++;
    }
    v += stepv;
  }

  ta->j = j;
  // Assign a color to each pixel.
  i = ymin * xres;
  j = xres * 3 * ymin;
  for (y = ymin; y < ymax; y++) {
    for (x = 0; x < xres; x++) {
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

      // Write pixel to file(RGB).
      framebuffer[j++] = (unsigned char)b;
      framebuffer[j++] = (unsigned char)g;
      framebuffer[j++] = (unsigned char)r;
    }
  }
  pthread_exit(NULL);
}

void usage(const char *progname, FILE *stream) {
  fprintf(
      stream,
      "Usage: %s [OPTION]...\n"
      "\t--help\tdisplay this help message\n"
      "\t-h, --hp ARG\thorizontal pixels - int\n"
      "\t-v, --vp ARG\tvertical pixels - int\n"
      "\t-r, --ri LOW:HIGH\tlow/high interval for real numbers - float:float\n"
      "\t-c, --ci LOW:HIGH\tlow/high interval for complex numbers - "
      "float:float\n"
      "\t-i, --iter\tnumber of iterations - int\n"
      "\t-o, --output\tfilepath to save image to - file path\n"
      "\t-t\t\tnumber of threads to use\n",
      progname);
}

typedef struct {
  unsigned xres, yres, maxit;
  float rlo, rhi, ilo, ihi;
  char *output;
  char *palette;
  int optind;
  unsigned threads;
} ParsedArgs;

ParsedArgs parse_args(int argc, char *argv[]) {
  ParsedArgs parsed_args = {0,
                            0,
                            0,
                            FLT_MAX,
                            FLT_MAX,
                            FLT_MAX,
                            FLT_MAX,
                            "output.bmp",
                            "palette",
                            0,
                            sysconf(_SC_NPROCESSORS_ONLN)};
  int c, option_index = 0;
  char *endptr;

  static struct option longopts[] = {
      {"help", no_argument, NULL, 'g'}, // we use 'g' because 'h' already taken
      {"hp", required_argument, NULL, 'h'},
      {"vp", required_argument, NULL, 'v'},
      {"ri", required_argument, NULL, 'r'},
      {"ci", required_argument, NULL, 'c'},
      {"iter", required_argument, NULL, 'i'},
      {"output", required_argument, NULL, 'o'},
      {"palette", required_argument, NULL, 'p'},
      {"threads", required_argument, NULL, 't'},
      {NULL, 0, NULL, 0}};

  /* optstring is a string containing the legitimate option characters. If
   * such a character is followed by a colon, the option requires an argument,
   * Two colons mean an option takes an optional arg */
  while ((c = getopt_long(argc, argv, "h:v:r:c:i:n:o:p:t:", longopts,
                          &option_index)) != -1) {
    switch (c) {
    case 'g':
      usage(argv[0], stdout);
      exit(EXIT_SUCCESS);
    case 'h':
      parsed_args.xres = strtol(optarg, &endptr, 0);
      if (*endptr != '\0') {
        usage(argv[0], stderr);
        exit(EXIT_FAILURE);
      }
      break;
    case 'v':
      parsed_args.yres = strtol(optarg, &endptr, 0);
      if (*endptr != '\0') {
        usage(argv[0], stderr);
        exit(EXIT_FAILURE);
      }
      break;
    case 'r':
      parsed_args.rlo = strtof(optarg, &endptr);
      if (*endptr != ':') {
        usage(argv[0], stderr);
        exit(EXIT_FAILURE);
      }
      optarg = endptr + 1;
      parsed_args.rhi = strtof(optarg, &endptr);
      if (*endptr != '\0') {
        usage(argv[0], stderr);
        exit(EXIT_FAILURE);
      }
      break;
    case 'c':
      parsed_args.ilo = strtof(optarg, &endptr);
      if (*endptr != ':') {
        usage(argv[0], stderr);
        exit(EXIT_FAILURE);
      }
      optarg = endptr + 1;
      parsed_args.ihi = strtof(optarg, &endptr);
      if (*endptr != '\0') {
        usage(argv[0], stderr);
        exit(EXIT_FAILURE);
      }
      break;
    case 'i':
      parsed_args.maxit = strtol(optarg, &endptr, 0);
      if (*endptr != '\0') {
        usage(argv[0], stderr);
        exit(EXIT_FAILURE);
      }
      break;
    case 't':
      parsed_args.threads = strtol(optarg, &endptr, 0);
      if (*endptr != '\0') {
        usage(argv[0], stderr);
        exit(EXIT_FAILURE);
      }
      break;
    case 'o':
      parsed_args.output = optarg;
      break;
    case 'p':
      parsed_args.palette = optarg;
      break;
    case ':':
      fputs("missing argument", stderr);
    default:
      usage(argv[0], stderr);
      exit(EXIT_FAILURE);
    }
  }
  parsed_args.optind = optind;

  return parsed_args;
}

int main(int argc, char *argv[]) {
  int i, index, ncolor, *c, n, maxit, pad;
  unsigned int xres, yres;
  unsigned int *tr, *tg, *tb, buf, filesize;
  double rlo, rhi, ilo, ihi, stepu, stepv;
  FILE *fp, *fo;
  unsigned char *framebuffer;
  Thread_arg *ta;
  Global_var gv;
  ParsedArgs args = parse_args(argc, argv);

  // Determine how many colors in color palette.
  if ((fp = fopen(args.palette, "r")) == NULL) {
    perror("Error opening palette file");
    exit(EXIT_FAILURE);
  }
  ncolor = 0;
  while ((n = fgetc(fp)) != EOF)
    if (n == '\n')
      ncolor++;
  rewind(fp);

  // Allocate memory for color palette.
  if ((tr = (unsigned int *)malloc(ncolor * sizeof(unsigned int))) == NULL) {
    perror("Error allocating memory");
    exit(EXIT_FAILURE);
  }
  if ((tg = (unsigned int *)malloc(ncolor * sizeof(unsigned int))) == NULL) {
    perror("Error allocating memory");
    exit(EXIT_FAILURE);
  }
  if ((tb = (unsigned int *)malloc(ncolor * sizeof(unsigned int))) == NULL) {
    perror("Error allocating memory");
    exit(EXIT_FAILURE);
  }
  memset(tr, 0, ncolor * sizeof(unsigned int));
  memset(tg, 0, ncolor * sizeof(unsigned int));
  memset(tb, 0, ncolor * sizeof(unsigned int));

  // Read in color palette.
  if ((fp = fopen(args.palette, "r")) == NULL) {
    perror("Error opening palette file");
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < ncolor; i++)
    fscanf(fp, "%*u %u %u %u", &tr[i], &tg[i], &tb[i]);
  fclose(fp);

  // Ask for image dimensions(px).
  if (args.xres == 0) {
    do {
      printf("What is the horizontal dimension(px) ?\n");
    } while (scanf("%u", &xres) == 0);
  } else {
    xres = args.xres;
  }
  if (args.yres == 0) {
    do {
      printf("What is the vertical dimension(px) ?\n");
    } while (scanf("%u", &yres) == 0);
  } else {
    yres = args.yres;
  }
  framebuffer =
      (unsigned char *)malloc(3 * xres * yres * sizeof(unsigned char));

  // Allocate memory for the array containing iterations.
  c = (int *)malloc(xres * yres * sizeof(int));
  memset(c, 0, xres * yres * sizeof(int));

  // Ask for range on real scale
  if (args.rlo == FLT_MIN || args.rlo == FLT_MAX || args.rlo != args.rlo) {
    do {
      printf("What is the lowest real value ?\n");
    } while (scanf("%lf", &rlo) == 0);
  } else {
    rlo = args.rlo;
  }
  if (args.rhi == FLT_MIN || args.rhi == FLT_MAX || args.rhi != args.rhi) {
    do {
      printf("What is the highest real value ?\n");
    } while (scanf("%lf", &rhi) == 0);
  } else {
    rhi = args.rhi;
  }

  // Ask for range on imaginary scale.
  if (args.ilo == FLT_MIN || args.ilo == FLT_MAX || args.ilo != args.ilo) {
    do {
      printf("What is the lowest imaginary value ?\n");
    } while (scanf("%lf", &ilo) == 0);
  } else {
    ilo = args.ilo;
  }
  if (args.ihi == FLT_MIN || args.ihi == FLT_MAX || args.ihi != args.ihi) {
    do {
      printf("What is the highest imaginary value ?\n");
    } while (scanf("%lf", &ihi) == 0);
  } else {
    ihi = args.ihi;
  }

  // Ask for maximum allowable number of iterations.
  if (args.maxit == 0) {
    do {
      printf("\nWhat is the maximum allowable number of iterations ?\n");
    } while (scanf("%i", &maxit) == 0);
  } else {
    maxit = args.maxit;
  }

  // Open output file.
  fo = fopen(args.output, "wb");
  if (fo == NULL) {
    printf("Can't open new bitmap file.\n");
    exit(EXIT_FAILURE);
  }

  // Determine step-sizes in(u,v) plane.
  stepu = (rhi - rlo) / xres;
  stepv = (ihi - ilo) / yres;

  // File is for Windows O/S.

  // File size. Pad to nearest 4-bytes. Add header length.
  filesize = (xres * yres * 3u) + 54u;
  pad = filesize % 4;
  filesize += pad;

  write_BMP_header(fo, filesize, xres, yres);

  // Plot selected area, iterating on each point.
  // Pixels are in(x,y) plane.

  /* start threaded mandlebrot construction */
  index = args.threads;
  ta = (Thread_arg *)alloca(sizeof(Thread_arg) * index);
  gv.framebuffer = framebuffer;
  gv.rlo = rlo;
  gv.ilo = ilo;
  gv.stepu = stepu;
  gv.stepv = stepv;
  gv.xres = xres;
  gv.c = c;
  gv.maxit = maxit;
  gv.ncolor = ncolor;
  gv.tr = tr;
  gv.tg = tg;
  gv.tb = tb;

  for (i = 0; i < index; i++) {
    ta[i].gv = gv;
    ta[i].ymin = i * (yres / index);
    ta[i].ymax = (i + 1) * (yres / index);
    ta[i].id = i;
  }
  ta[index - 1].ymax = yres;

  for (i = 0; i < index; i++) {
    if (pthread_create(&ta[i].th, NULL, &threaded_mp, (void *)&ta[i]) != 0) {
      perror("Error launching thread");
      exit(EXIT_FAILURE);
    }
  }

  for (i = 0; i < index; i++) {
    if (pthread_join(ta[i].th, NULL) != 0) {
      perror("Error joining thread");
      exit(EXIT_FAILURE);
    }
  }

  fwrite(framebuffer, sizeof(unsigned char), 3 * xres * yres, fo);

  // Add file padding to reach 4-byte boundary.
  buf = 0;
  for (i = 0; i < pad; i++) {
    fputc(buf, fo);
  }

  // Close file descriptor.
  fclose(fo);

  // Free allocated memory.
  free(framebuffer);
  free(tr);
  free(tg);
  free(tb);
  free(c);

  return (EXIT_SUCCESS);
}

void write_BMP_header(FILE *stream, unsigned int filesize, unsigned int xres,
                      unsigned int yres) {
  uint32_t val;

  fwrite("BM", 1, 2, stream);

  // file size (bytes)
  val = htole32(filesize);
  fwrite((unsigned char *)&val, 1, 4, stream);

  // Reserved for future use.
  fwrite("\0\0\0\0", 1, 4, stream);

  // Offset to BMP data.
  fputc(54, stream);
  fwrite("\0\0\0", 1, 3, stream);

  // Header is Windows O/S.
  fputc(40, stream);
  fwrite("\0\0\0", 1, 3, stream);

  // Image width(px).
  val = htole32(xres);
  fwrite((unsigned char *)&val, 1, 4, stream);

  // Image height(px).
  val = htole32(yres);
  fwrite((unsigned char *)&val, 1, 4, stream);

  // Number of planes.
  fputc(1, stream);
  fputc('\0', stream);

  // Bit depth of image.
  fputc(24, stream);
  fputc('\0', stream);

  // No compression.
  fwrite("\0\0\0\0", 1, 4, stream);

  // BMP data size.
  // file size (bytes)
  val = htole32(filesize - 54); //((54 << 24) | (54 << 16) | (54 << 8) | 54));
  fwrite((unsigned char *)&val, 1, 4, stream);

  // Horizontal resolution 72 dpi.
  fputc(18, stream);
  fputc(11, stream);
  fwrite("\0\0", 1, 2, stream);

  // Vertical resolution 72 dpi.
  fputc(18, stream);
  fputc(11, stream);
  fwrite("\0\0", 1, 2, stream);

  // Use maximum number of colors.
  fwrite("\0\0\0\0", 1, 4, stream);

  // All colors are important.
  fwrite("\0\0\0\0", 1, 4, stream);
}
