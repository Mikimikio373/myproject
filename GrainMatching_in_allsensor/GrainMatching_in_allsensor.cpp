#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <Windows.h>

#include "GrainMatching_in_allsensor.hpp"


using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::to_string;


vector<string> label_center = { "centerX", "centerY", "flag" };
vector<string> label_dist = { "dx","dy", "x1", "y1", "x2", "y2" ,"flag"};
string basepath = "IMAGE00_AREA-1/png";
string savepath = "GrainMatching_loop";
int minus = 45;



int main(int argc, char* argv[])
{
    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq))      // íPà èKìæ
        return 0;
    LARGE_INTEGER start, end;


    string ref_path, comp_path1, comp_path2, output_path, dist_name1, dist_name2;
    int skipmode = 0;
    int nx = 10;
    int ny = 12;

    string basepath = "Q:/minami/20230914_Ali/Module0/sensor-3";
    output_path = basepath + "/chech_stage";

    if (std::filesystem::exists(output_path) == false)
    {
        std::filesystem::create_directories(output_path);
    }

    for (int vx = 0; vx < nx - 1; vx++)
    {
        for (int vy = 0; vy < ny - 1; vy++)
        {
            int comp_vx = vx + 1;
            int comp_vy = vy + 1;
            string vx_s = int2string_0set(vx, 4);
            string vy_s = int2string_0set(vy, 4);
            string comp_vx_s = int2string_0set(vx + 1, 4);
            string comp_vy_s = int2string_0set(vy + 1, 4);
            ref_path = basepath + "/stage_all/L0_VX" + vx_s + "_VY" + vy_s + "_stage.csv";
            comp_path1 = basepath + "/stage_all/L0_VX" + comp_vx_s + "_VY" + vy_s + "_stage.csv";
            comp_path2 = basepath + "/stage_all/L0_VX" + vx_s + "_VY" + comp_vy_s + "_stage.csv";
            dist_name1 = output_path + "/" + vx_s + vy_s + "vs" + comp_vx_s + vy_s;
            dist_name2 = output_path + "/" + vx_s + vy_s + "vs" + vx_s + comp_vy_s;

            if (std::filesystem::exists(ref_path) == false)
            {
                cout << "there is no file: " << ref_path << endl;
                return -1;
            }
            if (std::filesystem::exists(comp_path1) == false)
            {
                cout << "there is no file: " << comp_path1 << endl;
                return -1;
            }
            if (std::filesystem::exists(comp_path2) == false)
            {
                cout << "there is no file: " << comp_path2 << endl;
                return -1;
            }
            vector<grain> ref_grain;
            QueryPerformanceCounter(&start);
            read_csv(ref_path, ref_grain);
            QueryPerformanceCounter(&end);
            std::cout << "read ref csv = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";

            vector<grain> comp_grain1;
            QueryPerformanceCounter(&start);
            read_csv(comp_path1, comp_grain1);
            QueryPerformanceCounter(&end);
            std::cout << "read comp csv = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";

            vector<grain> comp_grain2;
            QueryPerformanceCounter(&start);
            read_csv(comp_path2, comp_grain2);
            QueryPerformanceCounter(&end);
            std::cout << "read comp csv = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";

            vector<grain_match> distGrain1;
            QueryPerformanceCounter(&start);
            GrainMatching_in_stage(comp_grain1, ref_grain, distGrain1);
            QueryPerformanceCounter(&end);
            std::cout << "culculate = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";
            QueryPerformanceCounter(&start);
            grain2csv(dist_name1, distGrain1, label_dist);
            QueryPerformanceCounter(&end);
            std::cout << "output = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";

            vector<grain_match> distGrain2;
            QueryPerformanceCounter(&start);
            GrainMatching_in_stage(comp_grain2, ref_grain, distGrain2);
            QueryPerformanceCounter(&end);
            std::cout << "culculate = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";
            QueryPerformanceCounter(&start);
            grain2csv(dist_name2, distGrain2, label_dist);
            QueryPerformanceCounter(&end);
            std::cout << "output = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";
        }
    }

    return 0;
}