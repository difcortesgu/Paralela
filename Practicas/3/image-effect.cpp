#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <sys/time.h>
#include <string>
#include <cuda_runtime.h>
#include <unistd.h>
#include <stdio.h>

using namespace cv;
using namespace std;

Mat image;
vector<vector<int>> kernel {
        {0, 1, 0},
        {1, -4, 1},
        {0, 1, 0}
    };
double kernel_total = 0;
    

_global_ void filterImage(const unsigned char image, unsigned char *result_image, const unsigned char *kernel, float *kernel_total, int kernel_size, int *image_width, int *image_height){
    /*
        On each block run a row of the image or a fraction of it if it's too big (>4k?)
        each thread brings 3 items to the shared memory

        ->  * * *  * * *  * * *  * * *  * * * ... * * * * * * * * *  
        
        
        ->  * * *  * * *  * * * * * * * * * ... * * * * * * * * * 
        ->  * * *  * * *  * * * * * * * * * ... * * * * * * * * * 
            * * * * * * * * * * * * * * * ... * * * * * * * * * 
            * * * * * * * * * * * * * * * ... * * * * * * * * * 
            * * * * * * * * * * * * * * * ... * * * * * * * * * 
            * * * * * * * * * * * * * * * ... * * * * * * * * * 
        
        then sync all threads in the block
        run a multiplication of the kernel with the image section on each thread
    */
   // blockIdx.x 
   // threadId.x

    // Copy image to sharedMemory
    extern _shared_ unsigned char rows[];

    for (int i = 0; i < *kernel_size; i++) {
        for (int j = 0; j < 3; j++) {
            rows[i * *image_width * 3 + threadIdx.x * 3 + j] = image[((blockIdx.x + i) * *image_width * 3) + threadIdx.x];
            
        }
    }
    __syncthreads();

    int newPixelR = 0;
    int newPixelG = 0;
    int newPixelB = 0;

    // Loop kernel rows
    for (int ki = 0; ki < *image_height - (*kernel_size -1); ki++)
    {
        // Loop kernel cols
        for (int kj = 0; kj < *image_width - (*kernel_size -1); kj++)
        {

            // Check if inside the image
            if (threadIdx.x > (*kernel_size/2 - 1) && threadIdx.x < (*image_width - (*kernel_size/2 - 1)))
            {
                // Get image pixel RGB
                int B = (int)  rows[ki * *image_width * 3 + (threadIdx.x - 1 + kj) * 3];
                int G = (int)  rows[ki * *image_width * 3 + (threadIdx.x - 1 + kj) * 3 + 1];
                int R = (int)  rows[ki * *image_width * 3 + (threadIdx.x - 1 + kj) * 3 + 2];
                // int G = (int)  *rows[ki][threadIdx.x - 1 + kj][1];
                // int R = (int)  *rows[ki][threadIdx.x - 1 + kj][2];

                // Update new pixel sum
                newPixelB += (kernel[ki * *kernel_size + kj] * B);
                newPixelG += (kernel[ki * *kernel_size + kj] * G);
                newPixelR += (kernel[ki * *kernel_size + kj] * R);
            }
        }
    }

    //  Normalize pixel and bound it
    newPixelB /= *kernel_total;
    newPixelB = max(newPixelB, 0);
    newPixelB = min(newPixelB, 255);

    newPixelG /= *kernel_total;
    newPixelG = max(newPixelG, 0);
    newPixelG = min(newPixelG, 255);

    newPixelR /= *kernel_total;
    newPixelR = max(newPixelR, 0);
    newPixelR = min(newPixelR, 255);

    // Update new image pixel    
    result_image[((blockIdx.x + (*kernel_size / 2)) * *image_width * 3) + threadIdx.x] = newPixelB;
    result_image[((blockIdx.x + (*kernel_size / 2)) * *image_width * 3) + threadIdx.x + 1] = newPixelG;
    result_image[((blockIdx.x + (*kernel_size / 2)) * *image_width * 3) + threadIdx.x + 2] = newPixelR;
    
}

