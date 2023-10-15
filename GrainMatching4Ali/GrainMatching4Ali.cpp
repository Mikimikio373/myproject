#define _CRT_SECURE_NO_WARNINGS
#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <Windows.h>
#include <direct.h>

#include "GrainMatching.hpp"


using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::to_string;


vector<string> label_center = { "centerX", "centerY", "flag" };
vector<string> label_dist = { "distX","distY","flag" };
string basepath = "IMAGE00_AREA-1/png";
string savepath = "GrainMatching_loop";
int minus;



int main(int argc, char* argv[])
{
    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq))      // 単位習得
        return 0;
    LARGE_INTEGER start, end;

    int size = 15;
    int nLabs;
    for (int i = 0; i < argc; i++)
    {
        cout << argv[i] << endl;
    }


    if (argc != 9)
    {
        cout << "argc = " << argc << endl;
        cout << "command line error, prease command \"basepath, FirstModule, FirstSensor, SecondModule, SecondSensor, oudirectry, option_yflip, minus\" " << endl;
        return 3;
    }

    string basepath = argv[1];
    string module1 = argv[2];
    string sensor1 = argv[3];
    string module2 = argv[4];
    string sensor2 = argv[5];
    string outdir = argv[6];
    int option = atoi(argv[7]);
    minus = (int)argv[8];
    string filepath1 = basepath + "\\Module" + module1 + "\\sensor-" + sensor1;
    string filepath2 = basepath + "\\Module" + module2 + "\\sensor-" + sensor2;
    
    //ずれ計算
    cv::Mat mat_ori1, mat_temp1, mat_gauss1, mat_thr1;
    cv::Mat mat_ori2, mat_temp2, mat_gauss2, mat_thr2;
    cv::Mat labels, stats, centroids;

    //基準画像の輝度重心計算
    string csvpath1 = filepath1 + "\\aff_data.csv";
    string csvpath2 = filepath2 + "\\aff_data.csv";
    string pngpath1 = filepath1 + "\\IMAGE00_AREA-1\\png\\L0_VX0000_VY0000_4_0.png";
    string pngpath2 = filepath2 + "\\IMAGE00_AREA-1\\png\\L0_VX0000_VY0000_4_0.png";
    if (std::filesystem::exists(csvpath1) == false) {
        cout << "there is not png file " << csvpath1 << endl;
        return -1;
    }
    if (std::filesystem::exists(csvpath1) == false) {
        cout << "there is not png file " << csvpath1 << endl;
        return -1;
    }
    if (std::filesystem::exists(pngpath1) == false) {
        cout << "there is not png file " << pngpath1 << endl;
        return -1;
    }
    if (std::filesystem::exists(pngpath2) == false) {
        cout << "there is not png file " << pngpath2 << endl;
        return -1;
    }

    string outdir1 = filepath1 + "\\GrainMatching";
    string outdir2 = filepath2 + "\\GrainMatching";
    int tmp;
    if (std::filesystem::exists(outdir1) == false) { tmp = _mkdir(outdir1.c_str()); }
    if (std::filesystem::exists(outdir2) == false) { tmp = _mkdir(outdir2.c_str()); }

    vector<vector<string>> csvdata1 = csv2vector(csvpath1);
    vector<vector<string>> csvdata2 = csv2vector(csvpath2);

    
    mat_ori1 = cv::imread(pngpath1, 0);
    gaussianfilterminus(mat_ori1, mat_gauss1, size, 0, minus);
    cv::threshold(mat_gauss1, mat_thr1, 1, 255, cv::THRESH_BINARY);
    nLabs = cv::connectedComponentsWithStats(mat_thr1, labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT); //ラベリング
    vector<grain> CenterOfBrightness1;
    string outcsvpath1 = outdir1 + "\\CenterOfBrightness";
    CalculateBrightnessCenter(mat_gauss1, labels, stats, nLabs, CenterOfBrightness1, outcsvpath1);
    cout << outcsvpath1 << " ended" << endl;

    double a1 = std::stod(csvdata1[1][3]);
    double b1 = std::stod(csvdata1[1][4]);
    double c1 = std::stod(csvdata1[1][5]);
    double d1 = std::stod(csvdata1[1][6]);
    cout << a1 << ", " << b1 << ", " << c1 << ", " << d1 << endl;
    if (option == 0)
    {
        change_coordinate(CenterOfBrightness1, a1, b1, c1, d1);
    }
    else if (option == 1) {
        change_coordinate_yflip(CenterOfBrightness1, a1, b1, c1, d1);
    }
    else if (option == 2) {
        change_coordinate(CenterOfBrightness1, a1, b1, c1, d1);
    }
    else
    {
        cout << "option must be 0 or 1 or 2!" << endl;
        return -2;
    }
    
    grain2csv(outdir1 + "\\Center_stage", CenterOfBrightness1, label_center);

    mat_ori2 = cv::imread(pngpath2, 0);
    gaussianfilterminus(mat_ori2, mat_gauss2, size, 0, minus);
    cv::threshold(mat_gauss2, mat_thr2, 1, 255, cv::THRESH_BINARY);
    nLabs = cv::connectedComponentsWithStats(mat_thr2, labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT); //ラベリング
    vector<grain> CenterOfBrightness2;
    string outcsvpath2 = outdir2 + "\\CenterOfBrightness";
    CalculateBrightnessCenter(mat_gauss2, labels, stats, nLabs, CenterOfBrightness2, outcsvpath2);
    cout << outcsvpath2 << " ended" << endl;

    double a2 = std::stod(csvdata2[1][3]);
    double b2 = std::stod(csvdata2[1][4]);
    double c2 = std::stod(csvdata2[1][5]);
    double d2 = std::stod(csvdata2[1][6]);
    cout << a2 << ", " << b2 << ", " << c2 << ", " << d2 << endl;
    if (option == 0)
    {
        change_coordinate(CenterOfBrightness2, a2, b2, c2, d2);
    }
    if (option == 1)
    {
        change_coordinate(CenterOfBrightness2, a2, b2, c2, d2);
    }
    if (option == 2)
    {
        change_coordinate_yflip(CenterOfBrightness2, a2, b2, c2, d2);
    }
    grain2csv(outdir2 + "\\Center_stage", CenterOfBrightness2, label_center);

    vector<grain> distGrain;
    QueryPerformanceCounter(&start);
    GrainMatching(CenterOfBrightness1, CenterOfBrightness2, distGrain, 0, 0, 0, 0);
    QueryPerformanceCounter(&end);
    std::cout << "culculate = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";
    //csvpath = "R:\\minami\\20221017_forGrainMaching0.025\\dist_ver-1vs2";
    string csvpath = outdir + "\\dist_" + module1 + "-" + sensor1 + "vs" + module2 + "-" + sensor2;
    QueryPerformanceCounter(&start);
    grain2csv(csvpath, distGrain, label_dist);
    QueryPerformanceCounter(&end);
    std::cout << "output = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";
   
    return 0;
}

