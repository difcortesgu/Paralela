#include "device_launch_parameters.h"
#include <cuda_runtime_api.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <stdio.h>
#include <sys/time.h>

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

__global__ void filterImagekernel(
    const uchar* image, 
    const uchar* index_image, 
    const uchar* mosaic_image, 
    int image_width, 
    int image_height, 
    int image_channels, 
    int blocks, 
    int threads, 
    int tiles_per_block, 
    int tiles_per_thread, 
    uchar* result_image, 
    int tile_size, 
    int new_tile_size,
    int colors_per_thread
)
{
    extern __shared__ uchar index[];

    for (int i = threadIdx.x * colors_per_thread; i < (threadIdx.x + 1) * colors_per_thread; i++)
    {
        index[i * image_channels] = index_image[i * image_channels]
        index[i * image_channels + 1] = index_image[i * image_channels + 1]
        index[i * image_channels + 2] = index_image[i * image_channels + 2]
    }    
    __syncthreads();

    // Get iteration boundaries
    int initial_tile_row = blockIdx.x * tiles_per_block
    int final_tile_row = (blockIdx.x + 1) * tiles_per_block

    int initial_tile_col = threadIdx.x * tiles_per_block
    int final_tile_col = (threadIdx.x + 1) * tiles_per_block

    double newPixelB, newPixelG, newPixelR


    // Loop rows of tiles
    for (int ti = initial_tile_row; ti < final_tile_row; ti++)
    {
        // Loop cols of tiles
        for (int tj = initial_tile_col; tj < final_tile_col; tj++)
        {
            // Start of the tile
            int pos = (ti * tile_size * image_width * channels) + (tj * tile_size * channels)

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
                    int pix_pos = pos + (pi * image_width * channels) + (pj * channels)

                    // Add the values to the new pixel
                    newPixelB += (int)image[pix_pos];
                    newPixelG += (int)image[pix_pos + 1];
                    newPixelR += (int)image[pix_pos + 2];
                }
            }

            newPixelB /= (tile_size * tile_size)
            newPixelG /= (tile_size * tile_size)
            newPixelR /= (tile_size * tile_size)

            // Get the closest color
            float minDiff = 10000;
            int closest_pos = 0;

            for (int i = 0; i < index_image_size; i += channels)
            {
                float diff = norm3d(newPixelB-index_image[i], newPixelG-index_image[i+1],newPixelR-index_image[i+2])
                if (diff < minDiff)
                {
                    minDiff = diff;
                    closest_pos = i;
                }
            }

            int pos = (ti * new_tile_size * result_image_width * channels) + (tj * new_tile_size * channels)

            // Update new image pixels with the "pixel" image
            for (int ni = 0; ni < new_tile_size; ni++)
            {
                for (int nj = 0; nj < new_tile_size; nj++)
                {
                    int pix_pos = pos + (ni * result_image_width * channels) + (nj * channels)

                    result_image[pix_pos] = index_image[((closest_pos + i) * mosaicImage_width * channels) + j * channels];
                    result_image[pix_pos + 1] = index_image[((closest_pos + i) * mosaicImage_width * channels) + j * channels + 1];
                    result_image[pix_pos + 2] = index_image[((closest_pos + i) * mosaicImage_width * channels) + j * channels + 2];
                }
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
    std::string path_index_image = "./imagenes/indexImage.jpg";
    std::string path_mosaic_image = "./imagenes/mosaicImage.jpg";

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

    tile_size = atoi(argv[4]);
    if (tile_size == 0)
    {
        cout << "El argumento de tamaño es invalido" << endl;
        return -1;
    }

    new_tile_size = atoi(argv[5]);
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
    if (!image.data)
    {
        std::cout << "Could not open or find the image" << std::endl;
        return -1;
    }

    cv::Mat mosaic_image = cv::imread(path_mosaic_image, cv::IMREAD_COLOR); // Read the file
    if (!image.data)
    {
        std::cout << "Could not open or find the image" << std::endl;
        return -1;
    }

    int n_vertical_tiles = std::ceil((float)image.rows / (float)tile_size);
    int n_horizontal_tiles = std::ceil((float)image.cols / (float)tile_size);

    cv::Mat result_image = cv::Mat(n_vertical_tiles * tile_size, n_horizontal_tiles * tile_size, CV_8UC3, cv::Scalar(0, 0, 0));

    // Add vectors in parallel.
    cudaError_t cudaStatus = mosaicImage(image, index_image, mosaic_image, result_image, tile_size, new_tile_size, n_vertical_tiles, n_horizontal_tiles, n_blocks, n_threads);
    if (cudaStatus != cudaSuccess) {
        printf(stderr, "%s\n", cudaGetErrorString(cudaStatus));
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
        fprint("cudaDeviceReset failed!");
        return 1;
    }

    return 0;
}

cudaError_t filterImage(cv::Mat image, cv::Mat index_image, cv::Mat mosaic_image, cv::Mat result_image, int tile_size, int new_tile_size, int n_vertical_tiles, int n_horizontal_tiles, int n_blocks, int n_threads)
{
    //Declare pointers and variables
    uchar *d_image, *d_index_image, *d_mosaic_image, *d_result_image;

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

        int tiles_per_block = std::ceil((float)n_vertical_tiles / (float)n_blocks);
        int tiles_per_thread = std::ceil((float)n_horizontal_tiles / (float)n_threads);
        int index_image_size = index_image.rows * index_image.cols * index_image.channels() * sizeof(uchar);

        // Get initial time
        struct timeval tval_before, tval_after, tval_result;
        gettimeofday(&tval_before, NULL);


        // Launch a kernel on the GPU with one thread for each element.
        filterImagekernel <<<n_blocks, n_threads, index_image_size >>> (d_image, d_index_image, d_mosaic_image, image_width, image_height, image_channels, n_blocks, n_threads, tiles_per_block, tiles_per_thread, d_result_image, tile_size, new_tile_size);


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
    cudaFree(d_image);
    cudaFree(d_index_image);
    cudaFree(d_mosaic_image);
    cudaFree(d_result_image);
    return cudaStatus;
}