int main(int argc, char **argv)
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("Current working dir: %s\n", cwd);
    // //----------------------------------
    // // Check number of arguments
    // if (argc < 3)
    // {
    //     cout << "Ingrese todos los argumentos necesarios para ejecutar el proceso" << endl;
    //     return -1;
    // }

    // Get the arguments
    // string path_image = argv[1];
    // string path_save = argv[2];
    string path_image = "/home/naburoky/Paralela/git_remoto/Paralela/lenna.png";
    string path_save = "out.png";
    //----------------------------------

    // Read the image
    //----------------------------------
    image = imread(path_image, IMREAD_COLOR); // Read the file
    
    if (!image.data)
    {
        cout << "Could not open or find the image" << endl;
        return -1;
    }
    cout<< image << endl;
    //----------------------------------

    // Error code to check return values for CUDA calls
    cudaError_t err = cudaSuccess;

    // Allocate the host output image
    //----------------------------------
    Mat result_image = Mat(image.rows, image.cols, CV_8UC3, Scalar(0, 0, 0));

    // Allocate the device variables
    //----------------------------------
    // Input
    unsigned char *d_image = NULL;
    err = cudaMalloc((void **)&d_image, image.rows * image.cols * image.channels() * sizeof(unsigned char));
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to allocate device image (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    unsigned char *d_kernel = NULL;
    err = cudaMalloc((void **)&d_kernel, kernel.size() * kernel.size() * sizeof(int));
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to allocate device kernel (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    float *d_kernel_total = NULL;
    err = cudaMalloc((void **)&d_kernel_total, sizeof(float));
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to allocate device kernel total (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    int *d_kernel_size = NULL;
    err = cudaMalloc((void **)&d_kernel_size, sizeof(int));
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to allocate device kernel total (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    int *d_image_width = NULL;
    err = cudaMalloc((void **)&d_image_width, sizeof(int));
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to allocate device image width (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    int *d_image_height = NULL;
    err = cudaMalloc((void **)&d_image_height, sizeof(int));
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to allocate device image height (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }


    //Output
    unsigned char *d_result_image = NULL;
    err = cudaMalloc((void **)&d_result_image, image.rows * image.cols * image.channels() * sizeof(unsigned char));
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to allocate device result image (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    //----------------------------------

    // Copy data from host to the device
    //----------------------------------
    err = cudaMemcpy(d_image, image.data, image.rows * image.cols * image.channels() * sizeof(unsigned char), cudaMemcpyHostToDevice);
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to copy image from host to device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaMemcpy(d_kernel, &kernel[0][0], kernel.size() * kernel.size() * sizeof(int), cudaMemcpyHostToDevice);
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to copy kernel from host to device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaMemcpy(d_kernel_total, &kernel_total, sizeof(float), cudaMemcpyHostToDevice);
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to copy kernel total from host to device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    int kernel_size = kernel.size();
    err = cudaMemcpy(d_kernel_size, &kernel_size, sizeof(int), cudaMemcpyHostToDevice);
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to copy kernel size from host to device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaMemcpy(d_image_width, &image.rows, sizeof(int), cudaMemcpyHostToDevice);
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to copy image width from host to device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaMemcpy(d_image_width, &image.cols, sizeof(int), cudaMemcpyHostToDevice);
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to copy image height from host to device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    //----------------------------------

    // Launch the Vector Add CUDA Kernel
    int threadsPerBlock = 64;
    int blocksPerGrid = 40; //TODO: adjust this

    filterImage<<<blocksPerGrid, threadsPerBlock, kernel.size()*image.rows*3*sizeof(unsigned char) >>>(d_image, d_result_image, d_kernel, d_kernel_total, d_kernel_size, d_image_width, d_image_height);
    err = cudaGetLastError();
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to launch filterImage process (kernel) (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // Copy the device result vector in device memory to the host result vector
    // in host memory.
    printf("Copy output data from the CUDA device to the host memory\n");
    err = cudaMemcpy(result_image.data, d_result_image, image.rows * image.cols * image.channels() * sizeof(unsigned char), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to copy result image from device to host (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    //cout << result_image << endl;

    // Free device global memory
    err = cudaFree(d_image);
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to free device image (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaFree(d_kernel);
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to free device kernel (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaFree(d_kernel_total);
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to free device kernel total (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    
    if (!imwrite(path_save, result_image)){
        cout << "Could not save the image" << endl;
        return -1;
    }

    // Reset the device and exit
    err = cudaDeviceReset();
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to deinitialize the device! error=%s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    return 0;
}