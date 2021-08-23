#include "device_launch_parameters.h"
#include <cuda_runtime_api.h>
#include <cuda/std/cmath>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <stdio.h>
#include <sys/time.h>

using namespace std;


cudaError_t mosaicImage(
    cv::Mat image,
    cv::Mat index_image,
    cv::Mat mosaic_image,
    cv::Mat result_image,
    int tile_size,
    int new_tile_size,
    int n_vertical_tiles,
    int n_horizontal_tiles,
    int n_blocks,
    int n_threads
);

__global__ void mosaicImagekernel(
    const uchar* image,
    int image_width,
    int image_height,
    int image_channels,
    const uchar* index_image,
    int index_image_size,
    const uchar* mosaic_image,
    int mosaic_image_width,
    uchar* result_image,
    int result_image_width,
    int tile_size,
    int new_tile_size,
    int colors_per_thread,
    int blocks,
    int threads,
    int tiles_per_block,
    int tiles_per_thread,
    int tiles_per_row,
    int total_tiles
)
{
    extern __shared__ uchar index[];

    int begin = min(threadIdx.x * colors_per_thread, index_image_size);
    int end = min((threadIdx.x + 1) * colors_per_thread, index_image_size);

    for (int i = begin; i < end; i++)
    {
        index[i * image_channels] = index_image[i * image_channels];
        index[i * image_channels + 1] = index_image[i * image_channels + 1];
        index[i * image_channels + 2] = index_image[i * image_channels + 2];
    }

    __syncthreads();

    // Get iteration boundaries
    int initial_tile = min(min(blockIdx.x * tiles_per_block + threadIdx.x * tiles_per_thread, (blockIdx.x + 1) * tiles_per_block), total_tiles);
    int final_tile = min(min(initial_tile + tiles_per_thread, (blockIdx.x + 1) * tiles_per_block), total_tiles);

    int newPixelB, newPixelG, newPixelR;

    // Loop tiles
    for (int ti = initial_tile; ti < final_tile; ti++)
    {
        // Start of the tile
        int row = (ti / tiles_per_row);
        int col = (ti % tiles_per_row);

        int pos = ( row * image_width * tile_size * image_channels) + (col * tile_size * image_channels);

        newPixelB = 0;
        newPixelG = 0;
        newPixelR = 0;

        // Loop rows of pixels of the tile
        for (int pi = 0; pi < tile_size; pi++)
        {
            // Loop cols of pixels of the tile
            for (int pj = 0; pj < tile_size; pj++)
            {
                // Get pixel
                int pix_pos = pos + (pi * image_width * image_channels) + (pj * image_channels);

                // Add the values to the new pixel
                newPixelB += (int)image[pix_pos];
                newPixelG += (int)image[pix_pos + 1];
                newPixelR += (int)image[pix_pos + 2];
            }
        }

        newPixelB /= (tile_size * tile_size);
        newPixelG /= (tile_size * tile_size);
        newPixelR /= (tile_size * tile_size);

        // Get the closest color
        float minDiff = 10000;
        int closest_pos = 0;

        for (int i = 0; i < index_image_size; i++)
        {
            float diff = norm3df(newPixelB - index[i * image_channels], newPixelG - index[i * image_channels + 1], newPixelR - index[i * image_channels + 2]);
            if (diff < minDiff)
            {
                minDiff = diff;
                closest_pos = i;
            }
        }

        pos = (row * result_image_width * new_tile_size * image_channels) + (col * new_tile_size * image_channels);

        // Update new image pixels with the "pixel" image
        for (int ni = 0; ni < new_tile_size; ni++)
        {
            for (int nj = 0; nj < new_tile_size; nj++)
            {
                int pix_pos = pos + (ni * result_image_width * image_channels) + (nj * image_channels);

                //result_image[pix_pos] = newPixelB;
                //result_image[pix_pos + 1] = newPixelG;
                //result_image[pix_pos + 2] = newPixelR;

                //result_image[pix_pos] = index[closest_pos * image_channels];
                //result_image[pix_pos + 1] = index[closest_pos * image_channels + 1];
                //result_image[pix_pos + 2] = index[closest_pos * image_channels + 2];

                result_image[pix_pos] = mosaic_image[(closest_pos * mosaic_image_width * mosaic_image_width * image_channels) + (ni * mosaic_image_width * image_channels) + (nj * image_channels)];
                result_image[pix_pos + 1] = mosaic_image[(closest_pos * mosaic_image_width * mosaic_image_width * image_channels) + (ni * mosaic_image_width * image_channels) + (nj * image_channels) + 1];
                result_image[pix_pos + 2] = mosaic_image[(closest_pos * mosaic_image_width * mosaic_image_width * image_channels) + (ni * mosaic_image_width * image_channels) + (nj * image_channels) + 2];
            }
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

    int tile_size = 10, new_tile_size = 10, n_blocks = 24, n_threads = 128;

    //Get the arguments
    std::string path_image = argv[1];
    std::string path_save = argv[2];
    std::string path_index_image = "C:\\imagenes\\indexImage.jpg";
    std::string path_mosaic_image = "C:\\imagenes\\mosaicImage.jpg";

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

    tile_size = atoi(argv[5]);
    if (tile_size == 0)
    {
        cout << "El argumento de tamaño es invalido" << endl;
        return -1;
    }

    new_tile_size = atoi(argv[6]);
    if (new_tile_size == 0)
    {
        cout << "El argumento de tamaño nuevo es invalido" << endl;
        return -1;
    }

    cv::Mat image = cv::imread(path_image, cv::IMREAD_COLOR); // Read the file
    if (!image.data)
    {
        std::cout << "Could not open or find the image" << std::endl;
        return -1;
    }

    cv::Mat index_image = cv::imread(path_index_image, cv::IMREAD_COLOR); // Read the file
    if (!index_image.data)
    {
        std::cout << "Could not open or find the index image" << std::endl;
        return -1;
    }

    cv::Mat mosaic_image = cv::imread(path_mosaic_image, cv::IMREAD_COLOR); // Read the file
    if (!mosaic_image.data)
    {
        std::cout << "Could not open or find the mosaic image" << std::endl;
        return -1;
    }

    new_tile_size = min(new_tile_size, mosaic_image.cols);

    int h_pad = image.cols % tile_size;
    int v_pad = image.rows % tile_size;

    if (h_pad != 0)
    {
        h_pad = tile_size - h_pad;
    }

    if (v_pad != 0)
    {
        v_pad = tile_size - v_pad;
    }

    //Add padding to the image so that each tile is the same size
    cv::copyMakeBorder(image, image, 0, v_pad, 0, h_pad, cv::BORDER_REFLECT);

    int n_vertical_tiles = image.rows / tile_size;
    int n_horizontal_tiles = image.cols / tile_size;

    cv::Mat result_image = cv::Mat(n_vertical_tiles * new_tile_size, n_horizontal_tiles * new_tile_size, CV_8UC3, cv::Scalar(0, 0, 0));

    // Add vectors in parallel.
    cudaError_t cudaStatus = mosaicImage(image, index_image, mosaic_image, result_image, tile_size, new_tile_size, n_vertical_tiles, n_horizontal_tiles, n_blocks, n_threads);
    if (cudaStatus != cudaSuccess) {
        printf("%s\n", cudaGetErrorString(cudaStatus));
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
        printf("cudaDeviceReset failed!");
        return 1;
    }

    return 0;
}

cudaError_t mosaicImage(
    cv::Mat image,
    cv::Mat index_image,
    cv::Mat mosaic_image,
    cv::Mat result_image,
    int tile_size,
    int new_tile_size,
    int n_vertical_tiles,
    int n_horizontal_tiles,
    int n_blocks,
    int n_threads
)
{
    //Declare pointers and variables
    uchar* d_image, * d_index_image, * d_mosaic_image, * d_result_image;

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

        cudaStatus = cudaMalloc((void**)&d_index_image, index_image.rows * index_image.cols * index_image.channels() * sizeof(uchar));
        if (cudaStatus != cudaSuccess) throw "cudaMalloc failed! image";

        cudaStatus = cudaMalloc((void**)&d_mosaic_image, mosaic_image.rows * mosaic_image.cols * mosaic_image.channels() * sizeof(uchar));
        if (cudaStatus != cudaSuccess) throw "cudaMalloc failed! image";

        cudaStatus = cudaMalloc((void**)&d_result_image, result_image.rows * result_image.cols * result_image.channels() * sizeof(uchar));
        if (cudaStatus != cudaSuccess) throw "cudaMalloc failed! result image";


        // Copy input vectors from host memory to GPU buffers.
        cudaStatus = cudaMemcpy(d_image, image.data, image.rows * image.cols * image.channels() * sizeof(uchar), cudaMemcpyHostToDevice);
        if (cudaStatus != cudaSuccess) throw "cudaMemcpy failed! image to device";

        cudaStatus = cudaMemcpy(d_index_image, index_image.data, index_image.rows * index_image.cols * index_image.channels() * sizeof(uchar), cudaMemcpyHostToDevice);
        if (cudaStatus != cudaSuccess) throw "cudaMemcpy failed! result image to device";

        cudaStatus = cudaMemcpy(d_mosaic_image, mosaic_image.data, mosaic_image.rows * mosaic_image.cols * mosaic_image.channels() * sizeof(uchar), cudaMemcpyHostToDevice);
        if (cudaStatus != cudaSuccess) throw "cudaMemcpy failed! result image to device";

        int tiles_per_block = std::ceil((float)(n_vertical_tiles * n_horizontal_tiles) / (float)n_blocks);
        int tiles_per_thread = std::ceil((float)tiles_per_block / (float)n_threads);
        int index_image_size = index_image.rows * index_image.cols * index_image.channels() * sizeof(uchar);
        int colors_per_thread = std::ceil((float)index_image.rows / (float)n_threads);

        // Get initial time
        struct timeval tval_before, tval_after, tval_result;
        gettimeofday(&tval_before, NULL);

        // Launch a kernel on the GPU with one thread for each element.
        mosaicImagekernel << <n_blocks, n_threads, index_image_size >> > (d_image, image.cols, image.rows, image_channels, d_index_image, index_image.rows, d_mosaic_image, mosaic_image.cols, d_result_image, result_image.cols, tile_size, new_tile_size, colors_per_thread, n_blocks, n_threads, tiles_per_block, tiles_per_thread, n_horizontal_tiles, n_horizontal_tiles * n_vertical_tiles);


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
        std::cout << "bloques: " << n_blocks << std::endl;
        std::cout << "hilos por bloque: " << n_threads << std::endl;

        //if (cudaStatus != cudaSuccess) throw "cudaDeviceSynchronize returned error after launching kernel!";

        // Copy output vector from GPU buffer to host memory.

        cudaStatus = cudaMemcpy(result_image.data, d_result_image, result_image.rows * result_image.cols * result_image.channels() * sizeof(uchar), cudaMemcpyDeviceToHost);
        if (cudaStatus != cudaSuccess) {
            std::cout << cudaGetErrorString(cudaStatus) << 2 << std::endl;
            return cudaStatus;
        }
        // if (cudaStatus != cudaSuccess) throw "cudaMemcpy failed! result image to host";
    }
    catch (char* message)
    {
        cudaFree(d_image);
        cudaFree(d_index_image);
        cudaFree(d_mosaic_image);
        cudaFree(d_result_image);
        std::cout << message << std::endl;
    }
    cudaFree(d_image);
    cudaFree(d_index_image);
    cudaFree(d_mosaic_image);
    cudaFree(d_result_image);
    return cudaStatus;
}