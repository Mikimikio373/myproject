#define _CRT_SECURE_NO_WARNINGS
#include <opencv2/opencv.hpp>
#include <iostream>
#include <Math.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <Windows.h>
#include <thread>

#include "GrainMatching4Aff_second.hpp"

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::to_string;


vector<string> label_center = { "centerX", "centerY", "flag" };
vector<string> label_dist = { "distX","distY","flag" };
string basepath = "IMAGE00_AREA-1/png";
string savepath = "GrainMatching_loop";
string aff_path = "affdata_surf_first.csv";



int main(int argc, char* argv[])
{
    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq))      // 単位習得
        return 0;
    LARGE_INTEGER start, end;
    for (int i = 0; i < argc; i++)
    {
        cout << argv[i] << endl;
    }


    if (argc != 8)
    {
        cout << "argc = " << argc << endl;
        cout << "command line error, please filepath NPcture shift_x shift_Y pitch_X pitch_y minus" << endl;
        return 3;
    }


    std::string working_dir = argv[1];
    std::filesystem::current_path(working_dir);
    std::filesystem::path path = std::filesystem::current_path();
    cout << "current_path: " << path << endl;
    int NPcture = atoi(argv[2]) - 1;
    int shift_X = atoi(argv[3]);
    int shift_Y = atoi(argv[4]);
    double pitch_x = atof(argv[5]);
    double pitch_y = atof(argv[6]);
    int minus = atoi(argv[7]);
    int base_brightness = 202;

    int layer = 0;
    int vx = 0;
    int vy = 0;
    if (std::filesystem::exists(savepath) == false)
    {
        std::filesystem::create_directory(savepath);
    }
    double a, b, c, d;
    get_affparam(aff_path, a, b, c, d);
    double in_a, in_b, in_c, in_d;
    calc_inaff(a, b, c, d, in_a, in_b, in_c, in_d);

    string layer_s = std::to_string(layer);
    string vx_s = int2string_0set(vx, 4);
    string vy_s = int2string_0set(vy, 4);
    int nLabs;
    cv::Mat labels, stats, centroids;
    vector<double> alldata;
    vector<cv::Mat> vec_ori, vec_gauss, vec_thr, vec_bitwise;
    vector<vector<grain>> CenterOfBrightnessAll;
    vector<vector<double>> MatchingGrain;


    int size = 15;

    //ずれ計算
    cv::Mat mat_ori1, mat_temp1, mat_gauss1, mat_thr1;
    cv::Mat mat_ori2, mat_temp2, mat_gauss2, mat_thr2;

    for (int i = 0; i < shift_Y; i++)
    {
        //基準画像の輝度重心計算
        int startvx = 0;
        int startvy = 0;
        string s_startvx = int2string_0set(startvx, 4);
        string s_startvy = int2string_0set(startvy, 4);

        string pngname1 = "L0_VX" + s_startvx + "_VY" + s_startvy;
        string pngpath1 = basepath + "/" + pngname1 + "_" + std::to_string(NPcture) + "_" + std::to_string(i) + ".png";

        if (std::filesystem::exists(pngpath1) == false)
        {
            cout << "there is not png file " << pngpath1 << endl;
            return -1;
        }
        mat_ori1 = cv::imread(pngpath1, 0);
        gaussianfilterminus(mat_ori1, mat_gauss1, size, 0, minus);
        cv::threshold(mat_gauss1, mat_thr1, 1, 255, cv::THRESH_BINARY);
        nLabs = cv::connectedComponentsWithStats(mat_thr1, labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT); //ラベリング
        cout << nLabs << endl;
        if (nLabs < 4700)
        {
            while (nLabs < 4700) {
                minus--;
                gaussianfilterminus(mat_ori1, mat_gauss1, size, 0, minus);
                cv::threshold(mat_gauss1, mat_thr1, 1, 255, cv::THRESH_BINARY);
                nLabs = cv::connectedComponentsWithStats(mat_thr1, labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT); //ラベリング
                cout << nLabs << endl;
            }
        }if (nLabs > 6000)
        {
            while (nLabs > 6000) {
                minus++;
                gaussianfilterminus(mat_ori1, mat_gauss1, size, 0, minus);
                cv::threshold(mat_gauss1, mat_thr1, 1, 255, cv::THRESH_BINARY);
                nLabs = cv::connectedComponentsWithStats(mat_thr1, labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT); //ラベリング
                cout << nLabs << endl;
            }
        }
        vector<grain> CenterOfBrightness1;
        string csvpath = savepath + "/centerVX" + s_startvx + "_VY" + s_startvy + "_" + std::to_string(i);
        CalculateBrightnessCenter(mat_gauss1, labels, stats, nLabs, CenterOfBrightness1, csvpath);
        cout << csvpath << " ended" << endl;

        //二重ループでstage stepを変えて輝度重心計算→GrainMatching
        for (int j = 0; j < shift_X; j++)
        {
            if (i == 0 && j == 0) { continue; }
            int vx = j;
            int vy = i;
            string s_vx = int2string_0set(vx, 4);
            string s_vy = int2string_0set(vy, 4);
            string pngname2 = "L0_VX" + s_vx + "_VY" + s_vy;
            string pngpath2 = basepath + "/" + pngname2 + "_" + std::to_string(NPcture) + ".png";
            if (std::filesystem::exists(pngpath2) == false)
            {
                cout << "there is not png file" << pngpath2 << endl;
                return -1;
            }
            mat_ori2 = cv::imread(pngpath2, 0);
            gaussianfilterminus(mat_ori2, mat_gauss2, size, 0, minus);
            cv::threshold(mat_gauss2, mat_thr2, 1, 255, cv::THRESH_BINARY);
            nLabs = cv::connectedComponentsWithStats(mat_thr2, labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT); //ラベリング
            if (nLabs < 4700)
            {
                while (nLabs < 4700) {
                    minus--;
                    gaussianfilterminus(mat_ori2, mat_gauss2, size, 0, minus);
                    cv::threshold(mat_gauss2, mat_thr2, 1, 255, cv::THRESH_BINARY);
                    nLabs = cv::connectedComponentsWithStats(mat_thr2, labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT); //ラベリング
                }
            }if (nLabs > 6000)
            {
                while (nLabs > 6000) {
                    minus++;
                    gaussianfilterminus(mat_ori2, mat_gauss2, size, 0, minus);
                    cv::threshold(mat_gauss2, mat_thr2, 1, 255, cv::THRESH_BINARY);
                    nLabs = cv::connectedComponentsWithStats(mat_thr2, labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT); //ラベリング
                }
            }
            vector<grain> CenterOfBrightness2;
            csvpath = savepath + "/centerVX" + s_vx + "_VY" + s_vy;
            CalculateBrightnessCenter(mat_gauss2, labels, stats, nLabs, CenterOfBrightness2, csvpath);
            cout << csvpath << " ended" << endl;

            vector<grain> distGrain;
            double centerX = vx * pitch_x * in_a + vy * pitch_y * in_b;
            double centerY = vx * pitch_x * in_c + vy * pitch_y * in_d;
            QueryPerformanceCounter(&start);
            GrainMatching(CenterOfBrightness1, CenterOfBrightness2, distGrain, 1, centerX, centerY, 30);
            QueryPerformanceCounter(&end);
            std::cout << "culculate = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";
            csvpath = savepath + "/dist_" + s_startvx + s_startvy + "vs" + s_vx + s_vy;

            QueryPerformanceCounter(&start);
            grain2csv(csvpath, distGrain, label_dist);
            QueryPerformanceCounter(&end);
            std::cout << "output = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";
            cout << csvpath << " ended" << endl;
        }
    }

    return 0;
}