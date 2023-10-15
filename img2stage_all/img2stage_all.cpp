#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <Windows.h>

//外部インクルード
#include <nlohmann/json.hpp> //C:\Users\flab\Documents\json-develop\include

#include "img2stage_all.hpp"

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
    int minus = 30;
    int target_num_max = 0, target_num_min = 0;
    int nLabs;
    double a, b, c, d, p, q, sx, sy;

    int nx = 10;
    int ny = 8;
    int npicturenum = 2;
    string basepath = "Q:/minami/20230914_Ali2/Module0/sensor-3";
    string ref_path, json_path, aff_path, out_path, out_name;
    
    aff_path = basepath + "/affdata_surf.csv";
    get_affparam(aff_path, a, b, c, d);
    p = 0;
    q = 0;

    out_path = basepath + "/stage_all";
    if (std::filesystem::exists(out_path) == false)
    {
        std::filesystem::create_directories(out_path);
    }

    int n = 0;
    vector<grain> all_grain_list;
    for (int vy = 0; vy < ny; vy++)
    {
        for (int vx = 0; vx < nx; vx++)
        {
            string img_name = "L0_VX" + int2string_0set(vx, 4) +"_VY" + int2string_0set(vy, 4);
            ref_path = basepath + "/IMAGE00_AREA-1" + "/png/" + img_name + "/" + img_name + "_0.png";
            json_path = basepath + "/IMAGE00_AREA-1" + "/V" + int2string_0set(n, 8) + "_" + img_name + "_0_" + int2string_0set(npicturenum, 3) + ".json";
            n++;
            get_stage(json_path, sx, sy);

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

            out_name = img_name;
            vector<grain> CenterOfBrightness;
            string center_path = out_path + "/" + out_name;
            CalculateBrightnessCenter(mat_ref_gauss, labels, stats, nLabs, CenterOfBrightness, center_path);
            pixel2stage(CenterOfBrightness, a, b, c, d, p, q, sx, sy);
            string center_path_stage = out_path + "/" + out_name + "_stage";
            grain2csv_skip(center_path_stage, CenterOfBrightness, label_center);
            if (vy == 0) { continue; }
            for (int i = 0; i < CenterOfBrightness.size(); i++)
            {
                if (CenterOfBrightness[i].flg != 0) { continue; }
                all_grain_list.push_back(CenterOfBrightness[i]);
            }
        }
    }
    
    string all_out_path = basepath + "/all_stage_list";
    grain2csv(all_out_path, all_grain_list, label_center);
    return 0;
}