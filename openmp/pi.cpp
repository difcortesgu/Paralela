#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>
 
#define ITERATIONS 2e09
#define THREADS 50

int calculatePi(double *piTotal, int ID)
{   int start, end;   
    start = (ITERATIONS/omp_get_num_threads()) * ID;
    end = (ITERATIONS/omp_get_num_threads()) * (ID + 1);
    int i = start;
 
    do{
        *(piTotal + ID) = *(piTotal + ID) + (double)(4.0 / ((i*2)+1));
        i++;
        *(piTotal + ID) = *(piTotal + ID) - (double)(4.0 / ((i*2)+1));
        i++;
    }while(i < end);
    return 0;
}
 
 
int main()
{
    int i, threads = THREADS;
    double pi[threads];
    struct timeval tval_before, tval_after, tval_result;
 
    
        
    for (int nt = 1; nt < THREADS; nt++)
    {
        for(i = 0; i < THREADS; i++)
            pi[i] = 0;

        gettimeofday(&tval_before, NULL);

        #pragma omp parallel num_threads(nt)
        {
            int ID = omp_get_thread_num();
            calculatePi(pi, ID);
        }
        for(i = 1; i < THREADS; i++){
            pi[0] = pi[0] + pi[i];
        }
        gettimeofday(&tval_after, NULL);
    
        timersub(&tval_after, &tval_before, &tval_result);
    
        printf("Number of threads: %d\n", nt);
        printf("Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
        
        printf("\npi: %2.10f   \n\n", pi[0]);
    }
    
    
}