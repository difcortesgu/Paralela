#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>


using namespace cv;
using namespace std;


int main(int argc, char **argv)
{
    // if (argc != 2)
    // {
    //     cout << " Usage: display_image ImageToLoadAndDisplay" << endl;
    //     return -1;
    // }
    
    // DETECCION DE BORDES
    // vector<vector<int>> kernel = {
    //     {0, 1, 0},
    //     {1, -4, 1},
    //     {0, 1, 0}
    // };

    // REPUJADO
    // vector<vector<int>> kernel = {
    //     {-2, -1, 0},
    //     {-1, 1, 1},
    //     {0, 1, 2}
    // };

    // ENFOCADO
    // vector<vector<int>> kernel = {
    //     {0, -1, 0},
    //     {-1, 5, -1},
    //     {0, -1, 0}
    // };

    // ENFOCADO
    // vector<vector<int>> kernel = {
    //     {0, 0, 0, 0, 0},
    //     {0, 0, -1, 0, 0},
    //     {0, -1, 5, -1, 0},
    //     {0, 0, -1, 0, 0},
    //     {0, 0, 0, 0, 0}
    // };


    // DESENFOCADO 3x3
    // vector<vector<int>> kernel = {
    //     {1, 1, 1},
    //     {1, 1, 1},
    //     {1, 1, 1}
    // };

    // DESENFOCADO 5x5
    vector<vector<int>> kernel = {
        {0, 0, 0, 0, 0},
        {0, 1, 1, 1, 0},
        {0, 1, 1, 1, 0},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0}
    };


    // IDENTIDAD
    // vector<vector<int>> kernel = {
    //     {0, 0, 0},
    //     {0, 1, 0},
    //     {0, 0, 0}
    // };

    Mat image, newImage;

    image = imread("lenna.png", IMREAD_COLOR); // Read the file
    if (!image.data) // Check for invalid input
    {
        cout << "Could not open or find the image" << endl;
        return -1;
    }

    
    newImage = Mat(image.rows, image.cols, CV_8UC3, Scalar(0,0,0));
    
    int kernel_total = 0;
    for (int i = 0; i < kernel.size(); i++)
    {
        for (int j = 0; j < kernel.size(); j++)
        {
            kernel_total += kernel[i][j];
        }
    }   
    int inicio = -(int)(kernel.size() / 2);
    int final = (int)(kernel.size() / 2) + 1;
    int offset = (int)(kernel.size() / 2);

    cout<< inicio << ", " << final << endl;
    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            int newPixel[] = {0,0,0};
            

            // pixeles vecinos
            for (int ki = inicio; ki < final; ki++)
            {
                for (int kj = inicio; kj < final; kj++)
                {
                    if( i + ki >= 0 && i + ki < image.rows && j + kj >= 0 && j + kj < image.cols ){
                        
                        Vec3b pixel = image.at<Vec3b>(i + ki, j + kj);

                        int B = (int)pixel[0];
                        int G = (int)pixel[1];
                        int R = (int)pixel[2];

                        newPixel[0] += (kernel[-ki + offset][-kj + offset] * B);
                        newPixel[1] += (kernel[-ki + offset][-kj + offset] * G);
                        newPixel[2] += (kernel[-ki + offset][-kj + offset] * R);
                    }
                }
            }

            for (int k = 0; k < 3; k++){
                newPixel[k] /= kernel_total;
                newPixel[k]= max(newPixel[k], 0);
                newPixel[k]= min(newPixel[k], 255);
            }

            newImage.at<Vec3b>(i,j)[0] = newPixel[0];
            newImage.at<Vec3b>(i,j)[1] = newPixel[1];
            newImage.at<Vec3b>(i,j)[2] = newPixel[2];
        }
    }
    Mat dst;
    Mat kernel1 = (Mat_<float>(3, 3) << 0, 0, 0,
                                        0, 1, 0,
                                        0, 0, 0)/1;

    filter2D(image, dst, image.depth(), kernel1);
    

    imshow("Display Filter2D", dst);
    imshow("Original", image); // Show our image inside it.
    imshow("Modificado", newImage);// Show our image inside it.

    waitKey(0); // Wait for a keystroke in the window
    return 0;
}