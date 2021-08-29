#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <stdio.h>
#include <sys/time.h>
#include "mpi.h"


void mosaicImagekernel(const uchar* image, int image_width, int image_channels, const uchar* index_image, int index_image_size, const uchar* mosaic_image, int mosaic_image_width, uchar* result_image, int result_image_width, int tile_size, int new_tile_size, int tiles_per_row, int total_tiles)
{
    int newPixelB, newPixelG, newPixelR;

    // Loop tiles
    for (int ti = 0; ti < total_tiles; ti++)
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
            float diff = sqrt(pow(newPixelB - index_image[i * image_channels], 2) + pow(newPixelB - index_image[i * image_channels + 1], 2) +pow(newPixelB - index_image[i * image_channels + 2], 2));
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
    if (argc < 3)
    {
        std::cout << "Ingrese todos los argumentos necesarios para ejecutar el proceso" << std::endl;
        return -1;
    }

    int tile_size = 10, new_tile_size = 10;

    //Get the arguments
    std::string path_image = argv[1];
    std::string path_save = argv[2];
    std::string path_index_image = "indexImage.jpg";
    std::string path_mosaic_image = "mosaicImage.jpg";


    tile_size = atoi(argv[3]);
    if (tile_size == 0)
    {
        std::cout << "El argumento de tamaño es invalido" << std::endl;
        return -1;
    }

    new_tile_size = atoi(argv[4]);
    if (new_tile_size == 0)
    {
        std::cout << "El argumento de tamaño nuevo es invalido" << std::endl;
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

    new_tile_size = std::min(new_tile_size, mosaic_image.cols);

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
    int total_tiles = n_horizontal_tiles * n_vertical_tiles;

    cv::Mat result_image = cv::Mat(n_vertical_tiles * new_tile_size, n_horizontal_tiles * new_tile_size, CV_8UC3, cv::Scalar(0, 0, 0));
    
    //Call the processing method
    int rank, size;
    MPI_Init(&argc, &argv);
        MPI_Comm_size( MPI_COMM_WORLD, &size );
        MPI_Comm_rank( MPI_COMM_WORLD, &rank );

        int div = n_vertical_tiles / size;
        int mod = n_vertical_tiles % size;

        int *b = (int *)malloc((div + mod) * (tile_size * image.cols * image.channels()) * sizeof(int));
        int *c = (int *)malloc(size * sizeof(int));
        int *d = (int *)malloc(size * sizeof(int));

        c[0] = (div + mod) * (tile_size * image.cols * image.channels());
        d[0] = 0;

        for (int i = 1; i < size; i++)
        {
            c[i] = div * (tile_size * image.cols * image.channels());
            d[i] = c[i-1] + d[i-1];
        }

        if (rank == 0)
        {
            std::cout << image.cols * image.rows * image.channels() << std::endl;


            for (int i = 0; i < size; i++)
            {
                std::cout << d[i] << " --- " << d[i] + c[i] << std::endl;

                c[i] = div * (tile_size * image.cols * image.channels());
                d[i] = c[i-1] + d[i-1];
            }
        }

        MPI_Scatterv((void *)image.data, c, d, MPI_INT, (void *)b, c[0], MPI_INT, 0, MPI_COMM_WORLD);

        // // MPI_Scatter((void *) a, 2, MPI_INT, (void * )b, 2, MPI_INT, 0, MPI_COMM_WORLD);
        // for (int i = 0; i < 3; i++)
        // {
        //     cout << "Process: "<< rank << " --- " << b[i] << endl;
        // }

    MPI_Finalize();


    if (!cv::imwrite(path_save, result_image)) {
        std::cout << "Could not save the image" << std::endl;
        return -1;
    }

    return 0;
}
