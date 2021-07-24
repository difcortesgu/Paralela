#include "device_launch_parameters.h"
#include <device_functions.h>
#include <cuda_runtime_api.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <stdio.h>

cudaError_t filterImage(cv::Mat image, int *kernel, int kernel_size, cv::Mat result_image);

__global__ void filterImagekernel(const uchar* image, const int* kernel, float kernel_total, int kernel_size, int image_width, int image_height, int image_channels, int blocks, int threads, uchar* result_image)
{
    //printf("kernel_total: %f \nkernel_size: %d \nimage_width: %d \nimage_height: %d \nimage_channels: %d\n", kernel_total, kernel_size, image_width, image_height, image_channels);

    //printf("kernel:\n");
    //for (int i = 0; i < kernel_size; i++) {
    //    for (int j = 0; j < kernel_size; j++)
    //    {
    //        printf("%d ", kernel[i * kernel_size + j]);
    //    }
    //    printf("\n");
    //}

    //printf("image:\n");

    //for (int row = 0; row < image_height; row++) 
    //{
    //    for (int col = 0; col < image_width; col++)
    //    {
    //        for (int channel = 0; channel < image_channels; channel++)
    //        {
    //            printf("%d ", image[row * image_width * image_channels + col * image_channels + channel]);
    //        }
    //        printf("\t");
    //    }
    //    printf("\n");
    //}
    extern __shared__ unsigned char buffer[];

    int initial_row = blockIdx.x * (image_height / blocks);
    int final_row = blockIdx.x + 1 * (image_height / blocks);

    if (blockIdx.x == blocks - 1)
        final_row = image_height;

    int initial_col = threadIdx.x * (image_width / threads);
    int final_col = threadIdx.x + 1 * (image_width / threads);

    if (threadIdx.x == threads - 1)
        final_col = image_width;


    for (int row = initial_row ; row < final_row; row++)
    {
        for (int col = initial_col; col < final_col; col++)
        {
            for (int channel = 0; channel < image_channels; channel++)
            {
                buffer[row * image_width * image_channels + col * image_channels + channel] = image[row * image_width * image_channels + col * image_channels + channel];
            }
        }
    }

    //__syncthreads();


    initial_row = blockIdx.x * ((image_height - kernel_size + 1) / blocks);
    final_row = blockIdx.x + 1 * ((image_height - kernel_size + 1) / blocks);

    if (blockIdx.x == blocks - 1)
        final_row = (image_height - kernel_size + 1);

    initial_col = threadIdx.x * ((image_height - kernel_size + 1) / threads);
    final_col = threadIdx.x + 1 * ((image_height - kernel_size + 1) / threads);

    if (threadIdx.x == threads - 1)
        final_col = (image_height - kernel_size + 1);

    int newPixelR;
    int newPixelG;
    int newPixelB;

    for (int row = initial_row; row < final_row; row++)
    {
        for (int col = initial_col; col < final_col; col++)
        {
            newPixelR = 0;
            newPixelG = 0;
            newPixelB = 0;

            // Loop kernel rows
            for (int krow = 0; krow < kernel_size; krow++)
            {
                // Loop kernel cols
                for (int kcol = 0; kcol < kernel_size; kcol++)
                {
                    int pos = (row + krow) * image_width * image_channels + (col + kcol) * image_channels;
                    int kpos = krow * kernel_size + kcol;

                    // Update new pixel sum
                    newPixelB += (kernel[kpos] * (int)buffer[pos]);
                    newPixelG += (kernel[kpos] * (int)buffer[pos + 1]);
                    newPixelR += (kernel[kpos] * (int)buffer[pos + 2]);
                }
            }

            //  Normalize pixel and bound it
            newPixelB /= kernel_total;
            if (newPixelB < 0) newPixelB = 0;
            if (newPixelB < 255) newPixelB = 255;

            newPixelG /= kernel_total;
            if (newPixelG < 0) newPixelG = 0;
            if (newPixelG < 255) newPixelG = 255;

            newPixelR /= kernel_total;
            if (newPixelR < 0) newPixelR = 0;
            if (newPixelR < 255) newPixelR = 255;
            
            printf("R:%d G:%d B:%d\n", newPixelR, newPixelG, newPixelB);
            // Update new image pixel
            result_image[(row + (kernel_size / 2)) * image_width * image_channels + (col + (kernel_size / 2)) * image_channels] = (uchar)newPixelB;
            result_image[(row + (kernel_size / 2)) * image_width * image_channels + (col + (kernel_size / 2)) * image_channels + 1] = (uchar)newPixelG;
            result_image[(row + (kernel_size / 2)) * image_width * image_channels + (col + (kernel_size / 2)) * image_channels + 2] = (uchar)newPixelR;

        }
    }
}

