#include "device_launch_parameters.h"
#include <cuda_runtime_api.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <stdio.h>
#include <sys/time.h>

cudaError_t filterImage(cv::Mat image, cv::Mat result_image);

int kernel[9] = {
            0, 1, 0,
            1, -4, 1,
            0, 1, 0 };
int kernel_total = 1, kernel_size = 3, n_blocks = 24, n_threads = 128;


__global__ void filterImagekernel(const uchar* image, const int* kernel, float kernel_total, int kernel_size, int image_width, int image_height, int image_channels, int blocks, int threads, int rows_per_block, int cols_per_thread, uchar* result_image)
{
    extern __shared__ uchar buffer[];

    int initial_row, final_row, initial_col, final_col;


    initial_row = blockIdx.x * rows_per_block - (kernel_size / 2);
    final_row = ((blockIdx.x + 1) * rows_per_block) + (kernel_size / 2);
    rows_per_block = rows_per_block + (kernel_size - 1);

    initial_col = threadIdx.x * cols_per_thread - (kernel_size / 2);
    final_col = ((threadIdx.x + 1) * cols_per_thread) + (kernel_size / 2);
    cols_per_thread = cols_per_thread + (kernel_size - 1);

    if (initial_row < 0)
    {
        initial_row = 0;
    }
    if (initial_col < 0)
    {
        initial_col = 0;
    }


    if (final_row > image_height)
    {
        rows_per_block = image_height - initial_row;
        final_row = image_height;
    }
    if (final_col > image_width)
    {
        cols_per_thread = image_width - initial_col;
        final_col = image_width;
    }

    //printf("block: %d, thread: %d, rows: (%d, %d), cols:(%d, %d), rows_per_block: %d, cols_per_thread: %d\n", blockIdx.x, threadIdx.x, initial_row, final_row, initial_col, final_col, rows_per_block, cols_per_thread);
    for (int row = 0; row < rows_per_block; row++)
    {
        for (int col = initial_col; col < final_col; col++)
        {
            for (int channel = 0; channel < image_channels; channel++)
            {
                buffer[(row * image_width * image_channels) + (col * image_channels) + channel] = image[((initial_row + row) * image_width * image_channels) + (col * image_channels) + channel];
            }
        }
    }

    __syncthreads();

    int newPixelR, newPixelG, newPixelB;

    rows_per_block = rows_per_block - (kernel_size - 1);
    final_row = final_row - (kernel_size - 1);

    cols_per_thread = cols_per_thread - (kernel_size - 1);
    final_col = final_col - (kernel_size - 1);

    for (int row = 0; row < rows_per_block; row++)
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
                    int pos = ((row + krow) * image_width * image_channels) + ((col + kcol) * image_channels);
                    int kpos = (krow * kernel_size) + kcol;

                    //printf("pos: %d\n", pos);

                    //// Update new pixel sum
                    newPixelB += (kernel[kpos] * (int)buffer[pos]);
                    newPixelG += (kernel[kpos + 1] * (int)buffer[pos + 1]);
                    newPixelR += (kernel[kpos + 2] * (int)buffer[pos + 2]);
                }
            }

            ////  Normalize pixel and bound it
            newPixelB /= kernel_total;
            if (newPixelB < 0) newPixelB = 0;
            if (newPixelB > 255) newPixelB = 255;

            newPixelG /= kernel_total;
            if (newPixelG < 0) newPixelG = 0;
            if (newPixelG > 255) newPixelG = 255;

            newPixelR /= kernel_total;
            if (newPixelR < 0) newPixelR = 0;
            if (newPixelR > 255) newPixelR = 255;

            //// Update new image pixel
            result_image[(initial_row + row + (kernel_size / 2)) * image_width * image_channels + (col + (kernel_size / 2)) * image_channels] = (uchar)newPixelB;
            result_image[(initial_row + row + (kernel_size / 2)) * image_width * image_channels + (col + (kernel_size / 2)) * image_channels + 1] = (uchar)newPixelG;
            result_image[(initial_row + row + (kernel_size / 2)) * image_width * image_channels + (col + (kernel_size / 2)) * image_channels + 2] = (uchar)newPixelR;

        }
    }

}

