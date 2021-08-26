#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <pthread.h>
#include <sys/time.h>
#include <string>
#include <math.h>
#include <string>
#include <iostream>
#include <filesystem>

using namespace cv;
using namespace std;

Mat image, newImage;
int n_threads, block_size, new_block_size;
double RGBtoXYZ_RtoX[256];
double RGBtoXYZ_GtoX[256];
double RGBtoXYZ_BtoX[256];
double RGBtoXYZ_RtoY[256];
double RGBtoXYZ_GtoY[256];
double RGBtoXYZ_BtoY[256];
double RGBtoXYZ_RtoZ[256];
double RGBtoXYZ_GtoZ[256];
double RGBtoXYZ_BtoZ[256];
vector<string> images;

Vec3b RGBtoLAB(int R, int G, int B){
    //RGBtoXYZ
    double x = RGBtoXYZ_RtoX[R] + RGBtoXYZ_GtoX[G] + RGBtoXYZ_BtoX[B];
    double y = RGBtoXYZ_RtoY[R] + RGBtoXYZ_GtoY[G] + RGBtoXYZ_BtoY[B];
    double z = RGBtoXYZ_RtoZ[R] + RGBtoXYZ_GtoZ[G] + RGBtoXYZ_BtoZ[B];

    if (x > 0.008856)
    {
        x = cbrt(x);
    }
    else
    {
        x = (7.787 * x) + 0.13793103448275862;
    }

    if (y > 0.008856)
    {
        y = cbrt(y);
    }
    else
    {
        y = (7.787 * y) + 0.13793103448275862;
    }

    if (z > 0.008856)
    {
        z = cbrt(z);
    }
    else
    {
        z = (7.787 * z) + 0.13793103448275862;
    }

    u_char L = (116 * y) - 16;
    u_char a = 500 * (x - y);
    u_char b = 200 * (y - z);

    return {L, a, b};
}

void preload() {
    for(int i = 0; i < 256; i++){
        double r = (double)i/255;
    
        if (r > 0.04045 ){
            r = pow((r+0.055)/1.055 ,  2.4);
        }else{
            r = r/12.92;
        }
    
        r = r * 100;
    
        double ref_X =  95.047;
        double ref_Y = 100.000;
        double ref_Z = 108.883;
    
        RGBtoXYZ_RtoX[i] = r * 0.4124/ref_X;
        RGBtoXYZ_GtoX[i] = r * 0.3576/ref_X;
        RGBtoXYZ_BtoX[i] = r * 0.1805/ref_X;
        RGBtoXYZ_RtoY[i] = r * 0.2126/ref_Y;
        RGBtoXYZ_GtoY[i] = r * 0.7152/ref_Y;
        RGBtoXYZ_BtoY[i] = r * 0.0722/ref_Y;
        RGBtoXYZ_RtoZ[i] = r * 0.0193/ref_Z;
        RGBtoXYZ_GtoZ[i] = r * 0.1192/ref_Z;
        RGBtoXYZ_BtoZ[i] = r * 0.9505/ref_Z;
    }

    string path = "./imagenes";
    for (const auto & entry : filesystem::directory_iterator(path)){
        images.push_back(entry.path());
    }
}

string getClosestColor(int R, int G, int B){
    double minDiff = 10000;
    string closest;
    Vec3b lab1 = RGBtoLAB(R,G,B);

    for (int i = 0; i < images.size(); i++)
    {
        size_t lastindex = images[i].find_last_of(".") - 11; 
        string color = images[i].substr(11, lastindex); 

        int cr = stoul(color.substr(0,2), NULL, 16);
        int cg = stoul(color.substr(2,2), NULL, 16);
        int cb = stoul(color.substr(4,2), NULL, 16);
        
        Vec3b lab2 = RGBtoLAB(cr,cg,cb);
        
        double diff = sqrt( pow(lab1[0] - lab2[0], 2) + pow(lab1[1] - lab2[1], 2) + pow(lab1[2] - lab2[2], 2) );
        if (diff < minDiff)
        {
            minDiff = diff;
            closest = images[i];
        }
    }
    return closest;
}

void *processImage(void *arg)
{
    // Get iteration boundaries
    int threadId = *(int *)arg;
    int initIteration = (int)(image.rows / n_threads) * threadId;
    int endIteration = initIteration + (int)(image.rows / n_threads);
    int offset = new_block_size - block_size;
    // Loop image rows
    for (int i = initIteration/ block_size; i < endIteration / block_size; i++)
    {
        // Loop image cols
        for (int j = 0; j < image.cols / block_size; j++)
        {
            int newPixel[] = {0, 0, 0};

            // Loop block of pixels to mix
            for (int bi = 0; bi < block_size; bi++)
            {
                for (int bj = 0; bj < block_size; bj++)
                {
                    // Get image pixel RGB
                    Vec3b pixel = image.at<Vec3b>((i * block_size) + bi, (j * block_size) + bj);
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
                newPixel[k] /= pow(block_size, 2);
            }


            // Get the closest color
            string filename = getClosestColor((int)newPixel[2],(int)newPixel[1],(int)newPixel[0]);
            // Read the image for this "pixel"
            Mat img = imread(filename, IMREAD_COLOR); // Read the file
            if (!img.data)
            {
                cout << "Could not open or find the image " << filename << endl;
                exit(0);
            }
            
            // Crop the image to be a square without distorting it
            int init_i = max((img.rows / 2) - (new_block_size / 2), 0);
            int init_j = max((img.cols / 2) - (new_block_size / 2), 0);

            // Update new image pixels with the "pixel" image
            for (int ni = 0; ni < new_block_size; ni++)
            {
                for (int nj = 0; nj < new_block_size; nj++)
                {
                    newImage.at<Vec3b>((i * new_block_size) + ni, (j * new_block_size) + nj) = img.at<Vec3b>(init_i + ni, init_j + nj);
                }
            }

        }
    }
    return NULL;
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
    block_size = 1;
    if (argc > 4)
    {
        block_size = atoi(argv[4]);
        if (block_size == 0)
        {
            cout << "El argumento de tamaño es invalido" << endl;
            return -1;
        }
    }
    new_block_size = 10;
    if (argc > 5)
    {
        new_block_size = atoi(argv[5]);
        if (new_block_size == 0)
        {
            cout << "El argumento de tamaño nuevo es invalido" << endl;
            return -1;
        }
    }

    preload();

    // Read the image
    image = imread(path_image, IMREAD_COLOR); // Read the file
    if (!image.data)
    {
        cout << "Could not open or find the image" << endl;
        return -1;
    }

    int threadId[n_threads], *retval;
    pthread_t threads[n_threads];
    newImage = Mat((image.rows / block_size) * new_block_size, (image.cols / block_size) * new_block_size, CV_8UC3, Scalar(0, 0, 0));

    // Get initial time
    struct timeval tval_before, tval_after, tval_result;
    gettimeofday(&tval_before, NULL);

    // Launch threads
    for (int i = 0; i < n_threads; i++)
    {
        threadId[i] = i;
        pthread_create(&threads[i], NULL, processImage, &threadId[i]);
    }

    // wait for the threads to finish
    for (int i = 0; i < n_threads; i++)
    {
        pthread_join(threads[i], (void **)&retval);
    }

    // Calculate time
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);

    // Show results
    printf("Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
    
    // Write the new image
    // imwrite(path_save, newImage);
    if (!imwrite(path_save, newImage))
    {
        cout << "Could not save the image" << endl;
        return -1;
    }

    waitKey(0);
    return 0;
}
