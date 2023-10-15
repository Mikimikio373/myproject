#define _CRT_SECURE_NO_WARNINGS
#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>

#include "GrainMatching.hpp"


using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::to_string;

vector<string> label_center = { "centerX", "centerY", "flag" };
vector<string> label_dist = { "distX","distY","flag" };




int main(int argc, char* argv[])
{
    for (int i = 0; i < argc; i++)
    {
        cout << argv[i] << endl;
    }


    if (argc != 7)
    {
        cout << "argc = " << argc << endl;
        cout << "command line error, please reference path, refarence picture number, compared path, num of picture, minus, option" << endl;
        return 3;
    }
    string currentpath = std::filesystem::current_path().string<char>();
    string ref_path = argv[1];
    int ref_num = atoi(argv[2]);
    string compare_path = argv[3];
    int npicture = atoi(argv[4]);
    int minus = atoi(argv[5]);
    int option = atoi(argv[6]);
    int mode = 1;
    string savepath = currentpath + "/GrainMatching4z-match";
    std::filesystem::create_directory(savepath);
    int nLabs;
    cv::Mat labels, stats, centroids;
    vector<double> alldata;
    vector<cv::Mat> vec_ori, vec_gauss, vec_thr, vec_bitwise;
    vector<vector<grain>> CenterOfBrightnessAll;
    vector<vector<double>> MatchingGrain;

    string csvpath1 = currentpath + "/aff_data.csv";
    string csvpath2 = compare_path + "/aff_data.csv";
    if (std::filesystem::exists(csvpath1) == false) {
        cout << "there is no file: " + csvpath1;
        return -1;
    }
    vector<vector<string>> csvdata1 = csv2vector(csvpath1);
    double a1 = std::stod(csvdata1[1][3]);
    double b1 = std::stod(csvdata1[1][4]);
    double c1 = std::stod(csvdata1[1][5]);
    double d1 = std::stod(csvdata1[1][6]);
    cout << a1 << ", " << b1 << ", " << c1 << ", " << d1 << endl;
    if (std::filesystem::exists(csvpath2) == false) {
        cout << "there is no file: " + csvpath2;
        return -1;
    }
    vector<vector<string>> csvdata2 = csv2vector(csvpath2);
    double a2 = std::stod(csvdata2[1][3]);
    double b2 = std::stod(csvdata2[1][4]);
    double c2 = std::stod(csvdata2[1][5]);
    double d2 = std::stod(csvdata2[1][6]);
    cout << a2 << ", " << b2 << ", " << c2 << ", " << d2 << endl;


    int size = 15;
    double sigma = 0;
    double filter_length = 0.15;
    double cut_pixel = 1;
    int width = 2048;
    int height = 1088;


    
    //基準画像計算
    cv::Mat ref_ori, ref_gauss, ref_thr;
    string ref_img_path = ref_path + "/" + ref_path + "_" + std::to_string(ref_num) + ".png";
    if (std::filesystem::exists(ref_img_path) == false){
        cout << "there is not png file: " << ref_img_path << " \nIs current directry IMAGE00_AREA-1/png ?" << endl;
        return -1;
    }
    ref_ori = cv::imread(ref_img_path, 0);
    gaussianfilterminus(ref_ori, ref_gauss, size, 0, minus);
    cv::threshold(ref_gauss, ref_thr, 1, 255, cv::THRESH_BINARY);
    nLabs = cv::connectedComponentsWithStats(ref_thr, labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT);
    vector<grain> ref_CenterOfBrightness;
    string ref_out_path = savepath + "/center_ref_" + ref_path;
    string ref_out_stage = savepath + "/center_ref_stage_" + ref_path;
    CalculateBrightnessCenter(ref_gauss, labels, stats, nLabs, ref_CenterOfBrightness, ref_out_path);

    if (option == 0)
    {
        change_coordinate(ref_CenterOfBrightness, a1, b1, c1, d1);
    }
    else if (option == 1) {
        change_coordinate_yflip(ref_CenterOfBrightness, a1, b1, c1, d1);
    }
    else if (option == 2) {
        change_coordinate(ref_CenterOfBrightness, a1, b1, c1, d1);
    }
    else
    {
        cout << "option must be 0 or 1 or 2!" << endl;
        return -2;
    }
    grain2csv(ref_out_stage, ref_CenterOfBrightness, label_center);



    //メモリのせ
    for (int num = 0; num < npicture; num++)
    {
        cv::Mat mat_ori, mat_temp, mat_gauss, mat_thr;
        string pngpath = compare_path + "/" + ref_path + "/" + ref_path + "_" + std::to_string(num) + ".png";
        if (std::filesystem::exists(pngpath) == false)
        {
            cout << "there is not png file: " << pngpath << " Is current directry IMAGE00_AREA-1/png ?" << endl;
            return -1;
        }
        mat_ori = cv::imread(pngpath, 0);
        vec_ori.push_back(mat_ori);
        gaussianfilterminus(mat_ori, mat_gauss, size, 0, minus);
        vec_gauss.push_back(mat_gauss);
        cv::threshold(mat_gauss, mat_thr, 1, 255, cv::THRESH_BINARY);
        vec_thr.push_back(mat_thr);
    }



    //輝度重心計算
    for (int num = 0; num < npicture; num++)
    {
        nLabs = cv::connectedComponentsWithStats(vec_thr[num], labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT); //ラベリング
        vector<grain> CenterOfBrightness;
        string csvpath = savepath + "/center_compare_" + ref_path + to_string(num);
        string csvpath_stage = savepath + "/center_compare_stage_" + ref_path + to_string(num);
        CalculateBrightnessCenter(vec_gauss[num], labels, stats, nLabs, CenterOfBrightness, csvpath);
        if (option == 0)
        {
            change_coordinate(CenterOfBrightness, a2, b2, c2, d2);
        }
        if (option == 1)
        {
            change_coordinate(CenterOfBrightness, a2, b2, c2, d2);
        }
        if (option == 2)
        {
            change_coordinate_yflip(CenterOfBrightness, a2, b2, c2, d2);
        }
        grain2csv(csvpath_stage, CenterOfBrightness, label_center);
        CenterOfBrightnessAll.push_back(CenterOfBrightness);
        cout << csvpath << " ended" << endl;

    }

    //重心距離計算 loop
    for (int i = 0; i < npicture; i++)
    {
        vector<grain> distGrain;
        string pos1 = int2string_0set(ref_num, 3);
        string pos2 = int2string_0set(i, 3);

        GrainMatching(ref_CenterOfBrightness, CenterOfBrightnessAll[i], distGrain, 0, 0, 0, 0);
        vector<grain> distGrain_cut;
        calc_max_bin(distGrain, distGrain_cut, 200, -2048, 2048, 100, -1088, 1088);
        string csvpath = savepath + "/dist_" + pos1 + "_" + pos2;
        if (mode == 0)
        {
            grain2csv(csvpath, distGrain_cut, label_dist);
        }
        else if (mode == 1)
        {
            grain2csv_skip(csvpath, distGrain_cut, label_dist);
        }
        cout << csvpath << " ended" << endl;
    }
    return 0;
}