int main(int argc, char** argv)
{
    //Check number of arguments
    if (argc < 5)
    {
        std::cout << "Ingrese todos los argumentos necesarios para ejecutar el proceso" << std::endl;
        return -1;
    }

    //Get the arguments
    std::string path_image = argv[1];
    std::string path_save = argv[2];

    n_blocks = atoi(argv[3]);
    if (n_blocks == 0)
    {
        std::cout << "El número de bloques es invalido" << std::endl;
        return -1;
    }

    n_threads = atoi(argv[4]);
    if (n_threads == 0)
    {
        std::cout << "El número de hilos es invalido" << std::endl;
        return -1;
    }

    cv::Mat image = cv::imread(path_image, cv::IMREAD_COLOR); // Read the file
    if (!image.data)
    {
        std::cout << "Could not open or find the image" << std::endl;
        return -1;
    }

    cv::Mat result_image = cv::Mat(image.rows, image.cols, CV_8UC3, cv::Scalar(0, 0, 0));

    // Add vectors in parallel.
    cudaError_t cudaStatus = filterImage(image, result_image);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "%s\n", cudaGetErrorString(cudaStatus));
        fprintf(stderr, "Filter image failed!");
        return 1;
    }

    if (!cv::imwrite(path_save, result_image)) {
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

cudaError_t filterImage(cv::Mat image, cv::Mat result_image)
{
    //Declare pointers and variables
    uchar* d_image;
    uchar* d_result_image;
    int* d_kernel;
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


        int rows_per_block = std::ceil((float)image_height / (float)n_blocks);
        int cols_per_thread = std::ceil((float)image_width / (float)n_threads);
        int shared_buffer_size = (rows_per_block + (kernel_size - 1)) * image_width * image_channels * sizeof(uchar);

        // Get initial time
        struct timeval tval_before, tval_after, tval_result;
        gettimeofday(&tval_before, NULL);


        // Launch a kernel on the GPU with one thread for each element.
        filterImagekernel << <n_blocks, n_threads, shared_buffer_size >> > (d_image, d_kernel, kernel_total, kernel_size, image_width, image_height, image_channels, n_blocks, n_threads, rows_per_block, cols_per_thread, d_result_image);


        // Check for any errors launching the kernel
        cudaStatus = cudaGetLastError();
        if (cudaStatus != cudaSuccess) {
            std::cout << cudaGetErrorString(cudaStatus) << 0 << std::endl;
            return cudaStatus;
        }

        // cudaDeviceSynchronize waits for the kernel to finish, and returns
        // any errors encountered during the launch.
        cudaStatus = cudaDeviceSynchronize();
        if (cudaStatus != cudaSuccess) {
            std::cout << cudaGetErrorString(cudaStatus) << 1 << std::endl;
            return cudaStatus;
        }

        // Calculate time
        gettimeofday(&tval_after, NULL);
        timersub(&tval_after, &tval_before, &tval_result);

        // Show results
        printf("Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
        std::cout<<"bloques: "<< n_blocks << std::endl;
        std::cout<<"hilos por bloque: "<< n_threads << std::endl;


        //if (cudaStatus != cudaSuccess) throw "cudaDeviceSynchronize returned error after launching kernel!";

        // Copy output vector from GPU buffer to host memory.

        cudaStatus = cudaMemcpy(result_image.data, d_result_image, image.rows * image.cols * image.channels() * sizeof(uchar), cudaMemcpyDeviceToHost);
        if (cudaStatus != cudaSuccess) {
            std::cout << cudaGetErrorString(cudaStatus) << 2 << std::endl;
            return cudaStatus;
        }
        // if (cudaStatus != cudaSuccess) throw "cudaMemcpy failed! result image to host";


    }
    catch (char* message)
    {
        cudaFree(d_image);
        cudaFree(d_result_image);
        cudaFree(d_kernel);
        std::cout << message << std::endl;
    }
    return cudaStatus;
}
