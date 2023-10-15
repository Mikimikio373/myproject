#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <Windows.h>

#include "GrainMatching.hpp"


using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::to_string;


vector<string> label_center = { "centerX", "centerY", "flag" };
vector<string> label_dist = { "distX","distY","flag" };
string basepath = "IMAGE00_AREA-1/png";
string savepath = "GrainMatching_loop";
int minus = 45;



int main(int argc, char* argv[])
{
    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq))      // 単位習得
        return 0;
    LARGE_INTEGER start, end;

    int size = 15;
    int nLabs;

    vector<string> opt;
    double a1, b1, c1, d1;
    double a2, b2, c2, d2;
    
    string ref_path, comp_path, output_path, dist_name, ref_aff, comp_aff;
    string ref_name = "center_1";
    string comp_name = "center_2";
    int mode, target_num_max = 0, target_num_min = 0;
    int skipmode = 0;
    int opt_err = 0; //0: optionなし, 1: optionあり, -1: optionエラー
    if (argc == 5)
    {
        ref_path = argv[1];
        comp_path = argv[2];
        output_path = argv[3];
        dist_name = argv[4];
    }
    else if (argc > 5)
    {
        ref_path = argv[1];
        comp_path = argv[2];
        output_path = argv[3];
        dist_name = argv[4];
        for (int i = 5; i < argc; i++)
        {
            opt.emplace_back(argv[i]);
        }
        opt_err = getopt(opt, ref_name, comp_name, ref_aff, comp_aff, mode, minus, target_num_max, target_num_min, skipmode);
    }
    else
    {
        cerr << "exception : usage : GrainMatching.exe, ref_img, comp_img, output path, dist_name [options]" << endl;
        cerr << "   -COBname ref_name, comp_name" << endl;
        cerr << "   -aff ref_aff, comp_path, mode" << endl;
        cerr << "   -minus minus" << endl;
        cerr << "   -autominus max, min" << endl;
        cerr << "   -skip" << endl;
        exit(-1);
    }
    if (opt_err == -1)
    {
        cerr << "exception : usage : GrainMatching.exe, ref_img, comp_img, output path, dist_name [options]" << endl;
        cerr << "   -COBname ref_name, comp_name" << endl;
        cerr << "   -aff ref_aff, comp_path, mode" << endl;
        cerr << "   -minus minus" << endl;
        cerr << "   -autominus max, min" << endl;
        cerr << "   -skip" << endl;
        exit(-1);
    }
    
    cv::Mat mat_ref, mat_temp1, mat_ref_gauss, mat_ref_thr;
    cv::Mat mat_comp, mat_temp2, mat_comp_gauss, mat_comp_thr;
    cv::Mat labels, stats, centroids;

    if (std::filesystem::exists(output_path) == false)
    {
        std::filesystem::create_directories(output_path);
    }

    if (std::filesystem::exists(ref_path) == false)
    {
        cout << "there is not png file: " << ref_path << endl;
        return -1;
    }
    mat_ref = cv::imread(ref_path, 0);
    gaussianfilterminus(mat_ref, mat_ref_gauss, size, 0, minus);
    cv::threshold(mat_ref_gauss, mat_ref_thr, 1, 255, cv::THRESH_BINARY);
    nLabs = cv::connectedComponentsWithStats(mat_ref_thr, labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT); //ラベリング
    cout << nLabs << endl;
    if (target_num_max !=0 && target_num_min !=0)
    {
        
        if (nLabs < target_num_min)
        {
            while (nLabs < target_num_min) {
                minus--;
                gaussianfilterminus(mat_ref, mat_ref_gauss, size, 0, minus);
                cv::threshold(mat_ref_gauss, mat_ref_thr, 1, 255, cv::THRESH_BINARY);
                nLabs = cv::connectedComponentsWithStats(mat_ref_thr, labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT); //ラベリング
                cout << nLabs << endl;
            }
        }if (nLabs > target_num_max)
        {
            while (nLabs > target_num_max) {
                minus++;
                gaussianfilterminus(mat_ref, mat_ref_gauss, size, 0, minus);
                cv::threshold(mat_ref_gauss, mat_ref_thr, 1, 255, cv::THRESH_BINARY);
                nLabs = cv::connectedComponentsWithStats(mat_ref_thr, labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT); //ラベリング
                cout << nLabs << endl;
            }
        }
    }
    vector<grain> CenterOfBrightness1;
    string csvpath = output_path + "\\" + ref_name;
    CalculateBrightnessCenter(mat_ref_gauss, labels, stats, nLabs, CenterOfBrightness1, csvpath);
    cout << csvpath << " ended" << endl;
    if (ref_aff != "")
    {
        get_affparam(ref_aff, a1, b1, c1, d1);
        if (mode == 1)
        {
            change_coordinate_yflip(CenterOfBrightness1, a1, b1, c1, d1);
        }
        else if (mode ==2)
        {
            change_coordinate(CenterOfBrightness1, a1, b1, c1, d1);
        }
        string csvpath_stage = csvpath + "_stage";
        grain2csv(csvpath_stage, CenterOfBrightness1, label_center);
    }
    

    if (std::filesystem::exists(comp_path) == false)
    {
        cout << "there is not png file" << comp_path << endl;
        return -1;
    }
    mat_comp = cv::imread(comp_path, 0);
    gaussianfilterminus(mat_comp, mat_comp_gauss, size, 0, minus);
    cv::threshold(mat_comp_gauss, mat_comp_thr, 1, 255, cv::THRESH_BINARY);
    nLabs = cv::connectedComponentsWithStats(mat_comp_thr, labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT); //ラベリング
    cout << nLabs << endl;
    if (target_num_max != 0 && target_num_min != 0)
    {

        if (nLabs < target_num_min)
        {
            while (nLabs < target_num_min) {
                minus--;
                gaussianfilterminus(mat_comp, mat_comp_gauss, size, 0, minus);
                cv::threshold(mat_comp_gauss, mat_comp_thr, 1, 255, cv::THRESH_BINARY);
                nLabs = cv::connectedComponentsWithStats(mat_comp_thr, labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT); //ラベリング
                cout << nLabs << endl;
            }
        }if (nLabs > target_num_max)
        {
            while (nLabs > target_num_max) {
                minus++;
                gaussianfilterminus(mat_comp, mat_comp_gauss, size, 0, minus);
                cv::threshold(mat_comp_gauss, mat_comp_thr, 1, 255, cv::THRESH_BINARY);
                nLabs = cv::connectedComponentsWithStats(mat_comp_thr, labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT); //ラベリング
                cout << nLabs << endl;
            }
        }
    }
    vector<grain> CenterOfBrightness2;
    csvpath = output_path + "\\" + comp_name;
    CalculateBrightnessCenter(mat_comp_gauss, labels, stats, nLabs, CenterOfBrightness2, csvpath);
    cout << csvpath << " ended" << endl;

    if (comp_aff != "")
    {
        get_affparam(comp_aff, a2, b2, c2, d2);
        if (mode == 1)
        {
            change_coordinate(CenterOfBrightness2, a2, b2, c2, d2);
        }
        else if (mode == 2) {
            change_coordinate_yflip(CenterOfBrightness2, a2, b2, c2, d2);
        }
        
        string csvpath_stage = csvpath + "_stage";
        grain2csv(csvpath_stage, CenterOfBrightness2, label_center);
    }

    vector<grain> distGrain;
    QueryPerformanceCounter(&start);
    GrainMatching(CenterOfBrightness1, CenterOfBrightness2, distGrain, 0, 0, 0, 0);
    QueryPerformanceCounter(&end);
    std::cout << "culculate = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";
    csvpath = output_path + "\\" + dist_name;
    QueryPerformanceCounter(&start);
    if (skipmode == 1)
    {
        grain2csv_skip(csvpath, distGrain, label_dist);
    }
    else {
        grain2csv(csvpath, distGrain, label_dist);
    }
    QueryPerformanceCounter(&end);
    std::cout << "output = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";
   
    return 0;
}