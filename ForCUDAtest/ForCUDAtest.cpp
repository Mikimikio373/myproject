#include <opencv2/opencv.hpp>
//#include <opencv2/core/cuda.hpp>
//#include <opencv2/core/cuda.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <windows.h>


using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::to_string;




int main()
{
    cv::Mat img, gauss, dist;
    string filename = "R:\\minami\\20220704_minamichem\\003\\IMAGE00_AREA-1\\png\\L0_VX0004_VY0014\\L0_VX0004_VY0014_6.png";
    img = cv::imread(filename, 0);
    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq))      // íPà èKìæ
        return -3;
    LARGE_INTEGER start, end;
    QueryPerformanceCounter(&start);

    cv::GaussianBlur(img, gauss, cv::Size(5,5), 2);
    cv::subtract(img, gauss, dist);

    QueryPerformanceCounter(&end);
    std::cout << "duration = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";

    //cv::imshow("window", img);
    //cv::waitKey(0);

    return 0;
}