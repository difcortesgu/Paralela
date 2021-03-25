//%cflags:-fopenmp -lm -D_DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#define ITERATIONS 1e09

int calculatePi(double *piTotal, int initIteration, int endIteration)
{   int i = initIteration;
    
    do{
        *piTotal = *piTotal + (double)(4.0 / ((i*2)+1));
        i++;
        *piTotal = *piTotal - (double)(4.0 / ((i*2)+1));
        i++;
    }while(i < endIteration);

    return 0;
}


int main(){
    double pi, pi_hijo;
    int pipefd[2], r;
    pid_t pid;
    struct timeval tval_before, tval_after, tval_result;
    gettimeofday(&tval_before, NULL);
    r = pipe(pipefd);
    if(r < 0){
        perror("error pipe: ");
        exit(-1);
    }
    pi = 0;
    pid = fork();
    if(pid < 0){
        perror("Error en fork ");
        exit(-1);
    } 
    if(pid == 0){  //hijo
        close(pipefd[0]);
        calculatePi(&pi, 0, (ITERATIONS/2) );
        r = write(pipefd[1], &pi, sizeof(double));
        if(r <= 0){
            perror("error write: ");
            exit(-1);
        }        
        close(pipefd[1]);
        exit(0);
    }else{   
        close(pipefd[1]);
        calculatePi(&pi, (ITERATIONS/2), ITERATIONS);
        r = read(pipefd[0], &pi_hijo, sizeof(double));
        if(r <= 0){
            perror("error read: ");
            exit(-1);
        }
        close(pipefd[0]);
        pi = pi + pi_hijo;
    }
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);
    printf("Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
    
    printf("\npi: %2.12f \n", pi);
    return 0;
}
