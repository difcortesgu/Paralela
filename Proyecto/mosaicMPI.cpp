#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <sys/time.h>
#include <math.h>
#include "mpi.h"

using namespace std;
using namespace cv;

struct KernelInfo
{
    vector<vector<int>> kernel;
    int inicio, final, offset;
    double kernel_total = 0;
};

Mat processImage(Mat image, Mat index_image, Mat mosaic_image, int tile_size, int new_tile_size, int n_tile_rows, int n_tile_cols, int rank, int size)
{
    Mat newImage = Mat(n_tile_rows * new_tile_size, n_tile_cols * new_tile_size, CV_8UC3, Scalar(0, 0, 0));

    // Loop image rows
    for (int i = 0; i < n_tile_rows; i++)
    {
        // Loop image cols
        for (int j = 0; j < n_tile_cols; j++)
        {
            int newPixel[] = {0, 0, 0};

            // Loop block of pixels to mix
            for (int bi = 0; bi < tile_size; bi++)
            {
                for (int bj = 0; bj < tile_size; bj++)
                {
                    // Get image pixel RGB
                    Vec3b pixel = image.at<Vec3b>((i * tile_size) + bi, (j * tile_size) + bj);
                    int B = (int)pixel[0];
                    int G = (int)pixel[1];
                    int R = (int)pixel[2];

                    // Update new pixel sum
                    newPixel[0] += B;
                    newPixel[1] += G;
                    newPixel[2] += R;
                }
            }

            //  Normalize pixel and bound it
            for (int k = 0; k < 3; k++)
            {
                newPixel[k] /= pow(tile_size, 2);
            }


            // Get the closest color
            double minDiff = 10000;
            int closest_pos = 0;

            for (int i = 0; i < index_image.rows; i++)
            {
                Vec3b pixel = index_image.at<Vec3b>(i, 0);

                double diff = sqrt(pow(newPixel[0] -  pixel[0], 2) + pow(newPixel[1] - pixel[1], 2) + pow(newPixel[2] - pixel[2], 2));
                if (diff < minDiff)
                {
                    minDiff = diff;
                    closest_pos = i;
                }
            }
            
            int init_i = closest_pos * mosaic_image.cols;

            // Update new image pixels with the "pixel" image
            for (int ni = 0; ni < new_tile_size; ni++)
            {
                for (int nj = 0; nj < new_tile_size; nj++)
                {
                    // Vec3b newImagePixel = newImage.at<Vec3b>((i * new_tile_size) + ni, (j * new_tile_size) + nj);
                    Vec3b mosaicImagePixel = mosaic_image.at<Vec3b>(init_i + ni, nj);
                    
                    newImage.at<Vec3b>((i * new_tile_size) + ni, (j * new_tile_size) + nj)[0] = mosaicImagePixel[0];
                    newImage.at<Vec3b>((i * new_tile_size) + ni, (j * new_tile_size) + nj)[1] = mosaicImagePixel[1];
                    newImage.at<Vec3b>((i * new_tile_size) + ni, (j * new_tile_size) + nj)[2] = mosaicImagePixel[2];
                }
            }

        }
    }
    return newImage;
}

