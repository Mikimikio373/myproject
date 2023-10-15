#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <Windows.h>

#include "pixel2stage.hpp"

using std::cout;
using std::endl;
using std::cerr;
using std::vector;
using std::string;

vector<string> label_center = { "X", "Y", "flag" };

int main(int argc, char* argv[]) {
    cv::Mat mat_ref, mat_temp1, mat_ref_gauss, mat_ref_thr;
    cv::Mat labels, stats, centroids;

    int size = 15;
    int minus = 50;
    int target_num_max = 0, target_num_min = 0;
    int nLabs;
    double a, b, c, d, p, q, sx, sy;
    int json_flag = 0;

    string ref_path, json_path, aff_path, out_path, out_name;

    int opt_err = 0;
    if (argc == 6)
    {
        ref_path = argv[1];
        json_path = argv[2];
        aff_path = argv[3];
        out_path = argv[4];
        out_name = argv[5];
    }
    else if (argc > 6)
    {
        ref_path = argv[1];
        json_path = argv[2];
        aff_path = argv[3];
        out_path = argv[4];
        out_name = argv[5];
        vector<string> opt;
        for (int i = 6; i < argc; i++)
        {
            opt.emplace_back(argv[i]);
        }
        opt_err = getopt(opt, minus, target_num_max, target_num_min, json_flag, sx, sy);
    }
    else
    {
        cerr << "exception : usage : pixel2stage.exe, img_path, json_path, affine_path, output_path_name, output_name [options]" << endl;
        cerr << "   -minus minus" << endl;
        cerr << "   -autominus max, min" << endl;
        cerr << "   -nojson sx sy" << endl;
        exit(-1);
    }
    if (opt_err == -1)
    {
        cerr << "exception : usage : pixel2stage.exe, img_path, json_path, affine_path, output_path_name, output_name [options]" << endl;
        cerr << "   -minus minus" << endl;
        cerr << "   -autominus max, min" << endl;
        cerr << "   -nojson sx sy" << endl;
        exit(-1);
    }
    if (std::filesystem::exists(out_path) == false)
    {
        std::filesystem::create_directories(out_path);
    }
	
    get_affparam(aff_path, a, b, c, d);
    if (json_flag == 0)
    {
        get_stage(json_path, sx, sy);
    }
    
    p = 0;
    q = 0;

    if (std::filesystem::exists(ref_path) == false)
    {
        std::cerr << "threre no file: " << ref_path << endl;
        exit(-1);
    }
    mat_ref = cv::imread(ref_path, 0);
    gaussianfilterminus(mat_ref, mat_ref_gauss, size, 0, minus);
    cv::threshold(mat_ref_gauss, mat_ref_thr, 1, 255, cv::THRESH_BINARY);
    nLabs = cv::connectedComponentsWithStats(mat_ref_thr, labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT); //ラベリング
    cout << nLabs << endl;

    if (target_num_max != 0 && target_num_min != 0)
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

    vector<grain> CenterOfBrightness;
    string center_path = out_path + "\\" + out_name;
    CalculateBrightnessCenter(mat_ref_gauss, labels, stats, nLabs, CenterOfBrightness, center_path);
    cout << sx << ", " << sy << endl;
    pixel2stage(CenterOfBrightness, a, b, c, d, p, q, sx, sy);
    string center_path_stage = out_path + "\\" + out_name + "_stage";
    grain2csv_skip(center_path_stage, CenterOfBrightness, label_center);
}