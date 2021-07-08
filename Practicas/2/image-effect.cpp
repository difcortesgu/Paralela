#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <sys/time.h>
#include <string>
#include <omp.h>

using namespace cv;
using namespace std;

Mat image, newImage;
vector<vector<int>> kernel;
int n_threads, inicio, final, offset;
double kernel_total = 0;

void *filterImage(int id)
{
    // Get iteration boundaries
    int threadId = id;
    int initIteration = (int)(image.rows /omp_get_num_threads()) * threadId;
    int endIteration = initIteration + (int)(image.rows / omp_get_num_threads());
    // Loop image rows
    for (int i = initIteration; i < endIteration; i++)
    {
        // Loop image cols
        for (int j = 0; j < image.cols; j++)
        {
            int newPixel[] = {0, 0, 0};

            // Loop kernel rows
            for (int ki = inicio; ki < final; ki++)
            {
                // Loop kernel cols
                for (int kj = inicio; kj < final; kj++)
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
                        newPixel[0] += (kernel[-ki + offset][-kj + offset] * B);
                        newPixel[1] += (kernel[-ki + offset][-kj + offset] * G);
                        newPixel[2] += (kernel[-ki + offset][-kj + offset] * R);
                    }
                }
            }

            //  Normalize pixel and bound it
            for (int k = 0; k < 3; k++)
            {
                newPixel[k] /= kernel_total;
                newPixel[k] = max(newPixel[k], 0);
                newPixel[k] = min(newPixel[k], 255);
            }

            // Update new image pixel
            newImage.at<Vec3b>(i, j)[0] = newPixel[0];
            newImage.at<Vec3b>(i, j)[1] = newPixel[1];
            newImage.at<Vec3b>(i, j)[2] = newPixel[2];
        }
    }
    return NULL;
}

void get_kernel_info(int filter)
{
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
}

int main(int argc, char **argv)
{
    // Check number of arguments
    if (argc < 4)
    {
        cout << "Ingrese todos los argumentos necesarios para ejecutar el proceso" << endl;
        return -1;
    }

    // Get the arguments
    string path_image = argv[1];
    string path_save = argv[2];

    n_threads = atoi(argv[3]);
    if (n_threads == 0)
    {
        cout << "El argumento de número de hilos es invalido" << endl;
        return -1;
    }
    int filter = 0;
    if (argc > 4)
    {
        filter = atoi(argv[4]);
        if (filter == 0)
        {
            cout << "El argumento de filtro es invalido" << endl;
            return -1;
        }
    }

    get_kernel_info(filter);

    // Read the image
    image = imread(path_image, IMREAD_COLOR); // Read the file
    if (!image.data)
    {
        cout << "Could not open or find the image" << endl;
        return -1;
    }

   
    newImage = Mat(image.rows, image.cols, CV_8UC3, Scalar(0, 0, 0));

    // Get initial time
    struct timeval tval_before, tval_after, tval_result;
    gettimeofday(&tval_before, NULL);

    // Launch threads
    #pragma omp parallel num_threads(n_threads)
    {
            int ID = omp_get_thread_num();
            filterImage(ID);
     }

    
    // Calculate time
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);

    // Show results
    printf("Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

    // imshow("Original", image);
    // imshow("Modificado", newImage);

    // Write the new image
    // imwrite(path_save, newImage);
    
    cout<<"cantidad de hilos "<< n_threads<<endl;
    cout<<"numero de filtro "<< filter<< endl;
    cout<<"imagen "<< path_image <<endl<<endl;
    if (!imwrite(path_save, newImage))
    {
        cout << "Could not save the image" << endl;
        return -1;
    }

    waitKey(0);
    return 0;
}