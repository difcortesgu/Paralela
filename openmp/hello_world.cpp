#include <iostream>
#include <omp.h>

int main(){
    #pragma omp parallel num_threads(4)
    {

        int ID = omp_get_thread_num();
        std::cout << ID << "Hello world" << std::endl;
    }
}