int main()
{


    //NVIDIA GeForce GTX 1050 Ti
    //    CUDA Driver Version / Runtime Version          11.4 / 11.4
    //    CUDA Capability Major / Minor version number : 6.1
    //    Total amount of global memory : 4096 MBytes(4294967296 bytes)
    //    (006) Multiprocessors, (128) CUDA Cores / MP : 768 CUDA Cores
    //    Memory Bus Width : 128 - bit
    //    L2 Cache Size : 1048576 bytes
    //    Total amount of constant memory : 65536 bytes
    //    Total amount of shared memory per block : 49152 bytes
    //    Total shared memory per multiprocessor : 98304 bytes
    //    Total number of registers available per block : 65536
    //    Warp size : 32
    //    Maximum number of threads per multiprocessor : 2048
    //    Maximum number of threads per block : 1024

    cv::Mat image = cv::imread("C:\\Users\\User\\source\\repos\\test\\lenna.png", cv::IMREAD_COLOR); // Read the file
    if (!image.data)
    {
        std::cout << "Could not open or find the image" << std::endl;
        return -1;
    }
    
    cv::Mat result_image = cv::Mat(image.rows, image.cols, CV_8UC3, cv::Scalar(0, 0, 0));

    int kernel[9] = {
        0, 1, 0,
        1, -4, 1,
        0, 1, 0
    };

    // Add vectors in parallel.
    cudaError_t cudaStatus = filterImage(image, kernel, 3, result_image);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "Filter image failed!");
        return 1;
    }

    if (!cv::imwrite("C:\\Users\\User\\source\\repos\\test\\lenna_out.png", result_image)) {
        std::cout << "Could not save the image" << std::endl;
        return -1;
    }

    // cudaDeviceReset must be called before exiting in order for profiling and
    // tracing tools such as Nsight and Visual Profiler to show complete traces.
    cudaStatus = cudaDeviceReset();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceReset failed!");
        return 1;
    }

    return 0;
}

cudaError_t filterImage(cv::Mat image, int *kernel, int kernel_size, cv::Mat result_image)
{

    for (int i = 0; i < kernel_size; i++)
    {
        for (int j = 0; j < kernel_size; j++)
        {
            std::cout << kernel[i*kernel_size + j] << " ";
        }
        std::cout << std::endl;
    }

    //Declare pointers and variables
    uchar* d_image;
    uchar* d_result_image;
    int* d_kernel;
    int kernel_total = 1;
    int image_width = image.rows;
    int image_height = image.cols;
    int image_channels = image.channels();

    cudaError_t cudaStatus = cudaSuccess;

    try
    {
        // Choose which GPU to run on, change this on a multi-GPU system.
        cudaStatus = cudaSetDevice(0);
        if (cudaStatus != cudaSuccess) throw "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?";

        // Allocate GPU buffers.
        cudaStatus = cudaMalloc((void**)&d_image, image.rows * image.cols * image.channels() * sizeof(uchar));
        if (cudaStatus != cudaSuccess) throw "cudaMalloc failed! image";

        cudaStatus = cudaMalloc((void**)&d_kernel, kernel_size * kernel_size * sizeof(int));
        if (cudaStatus != cudaSuccess) throw "cudaMalloc failed! kernel";

        cudaStatus = cudaMalloc((void**)&d_result_image, image.rows * image.cols * image.channels() * sizeof(uchar));
        if (cudaStatus != cudaSuccess) throw "cudaMalloc failed! result image";


        // Copy input vectors from host memory to GPU buffers.
        cudaStatus = cudaMemcpy(d_image, image.data, image.rows * image.cols * image.channels() * sizeof(uchar), cudaMemcpyHostToDevice);
        if (cudaStatus != cudaSuccess) throw "cudaMemcpy failed! image to device";

        cudaStatus = cudaMemcpy(d_kernel, kernel, kernel_size * kernel_size * sizeof(int), cudaMemcpyHostToDevice);
        if (cudaStatus != cudaSuccess) throw "cudaMemcpy failed! kernel to device";

        cudaStatus = cudaMemcpy(d_image, image.data, image.rows * image.cols * image.channels() * sizeof(uchar), cudaMemcpyHostToDevice);
        if (cudaStatus != cudaSuccess) throw "cudaMemcpy failed! result image to device";

        int blocks = 24;
        int threads_per_block = 128;

        int shared_buffer_size = image_width * image_height * image_channels / blocks ;

        // Launch a kernel on the GPU with one thread for each element.
        filterImagekernel << <blocks, threads_per_block, shared_buffer_size>> > (d_image, d_kernel, kernel_total, kernel_size, image_width, image_height, image_channels, blocks, threads_per_block, d_result_image);

        // Check for any errors launching the kernel
        cudaStatus = cudaGetLastError();
        if (cudaStatus != cudaSuccess) {
            std::cout << cudaGetErrorString(cudaStatus) << std::endl;
            throw "Error aaaaaaaaaaaaaaaaaaaa";
        }

        // cudaDeviceSynchronize waits for the kernel to finish, and returns
        // any errors encountered during the launch.
        //cudaStatus = cudaDeviceSynchronize();
        //if (cudaStatus != cudaSuccess) throw "cudaDeviceSynchronize returned error after launching kernel!";

        // Copy output vector from GPU buffer to host memory.
        cudaStatus = cudaMemcpy(result_image.data, d_result_image, image.rows * image.cols * image.channels() * sizeof(uchar), cudaMemcpyDeviceToHost);
        if (cudaStatus != cudaSuccess) throw "cudaMemcpy failed! result image to host";


    }
    catch (char *message)
    {
        cudaFree(d_image);
        cudaFree(d_result_image);
        cudaFree(d_kernel);
        std::cerr << message << std::endl;
    }
     return cudaStatus;
}
