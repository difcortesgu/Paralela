//%cflags:-fopenmp -lm -D_DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define ITERATIONS 1e09

int calculatePi(double *piTotal){   
    int i = 0;
    do{
        *piTotal = *piTotal + (double)(4.0 / ((i*2)+1));
        i++;
        *piTotal = *piTotal - (double)(4.0 / ((i*2)+1));
        i++;
    }while(i < ITERATIONS);
    return 0;
}


int main()
{
    double pi;
    struct timeval tval_before, tval_after, tval_result;

    gettimeofday(&tval_before, NULL);
    pi = 0;
    calculatePi(&pi);
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);

    printf("Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
    printf("\npi: %2.12f \n", pi);
}