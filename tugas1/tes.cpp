#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
int main(){
    Mat img = imread("/home/rezaafzaal/Downloads/ijo.jpeg");

    imshow("Hijau",img);

    waitkey(0);



    return 0;
}