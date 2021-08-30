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

Mat filterImage(int id, Mat image, KernelInfo kernelInfo, int endIteration){

    Mat newImage = Mat(image.rows, image.cols, CV_8UC3, Scalar(0, 0, 0));

    if (id == 4)
    {
        endIteration -= 1;
    }
    

    // Loop image rows
    for (int i = 0; i < endIteration; i++)
    {
        // Loop image cols
        for (int j = 0; j < image.cols; j++)
        {
            int newPixel[] = {0, 0, 0};

            // Loop kernel rows
            for (int ki = kernelInfo.inicio; ki < kernelInfo.final; ki++)
            {
                // Loop kernel cols
                for (int kj = kernelInfo.inicio; kj < kernelInfo.final; kj++)
                {

                    // Check if inside the image
                    if (i + ki >= 0 && i + ki < image.rows && j + kj >= 0 && j + kj < image.cols)
                    {

                        // Get image pixel RGB
                        Vec3b pixel = image.at<Vec3b>(i + ki, j + kj);
                        int B = (int)pixel[0];
                        int G = (int)pixel[1];
                        int R = (int)pixel[2];

                        // Update new pixel sum
                        newPixel[0] += (kernelInfo.kernel[-ki + kernelInfo.offset][-kj + kernelInfo.offset] * B);
                        newPixel[1] += (kernelInfo.kernel[-ki + kernelInfo.offset][-kj + kernelInfo.offset] * G);
                        newPixel[2] += (kernelInfo.kernel[-ki + kernelInfo.offset][-kj + kernelInfo.offset] * R);
                    }
                }
            }

            //  Normalize pixel and bound it
            for (int k = 0; k < 3; k++)
            {
                newPixel[k] /= kernelInfo.kernel_total;
                newPixel[k] = max(newPixel[k], 0);
                newPixel[k] = min(newPixel[k], 255);
            }

            // Update new image pixel
            newImage.at<Vec3b>(i, j)[0] = newPixel[0];
            newImage.at<Vec3b>(i, j)[1] = newPixel[1];
            newImage.at<Vec3b>(i, j)[2] = newPixel[2];
        }
    }
    return newImage;
}

KernelInfo get_kernel_info(int filter){
    vector<vector<int>> kernel;
    int n_threads, inicio, final, offset;
    double kernel_total = 0;

    // Get kernel sum to use it later
    bool brillo = false;
    switch (filter)
    {
    case 1:
        //DETECCION DE BORDES
        kernel = {
            {0, 1, 0},
            {1, -4, 1},
            {0, 1, 0}};

        break; //optional
    case 2:
        // REPUJADO
        kernel = {
            {-2, -1, 0},
            {-1, 1, 1},
            {0, 1, 2}};

        break; //optional
    case 3:
        // DESENFOCADO 3x3
        kernel = {
            {1, 1, 1},
            {1, 1, 1},
            {1, 1, 1}};

        break; //optional
    case 4:
        // ENFOCADO
        kernel = {
            {0, -1, 0},
            {-1, 5, -1},
            {0, -1, 0}};
        break; //optional
    case 5:
        // brillo bajo
        kernel = {
            {0, 0, 0},
            {0, 1, 0},
            {0, 0, 0}};
        brillo = true;
        kernel_total = 1.5;
        break;
    case 6:
        // brillo alto
        kernel = {
            {0, 0, 0},
            {0, 1, 0},
            {0, 0, 0}};
        brillo = true;
        kernel_total = 0.5;
        break; //optional
    case 7:
        // DESENFOCADO 3x3
        kernel = {
            {1, 1, 1 , 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1 , 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1 , 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1 , 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1 , 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1 , 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1 , 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1 , 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1 , 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1 , 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1 , 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1 , 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1 , 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1 , 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1 , 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

        break;
    default:   //Identidad
        kernel = {
            {0, 0, 0},
            {0, 1, 0},
            {0, 0, 0}};
    }
    if (!brillo)
    {
        for (int i = 0; i < kernel.size(); i++)
        {
            for (int j = 0; j < kernel.size(); j++)
            {
                kernel_total += kernel[i][j];
            }
        }

        if (kernel_total == 0)
            kernel_total = 1;
    }

    // Get iteration indexed based on the kernel
    inicio = -(int)(kernel.size() / 2);
    final = (int)(kernel.size() / 2) + 1;
    offset = (int)(kernel.size() / 2);

    KernelInfo kernelInfo = {
        kernel,
        inicio,
        final,
        offset,
        kernel_total
    };
    
    return kernelInfo;
}

int main(int argc, char **argv){

    // Check number of arguments
    if (argc < 2)
    {
        cout << "Ingrese todos los argumentos necesarios para ejecutar el proceso" << endl;
        return -1;
    }

    // Get the arguments
    string path_image = argv[1];

    int filter = 0;
    filter = atoi(argv[2]);
    if (filter == 0)
    {
        cout << "El argumento de filtro es invalido" << endl;
        return -1;
    }

    Mat image = imread(path_image, IMREAD_COLOR); // Read the file
    if (!image.data)
    {
        cout << "Could not open or find the image" << endl;
        return -1;
    }
    int image_len = image.cols * image.rows * image.channels();


    int rank, size;
    MPI_Init(&argc, &argv);
        MPI_Comm_size( MPI_COMM_WORLD, &size );
        MPI_Comm_rank( MPI_COMM_WORLD, &rank );

        int rows_per_process = image.rows / size;
        int left_rows = image.rows % size;
        int div = rows_per_process * image.cols * image.channels();
        int mod = left_rows * image.cols * image.channels();

        int *c = (int *)malloc(size * sizeof(int));
        int *d = (int *)malloc(size * sizeof(int));

        c[0] = div + mod;
        d[0] = 0;

        for (int i = 1; i < size; i++)
        {
            c[i] = div;
            d[i] = c[i-1] + d[i-1];
        }

        KernelInfo kernelInfo = get_kernel_info(filter);
        Mat newImage = Mat(image.rows, image.cols, CV_8UC3, Scalar(0, 0, 0));
        Mat tempImage = Mat(rows_per_process, image.cols, CV_8UC3, Scalar(0, 0, 0));

        // Get initial time
        struct timeval tval_before, tval_after, tval_result;
        gettimeofday(&tval_before, NULL);

        MPI_Bcast((void *)image.data, image_len, MPI_CHAR, 0, MPI_COMM_WORLD);

        // MPI_Scatterv((void *)image.data, c, d, MPI_CHAR, (void *)tempImage.data, c[0], MPI_CHAR, 0, MPI_COMM_WORLD);
        // tempImage = filterImage(rank, tempImage, kernelInfo, rows_per_process);
        // MPI_Gatherv((void *)tempImage.data, c[0], MPI_CHAR, (void *)newImage.data, c, d, MPI_CHAR, 0, MPI_COMM_WORLD);

        if (rank == 0){
            // Calculate time
            gettimeofday(&tval_after, NULL);
            timersub(&tval_after, &tval_before, &tval_result);

            // Show results
            printf("%ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

            string path_save = "out_" + path_image;

            if (!imwrite(path_save, newImage)){
                cout << "Could not save the image" << endl;
                return -1;
            }
        }
        
    MPI_Finalize();

    return 0;
}