int main(int argc, char **argv){
    //Check number of arguments
    if (argc < 2)
    {
        std::cout << "Ingrese todos los argumentos necesarios para ejecutar el proceso" << std::endl;
        return -1;
    }

    int tile_size = 10, new_tile_size = 10;

    //Get the arguments
    string path_image = argv[1];
    string path_index_image = "indexImage.jpg";
    string path_mosaic_image = "mosaicImage.jpg";

    tile_size = atoi(argv[2]);
    if (tile_size == 0)
    {
        cout << "El argumento de tamaño es invalido" << endl;
        return -1;
    }

    new_tile_size = atoi(argv[3]);
    if (new_tile_size == 0)
    {
        cout << "El argumento de tamaño nuevo es invalido" << endl;
        return -1;
    }
    
    Mat image = imread(path_image, cv::IMREAD_COLOR); // Read the file
    if (!image.data)
    {
        cout << "Could not open or find the image" << endl;
        return -1;
    }

    Mat index_image = imread(path_index_image, cv::IMREAD_COLOR); // Read the file
    if (!index_image.data)
    {
        cout << "Could not open or find the index image" << endl;
        return -1;
    }

    Mat mosaic_image = imread(path_mosaic_image, cv::IMREAD_COLOR); // Read the file
    if (!mosaic_image.data)
    {
        cout << "Could not open or find the mosaic image" << endl;
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
    copyMakeBorder(image, image, 0, v_pad, 0, h_pad, BORDER_REFLECT);

    int n_vertical_tiles = image.rows / tile_size;
    int n_horizontal_tiles = image.cols / tile_size;
    // Mat result_image = Mat(n_vertical_tiles * new_tile_size, n_horizontal_tiles * new_tile_size, CV_8UC3, Scalar(0, 0, 0));
    
    int image_len = image.cols * image.rows * image.channels();
    int index_image_len = index_image.cols * index_image.rows * index_image.channels();
    int mosaic_image_len = mosaic_image.cols * mosaic_image.rows * mosaic_image.channels();


    int rank, size;
    
    MPI_Init(&argc, &argv);
        MPI_Comm_size( MPI_COMM_WORLD, &size );
        MPI_Comm_rank( MPI_COMM_WORLD, &rank );

        int tile_rows_per_process = n_vertical_tiles / size;
        int left_tile_rows = n_vertical_tiles % size;

        int div = tile_rows_per_process * tile_size * image.cols * image.channels();
        int mod = left_tile_rows * tile_size * image.cols * image.channels();

        int *counts = (int *)malloc(size * sizeof(int));
        int *displacements = (int *)malloc(size * sizeof(int));

        counts[0] = div + mod;
        displacements[0] = 0;

        for (int i = 1; i < size; i++)
        {
            counts[i] = div;
            displacements[i] = counts[i-1] + displacements[i-1];
        }

        // Get initial time
        struct timeval tval_before, tval_after, tval_result;
        gettimeofday(&tval_before, NULL);

        ///bcast index_image mosaic.img 
        MPI_Bcast((void *)index_image.data, index_image_len, MPI_CHAR, 0, MPI_COMM_WORLD);
        MPI_Bcast((void *)mosaic_image.data, mosaic_image_len, MPI_CHAR, 0, MPI_COMM_WORLD);

        Mat tempImage, tempResultImage;
        Mat result_image = Mat(n_vertical_tiles * new_tile_size, n_horizontal_tiles * new_tile_size, CV_8UC3, Scalar(0, 0, 0));
        string path_save;

        if (rank == 0){
            tempImage = Mat((tile_rows_per_process + left_tile_rows) * tile_size, image.cols, CV_8UC3, Scalar(0, 0, 0));
            
            MPI_Scatterv((void *)image.data, counts, displacements, MPI_CHAR, (void *)tempImage.data, div + mod, MPI_CHAR, 0, MPI_COMM_WORLD);

            tempResultImage = processImage(tempImage, index_image, mosaic_image, tile_size, new_tile_size,tile_rows_per_process + left_tile_rows, n_horizontal_tiles, rank, size);        

            div = tile_rows_per_process * new_tile_size * result_image.cols * result_image.channels();
            mod = left_tile_rows * new_tile_size * result_image.cols * result_image.channels();

            counts[0] = div + mod;
            displacements[0] = 0;

            for (int i = 1; i < size; i++)
            {
                counts[i] = div;
                displacements[i] = counts[i-1] + displacements[i-1];
            }

            MPI_Gatherv((void *)tempResultImage.data, div + mod, MPI_CHAR, (void *)result_image.data, counts, displacements, MPI_CHAR, 0, MPI_COMM_WORLD);
        
        }else{
            tempImage = Mat(tile_rows_per_process * tile_size, image.cols, CV_8UC3, Scalar(0, 0, 0));
            
            MPI_Scatterv((void *)image.data, counts, displacements, MPI_CHAR, (void *)tempImage.data, div, MPI_CHAR, 0, MPI_COMM_WORLD);

            tempResultImage = processImage(tempImage, index_image, mosaic_image, tile_size, new_tile_size, tile_rows_per_process, n_horizontal_tiles, rank, size);        

            div = tile_rows_per_process * new_tile_size * result_image.cols * result_image.channels();
            mod = left_tile_rows * new_tile_size * result_image.cols * result_image.channels();

            counts[0] = div + mod;
            displacements[0] = 0;

            for (int i = 1; i < size; i++)
            {
                counts[i] = div;
                displacements[i] = counts[i-1] + displacements[i-1];
            }

            MPI_Gatherv((void *)tempResultImage.data, div, MPI_CHAR, (void *)result_image.data, counts, displacements, MPI_CHAR, 0, MPI_COMM_WORLD);
        }


        if (rank == 0){
            // Calculate time
            gettimeofday(&tval_after, NULL);
            timersub(&tval_after, &tval_before, &tval_result);

            // Show results
            printf("%ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

            string path_save = "out_" + path_image;

            if (!imwrite(path_save, result_image)){
                cout << "Could not save the image" << endl;
                return -1;
            }
        }
        
    MPI_Finalize();

    return 0;
}
