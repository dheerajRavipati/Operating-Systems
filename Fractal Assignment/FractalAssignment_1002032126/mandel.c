#include "bitmap.h"

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

struct mandel_args{
    struct bitmap *bm;
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double max;
    int hmin;
    int hmax;
};

int iteration_to_color( int i, int max );
int iterations_at_point( double x, double y, int max );

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

void* compute_image(void* arg)
{
    int i,j;
    struct mandel_args* mandel_arg = (struct mandel_args*) arg;

    int width = bitmap_width(mandel_arg -> bm);
    int height = bitmap_height(mandel_arg -> bm);
    int hmin = mandel_arg -> hmin;
    int hmax = mandel_arg -> hmax;

    // For every pixel in the image...

    for(j=hmin; j<hmax; j++) {

        for(i=0; i<width; i++) {

            // Determine the point in x,y space for that pixel.
            double x = mandel_arg->xmin + i*(mandel_arg->xmax-mandel_arg->xmin)/width;
            double y = mandel_arg->ymin + j*(mandel_arg->ymax-mandel_arg->ymin)/height;

            // Compute the iterations at that point.
            int iters = iterations_at_point(x,y,mandel_arg -> max);

            // Set the pixel in the bitmap.
            bitmap_set(mandel_arg->bm,i,j,iters);
        }

    }

    return 0;
}


void show_help()
{
    printf("Use: mandel [options]\n");
    printf("Where options are:\n");
    printf("-m <max>     The maximum number of iterations per point. (default=1000)\n");
    printf("-x <coord>   X coordinate of image center point. (default=0)\n");
    printf("-y <coord>   Y coordinate of image center point. (default=0)\n");
    printf("-s <scale>   Scale of the image in Mandlebrot coordinates. (default=4)\n");
    printf("-W <pixels>  Width of the image in pixels. (default=500)\n");
    printf("-H <pixels>  Height of the image in pixels. (default=500)\n");
    printf("-o <file>    Set output file. (default=mandel.bmp)\n");
    printf("-h           Show this help text.\n");
    printf("-n <threads> Number of threads to run the program.(default=1)\n");
    printf("\nSome examples are:\n");
    printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
    printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
    printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}

int main( int argc, char *argv[] )
{
    char c;

    // These are the default configuration values used
    // if no command line arguments are given.

    const char *outfile = "mandel.bmp";
    double xcenter = 0;
    double ycenter = 0;
    double scale = 4;
    int    image_width = 500;
    int    image_height = 500;
    int    max = 1000;
    int    n = 1; //n represents number of threads.
                  //If n is not specified in the argument, it will default to 1.

    struct timeval begin_time;
    struct timeval end_time;

    gettimeofday( &begin_time, NULL );
    // For each command line argument given,
    // override the appropriate configuration value.

    while((c = getopt(argc,argv,"x:y:s:W:H:m:o:n:h"))!=-1) {
        switch(c) {
            case 'x':
                    xcenter = atof(optarg);
                    break;
            case 'y':
                    ycenter = atof(optarg);
                    break;
            case 's':
                    scale = atof(optarg);
                    break;
            case 'W':
                    image_width = atoi(optarg);
                    break;
            case 'H':
                    image_height = atoi(optarg);
                    break;
            case 'm':
                    max = atoi(optarg);
                    break;
            case 'o':
                    outfile = optarg;
                    break;
            case 'n':
                    n = atoi(optarg);
                    break;
            case 'h':
                    show_help();
                    exit(1);
                    break;
        }
    }

    // Display the configuration of the image.
    printf("mandel: x=%lf y=%lf scale=%lf max=%d outfile=%s numberofthreads=%d\n" ,
    xcenter,ycenter,scale,max,outfile,n);

    // Create a bitmap of the appropriate size.
    struct bitmap *bm = bitmap_create(image_width,image_height);

    // Fill it with a dark blue, for debugging
    bitmap_reset(bm,MAKE_RGBA(0,0,255,0));
    
    // Create n number of threads to divide the process to each thread
	pthread_t* tid = malloc(n * sizeof(pthread_t));

    // Calculate average height of each part that the image can be divided to
    int avg_height = image_height / n;

    // Loop to assign each thread to compute part of the Mandelbrot image.
	int i;
    struct mandel_args* mandel_arg = malloc(n * sizeof(struct mandel_args));

	for(i=0; i<n; i++) {
		mandel_arg[i].bm = bm;
		mandel_arg[i].xmin = xcenter-scale;
		mandel_arg[i].xmax = xcenter+scale;
		mandel_arg[i].ymin = ycenter-scale;
		mandel_arg[i].ymax = ycenter+scale;
		mandel_arg[i].max = max;
		if(i == 0){
			mandel_arg[i].hmin = 0;
			mandel_arg[i].hmax = avg_height;
		}
        else{
			mandel_arg[i].hmin = mandel_arg[i-1].hmax;
			mandel_arg[i].hmax= mandel_arg[i-1].hmax + avg_height;			
		}
        
		pthread_create(&tid[i], NULL, compute_image, (void*) &mandel_arg[i]);

	}

	for(i = 0; i < n; ++i) {
		pthread_join(tid[i], NULL);
	}

    // Save the image in the stated file.
    if(!bitmap_save(mandel_arg->bm,outfile)) {
        fprintf(stderr,"mandel: couldn't write to %s: %s\n",outfile,strerror(errno));
        return 1;
    }

    gettimeofday( &end_time, NULL );

    //Calculate time taken by all threads to render the image.
    long time_to_execute = ( end_time.tv_sec * 1000000 + end_time.tv_sec ) - ( begin_time.tv_sec * 1000000 + begin_time.tv_usec );
    printf("This code took %d microseconds to execute\n", time_to_execute);

    return 0;
}

/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
    double x0 = x;
    double y0 = y;

    int iter = 0;

    while( (x*x + y*y <= 4) && iter < max ) {
        double xt = x*x - y*y + x0;
        double yt = 2*x*y + y0;

        x = xt;
        y = yt;

        iter++;
    }

    return iteration_to_color(iter,max);
}

/*
Convert a iteration number to an RGBA color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/

int iteration_to_color( int i, int max )
{
    int gray = 255*i/max;
    return MAKE_RGBA(gray,gray,gray,0);
}
