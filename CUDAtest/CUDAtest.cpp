#include <iostream>
#include <stdio.h>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudawarping.hpp>
#include <Windows.h>


std::string int2string_0set(int input, int digit);

int main()
{
    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq))      // 単位習得
        return 0;
    LARGE_INTEGER start, end;

    //std::string filepath1 = "Q:\\minami\\20220429_suganami\\006\\cpudft.csv";
    //std::ofstream ofs1;
    //ofs1.open(filepath1, std::ios::out);
    //for (int L = 0; L < 2; L++)
    //{
    //    for (int vx = 0; vx < 18; vx++)
    //    {
    //        for (int vy = 0; vy < 37; vy++)
    //        {
    //            std::string pngname = "L" + std::to_string(L) + "_VX" + int2string_0set(vx, 4) + "_VY" + int2string_0set(vy, 4);
    //            std::string filename = "R:\\minami\\20220704_minamichem\\003\\IMAGE00_AREA-1\\png\\"+pngname+"\\"+pngname+"_11.png";
    //            cv::Mat img = cv::imread(filename, 0);
    //            cv::Mat dst;
    //            cv::Mat furimg;
    //            cv::Mat ch;
    //            cv::Mat RealIamginary[] = { cv::Mat_<float>(img), cv::Mat::zeros(img.size(), CV_32F) };
    //            cv::merge(RealIamginary, 2, furimg);
    //            QueryPerformanceCounter(&start);
    //            cv::dft(furimg, furimg);
    //            cv::idft(furimg, dst);
    //            QueryPerformanceCounter(&end);
    //            double cpudft = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
    //            ofs1 << cpudft << std::endl;
    //            std::cout << pngname + " ended" << std::endl;
    //        }
    //    }
    //}
    //ofs1.close();

    //std::string filepath2 = "Q:\\minami\\20220429_suganami\\006\\cpugauss.csv";
    //std::ofstream ofs2;
    //ofs2.open(filepath2, std::ios::out);
    //for (int L = 0; L < 2; L++)
    //{
    //    for (int vx = 0; vx < 18; vx++)
    //    {
    //        for (int vy = 0; vy < 37; vy++)
    //        {
    //            std::string pngname = "L" + std::to_string(L) + "_VX" + int2string_0set(vx, 4) + "_VY" + int2string_0set(vy, 4);
    //            std::string filename = "R:\\minami\\20220704_minamichem\\003\\IMAGE00_AREA-1\\png\\" + pngname + "\\" + pngname + "_11.png";
    //            cv::Mat img = cv::imread(filename, 0);
    //            cv::Mat dst, img_gauss;
    //            QueryPerformanceCounter(&start);
    //            cv::GaussianBlur(img, img_gauss, cv::Size(5, 5), 2);
    //            cv::subtract(img, img_gauss, dst);
    //            QueryPerformanceCounter(&end);
    //            double cpugauss = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
    //            ofs2 << cpugauss << std::endl;
    //            std::cout << pngname + " ended" << std::endl;
    //        }
    //    }
    //}
    //ofs2.close();

    //std::string filepath3 = "Q:\\minami\\20220429_suganami\\006\\gpudft.csv";
    //std::ofstream ofs3;
    //ofs3.open(filepath3, std::ios::out);
    //for (int L = 0; L < 2; L++)
    //{
    //    for (int vx = 0; vx < 18; vx++)
    //    {
    //        for (int vy = 0; vy < 37; vy++)
    //        {
    //            LARGE_INTEGER n1, n2, n3, n4;
    //            std::string pngname = "L" + std::to_string(L) + "_VX" + int2string_0set(vx, 4) + "_VY" + int2string_0set(vy, 4);
    //            std::string filename = "R:\\minami\\20220704_minamichem\\003\\IMAGE00_AREA-1\\png\\" + pngname + "\\" + pngname + "_11.png";
    //            cv::Mat img = cv::imread(filename, 0);
    //            cv::Mat tmp;
    //            cv::cuda::GpuMat img_gpu, dst_gpu, tmp_gpu, padded, padded2;
    //            int m = cv::getOptimalDFTSize(img.rows);
    //            int n = cv::getOptimalDFTSize(img.cols);
    //            //std::cout << m << n << std::endl;
    //            // 入力画像を中央に置き、周囲は0で埋める
    //            img_gpu.upload(img);
    //            img_gpu.convertTo(img_gpu, CV_32FC2, 1.0 / 255);
    //            cv::cuda::copyMakeBorder(img_gpu, padded, 0, m - img_gpu.rows, 0, n - img_gpu.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
    //            QueryPerformanceCounter(&n1);
    //            cv::cuda::dft(padded, tmp_gpu, cv::Size(m, n));
    //            QueryPerformanceCounter(&n2);
    //            tmp_gpu.download(tmp);
    //            m = cv::getOptimalDFTSize(tmp.rows);
    //            n = cv::getOptimalDFTSize(tmp.cols);
    //            //std::cout << m << n << std::endl;
    //            std::vector<cv::cuda::GpuMat> single_tmp;
    //            cv::cuda::split(tmp_gpu, single_tmp);
    //            cv::cuda::resize(single_tmp.at(0), single_tmp.at(0), cv::Size(m, n));
    //            cv::cuda::resize(single_tmp.at(1), single_tmp.at(1), cv::Size(m, n));
    //            cv::cuda::merge(single_tmp, padded2);
    //            QueryPerformanceCounter(&n3);
    //            cv::cuda::dft(padded2, dst_gpu, cv::Size(3, 3));
    //            QueryPerformanceCounter(&n4);
    //            double gpudft1 = (double)(n2.QuadPart - n1.QuadPart) / freq.QuadPart;
    //            double gpudft2 = (double)(n4.QuadPart - n3.QuadPart) / freq.QuadPart;
    //            ofs3 << gpudft1 << "," << gpudft2 << std::endl;
    //            std::cout << pngname + " ended" << std::endl;
    //        }
    //    }
    //}
    //ofs3.close();

    std::string filepath4 = "Q:\\minami\\20220429_suganami\\006\\gpugauss.csv";
    std::ofstream ofs4;
    ofs4.open(filepath4, std::ios::out);
    for (int L = 0; L < 2; L++)
    {
        for (int vx = 0; vx < 18; vx++)
        {
            for (int vy = 0; vy < 37; vy++)
            {
                std::string pngname = "L" + std::to_string(L) + "_VX" + int2string_0set(vx, 4) + "_VY" + int2string_0set(vy, 4);
                std::string filename = "R:\\minami\\20220704_minamichem\\003\\IMAGE00_AREA-1\\png\\" + pngname + "\\" + pngname + "_11.png";
                cv::Mat img = cv::imread(filename, 0);
                cv::cuda::GpuMat img_gpu, dst_gpu, gauss_gpu;
                img_gpu.upload(img);
                QueryPerformanceCounter(&start);
                cv::Ptr<cv::cuda::Filter> sobelobj = cv::cuda::createGaussianFilter(img_gpu.type(), gauss_gpu.type(), cv::Size(5, 5), 1.2, 0);
                sobelobj->apply(img_gpu, gauss_gpu);
                cv::cuda::subtract(img_gpu, gauss_gpu, dst_gpu);
                QueryPerformanceCounter(&end);
                double gpugauss = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
                ofs4 << gpugauss << std::endl;
                std::cout << pngname + " ended" << std::endl;
            }
        }
    }
    ofs4.close();
    
}

std::string int2string_0set(int input, int digit) {
    std::ostringstream ss;
    ss << std::setw(digit) << std::setfill('0') << input;
    std::string s(ss.str());

    return s;
}