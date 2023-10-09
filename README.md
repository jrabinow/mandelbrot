# mandelbrot
http://pdbuchan.com/mandelbrot/mandelbrot.html made parallel using POSIX threads

P David Buchan implemented a program for generating mandelbrot fractal bitmaps in 2002. I found the code and sped it up by making it parallel. 
 
The code generates an image in BMP format, output.bmp by default. This is an uncompressed image format so it's quite large - 88MB for a 5000x5000 image.

## Building:
```
cd ./build && make
```

## Usage:
```
./mp --help
Usage: ./mp [OPTION]...
        --help  display this help message
        -h, --hp ARG    horizontal pixels - int
        -v, --vp ARG    vertical pixels - int
        -r, --ri LOW:HIGH       low/high interval for real numbers - float:float
        -c, --ci LOW:HIGH       low/high interval for complex numbers - float:float
        -i, --iter      number of iterations - int
        -o, --output    filepath to save image to - file path
        -t              number of threads to use
```

```
mp --hp 5000 --vp 5000 --ri -2:0.5 --ci -1.25:1.25 --iter 3000 -o output.bmp --palette ./tests/palette
```
Or if you prefer interactive input, the program will prompt for any value that's not passed in as a flag:
```
$ mp --palette ./tests/palette
What is the horizontal dimension(px) ?
5000
What is the vertical dimension(px) ?
5000
What is the lowest real value ?
-2
What is the highest real value ?
0.5
What is the lowest imaginary value ?
-1.25
What is the highest imaginary value ?
1.25

What is the maximum allowable number of iterations ?
3000
```

Try the sample input:
```
mp --palette palette < input
```
