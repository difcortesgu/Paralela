#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <sys/time.h>
#include <string>
// #include <cuda_runtime.h>

using namespace cv;
using namespace std;

Mat image, newImage;
vector<vector<int>> kernel {
        {0, 1, 0},
        {1, -4, 1},
        {0, 1, 0}
    };
double kernel_total = 0;

__global__ void filterImage(const unsigned char *image, const int *kernel, float *kernel_total, int* kernel_size, unsigned char *result_image, int *image_width, int *image_height){
    /*
        On each block run a row of the image or a fraction of it if it's too big (>4k?)
        each thread brings 3 items to the shared memory

        ->  * * * * * * * * * * * * * * * ... * * * * * * * * * 
        ->  * * * * * * * * * * * * * * * ... * * * * * * * * * 
        ->  * * * * * * * * * * * * * * * ... * * * * * * * * * 
            * * * * * * * * * * * * * * * ... * * * * * * * * * 
            * * * * * * * * * * * * * * * ... * * * * * * * * * 
            * * * * * * * * * * * * * * * ... * * * * * * * * * 
            * * * * * * * * * * * * * * * ... * * * * * * * * * 
        
        then sync all threads in the block
        run a multiplication of the kernel with the image section on each thread
    */
    blockId.x 
    threadId.x

    int result_image_width = image_width - (kernel_size -1);
    int result_image_height = image_height - (kernel_size -1);

    for (int i = 0; i < result_image_height; i++) {
        
    }
    
}

int main(int argc, char **argv)
{
    //----------------------------------
    // Check number of arguments
    if (argc < 3)
    {
        cout << "Ingrese todos los argumentos necesarios para ejecutar el proceso" << endl;
        return -1;
    }

    // Get the arguments
    string path_image = argv[1];
    string path_save = argv[2];
    //----------------------------------

    // Read the image
    //----------------------------------
    image = imread(path_image, IMREAD_COLOR); // Read the file
    if (!image.data)
    {
        cout << "Could not open or find the image" << endl;
        return -1;
    }
    //----------------------------------

    // Error code to check return values for CUDA calls
    cudaError_t err = cudaSuccess;

    // Allocate the host output image
    //----------------------------------
    Mat result_image = Mat(image.rows - (kernel.size() - 1), image.cols - (kernel.size() - 1), CV_8UC3, Scalar(0, 0, 0));


    // Allocate the device variables
    //----------------------------------
    // Input
    unsigned char *d_image = NULL;
    err = cudaMalloc((void **)&d_image, image.rows * image.cols * image.channels() * sizeof(unsigned char));
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to allocate device image (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    int *d_kernel = NULL;
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

    //Output
    unsigned char *d_result_image = NULL;
    err = cudaMalloc((void **)&d_result_image, (image.rows - (kernel.size() - 1)) * (image.cols - (kernel.size() - 1)) * image.channels() * sizeof(unsigned char));
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
    //----------------------------------

    // Launch the Vector Add CUDA Kernel
    int threadsPerBlock = 64;
    int blocksPerGrid = 40; //TODO: adjust this

    filterImage<<<blocksPerGrid, threadsPerBlock>>>(d_image, d_kernel, d_kernel_total, d_result_image, image.rows, image.cols);
    err = cudaGetLastError();
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to launch filterImage process (kernel) (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // Copy the device result vector in device memory to the host result vector
    // in host memory.
    printf("Copy output data from the CUDA device to the host memory\n");
    err = cudaMemcpy(result_image.data, d_result_image, (image.rows - (kernel.size() - 1)) * (image.cols - (kernel.size() - 1)) * image.channels() * sizeof(unsigned char), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to copy result image from device to host (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

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
    
    if (!imwrite(path_save, newImage)){
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