#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <Windows.h>

#include "GrainMatchig_in_Stage.hpp"


using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::to_string;


vector<string> label_center = { "centerX", "centerY", "flag" };
vector<string> label_dist = { "distX","distY","flag" };
vector<string> label_dist_match = { "dx","dy", "x1", "y1", "x2", "y2"};
string basepath = "IMAGE00_AREA-1/png";
string savepath = "GrainMatching_loop";
int minus = 45;



int main(int argc, char* argv[])
{
    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq))      // ’PˆÊK“¾
        return 0;
    LARGE_INTEGER start, end;

    string ref_path, comp_path, output_path, dist_name;
    double tar_x = 0;
    double tar_y = 0;
    double cutrange = 0;
    int skipmode = 0;
    int opt_err = 0;
    if (argc == 5)
    {
        ref_path = argv[1];
        comp_path = argv[2];
        output_path = argv[3];
        dist_name = argv[4];
        dist_name = output_path + "/" + dist_name;
    }
    else if (argc > 5)
    {
        ref_path = argv[1];
        comp_path = argv[2];
        output_path = argv[3];
        dist_name = argv[4];
        dist_name = output_path + "/" + dist_name;
        vector<string> opt;
        for (int i = 5; i < argc; i++)
        {
            opt.emplace_back(argv[i]);
        }
        opt_err = getopt(opt, tar_x, tar_y, cutrange);
    }
    else
    {
        cerr << "exception : usage : .exe, ref_path, comp_path, output_path, output_name [option]" << endl;
        cerr << "   -cutmode target_x target_y cutrange" << endl;
        return -1;
    }
    if (opt_err == -1)
    {
        cerr << "optionerror : usage : .exe, ref_path, comp_path, output_path, output_name" << endl;
        cerr << "    -cutmode target_x target_y cutrange" << endl;
        return -1;
    }
    

    if (std::filesystem::exists(output_path) == false)
    {
        std::filesystem::create_directories(output_path);
    }

    if (std::filesystem::exists(ref_path) == false)
    {
        cout << "there is not png file: " << ref_path << endl;
        return -1;
    }
    if (std::filesystem::exists(comp_path) == false)
    {
        cout << "there is not png file: " << comp_path << endl;
        return -1;
    }
    vector<grain> ref_grain;
    QueryPerformanceCounter(&start);
    read_csv(ref_path, ref_grain);
    QueryPerformanceCounter(&end);
    std::cout << "read ref csv = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";

    vector<grain> comp_grain;
    QueryPerformanceCounter(&start);
    read_csv(comp_path, comp_grain);
    QueryPerformanceCounter(&end);
    std::cout << "read comp csv = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";

    vector<grain_match> distGrain;
    QueryPerformanceCounter(&start);
    GrainMatching_in_stage(comp_grain, ref_grain, distGrain, tar_x, tar_y, cutrange);
    QueryPerformanceCounter(&end);
    std::cout << "culculate = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";
    vector<grain> distGrain_cut;
    //calc_max_bin(distGrain, distGrain_cut, 500, -10, 10, 300, -6, 6);

    QueryPerformanceCounter(&start);
    grain2csv(dist_name, distGrain, label_dist_match);
    //grain2csv(dist_name, distGrain_cut, label_dist);
    QueryPerformanceCounter(&end);
    std::cout << "output = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";

    return 0;
}