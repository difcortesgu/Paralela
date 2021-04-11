#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;


int main(int argc, char **argv)
{
    // if (argc != 2)
    // {
    //     cout << " Usage: display_image ImageToLoadAndDisplay" << endl;
    //     return -1;
    // }
    
    // int kernel[3][3] = {
    //     {0, 1, 0},
    //     {1, -4, 1},
    //     {0, 1, 0}
    // };

    // int kernel[3][3] = {
    //     {0, -1, 0},
    //     {-1, 5, -1},
    //     {0, -1, 0}
    // };
    
    int kernel[3][3] = {
        {1, 1, 1},
        {1, 1, 1},
        {1, 1, 1}
    };

    // int kernel[3][3] = {
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
    
    int kernel_size = 9;//kernel.size()() * kernel.size()[0];
    for (size_t i = 0; i < image.rows; i++)
    {
        for (size_t j = 0; j < image.cols; j++)
        {
            int newPixel[] = {0,0,0};

            // pixeles vecinos
            for (int ki = -1; ki < 2; ki++)
            {
                for (int kj = -1; kj < 2; kj++)
                {

                    if( i + ki >= 0 && i + ki < image.rows && j + kj >= 0 && j + kj < image.cols ){
                        Vec3b pixel = image.at<Vec3b>(i + ki, j + kj);

                        int B = (int)pixel[0];
                        int G = (int)pixel[1];
                        int R = (int)pixel[2];

                        newPixel[0] += (kernel[ki + 1][kj + 1] * B);
                        newPixel[1] += (kernel[ki + 1][kj + 1] * G);
                        newPixel[2] += (kernel[ki + 1][kj + 1] * R);
                    }



                }
            }

            newPixel[0] /= kernel_size;
            newPixel[1] /= kernel_size;
            newPixel[2] /= kernel_size;

            newImage.at<Vec3b>(i,j)[0] = newPixel[0] % 256;
            newImage.at<Vec3b>(i,j)[1] = newPixel[1] % 256;
            newImage.at<Vec3b>(i,j)[2] = newPixel[2] % 256;
            
            // Vec3b pixel = image.at<Vec3b>(i, j);
            // cout << newPixel[0] << newPixel[1] << newPixel[0] << endl;
            // cout << pixel[0] << pixel[1] << pixel[0] << endl;
        }
    }


    Mat dst;
    Mat kernel1 = (Mat_<float>(3, 3) << 1/9, 1/9, 1/9,
                                        1/9, 1/9, 1/9,
                                        1/9, 1/9, 1/9);

    filter2D(image, dst, image.depth(), kernel1);

    imshow("Display Filter2D", dst);
    
    for (size_t i = 0; i < 5; i++)
    {
        for (size_t j = 0; j <  5; j++)
        {
            uchar newPixel[] = {0,0,0};
            Vec3b pixel = dst.at<Vec3b>(i, j);

                    uchar B = pixel[0];
                    uchar G = pixel[1];
                    uchar R = pixel[2];
            cout << "soy la imagen esperada" << " " << (int)pixel[0] <<" " << (int) pixel[1] << " " << (int)pixel[2] << endl;
        }
    }
    cout<<""<<endl;
    for (size_t i = 0; i < 5; i++)
    {
        for (size_t j = 0; j <  5; j++)
        {
            Vec3b pixel = image.at<Vec3b>(i, j);

                    uchar B = pixel[0];
                    uchar G = pixel[1];
                    uchar R = pixel[2];
            cout << "imagen original" << " " << (int)pixel[0] <<" " << (int) pixel[1] << " " << (int)pixel[2] << endl;
        }
    }
    cout<<""<<endl;

    for (size_t i = 0; i < 5; i++)
    {
        for (size_t j = 0; j <  5; j++)
        {
            Vec3b pixel = newImage.at<Vec3b>(i, j);

                    uchar B = pixel[0];
                    uchar G = pixel[1];
                    uchar R = pixel[2];
            cout << "imagen Modificada" << " " << (int)pixel[0] <<" " << (int) pixel[1] << " " << (int)pixel[2] << endl;
        }
    }
    cout<<""<<endl;


    imshow("Original", image);                    // Show our image inside it.
    imshow("Modificado", newImage);                    // Show our image inside it.

    waitKey(0); // Wait for a keystroke in the window
    return 0;
}