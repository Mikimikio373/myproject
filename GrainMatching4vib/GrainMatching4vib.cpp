﻿#define _CRT_SECURE_NO_WARNINGS
#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>


using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::to_string;

class grain
{
public:
    double x = 0;
    double y = 0;
    int flg = 0;

private:

};


string int2string_0set(int input, int digit);
void gaussianfilter(const cv::Mat& src, cv::Mat& dst, int size, double sigma);
void gaussianfilterminus(const cv::Mat& src, cv::Mat& dst, int size, double sigma, int minus);
void CalculateBrightnessCenter(cv::Mat src, cv::Mat labels, cv::Mat stats, int nLabs, vector<grain>& out, string filepath);
void GrainMatching(const vector<grain>& input1, const vector<grain>& input2, vector<grain>& out, int type, double centerX, double centerY, double cut_pixel);
void calc_max_bin(const vector<grain>& input, vector<grain>& output, int xbin, double xmin, double xmax, int ybin, double ymin, double ymax);
void grain2csv(string filepath, const vector<grain>& input, vector<string> label);
void grain2csv_skip(string filepath, const vector<grain>& input, vector<string> label);
void grain2csv0(string filepath, const vector<grain>& input, vector<string> label);

vector<string> label_center = { "centerX", "centerY", "flag" };
vector<string> label_dist = { "distX","distY","flag" };




int main(int argc, char* argv[])
{
    for (int i = 0; i < argc; i++)
    {
        cout << argv[i] << endl;
    }


    if (argc != 6)
    {
        cout << "argc = " << argc << endl;
        cout << "command line error, please vx vy NPicture minus mode" << endl;
        return 3;
    }
    int layer = 0;
    int vx = atoi(argv[1]);
    int vy = atoi(argv[2]);
    int NPcture = atoi(argv[3]);
    int minus = atoi(argv[4]);
    int mode = atoi(argv[5]);
    string layer_s = std::to_string(layer);
    string vx_s = int2string_0set(vx, 4);
    string vy_s = int2string_0set(vy, 4);
    string foldername = "L" + layer_s + "_VX" + vx_s + "_VY" + vy_s;
    string savepath = foldername + "/GrainMatching4vib";
    std::filesystem::create_directory(savepath);
    int nLabs;
    cv::Mat labels, stats, centroids;
    vector<double> alldata;
    vector<cv::Mat> vec_ori, vec_gauss, vec_thr, vec_bitwise;
    vector<vector<grain>> CenterOfBrightnessAll;
    vector<vector<double>> MatchingGrain;
    

    int size = 15;
    double sigma = 0;
    double filter_length = 0.15;
    double cut_pixel = 1;
    int width = 2048;
    int height = 1088;
    int cut_numX = 4;
    int cut_numY = 2;
    int cut_num = cut_numX * cut_numY;
    if (width % cut_numX != 0)
    {
        cout << "cut_width error! cut_width must be the approxior of 2048" << endl;
        return -3;
    }
    if (height % cut_numY != 0)
    {
        cout << "cut_height error! cut_height must be the approxior of 1088" << endl;
        return -3;
    }
    


    // 振動計算用
    //メモリのせ
    for (int num = 0; num < NPcture; num++)
    {
        cv::Mat mat_ori, mat_temp, mat_gauss, mat_thr;
        string pngpath = foldername + "/" + foldername + "_" + std::to_string(num) + ".png";
        //string pngpath = foldername + "/" + foldername + "_" + std::to_string(num) + "_0.png";
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
    for (int num = 0; num < NPcture; num++)
    {
        nLabs = cv::connectedComponentsWithStats(vec_thr[num], labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT); //ラベリング
        vector<grain> CenterOfBrightness;
        string csvpath = savepath + "/center_" + to_string(num);
        CalculateBrightnessCenter(vec_gauss[num], labels, stats, nLabs, CenterOfBrightness, csvpath);
        CenterOfBrightnessAll.push_back(CenterOfBrightness);
        cout << csvpath << " ended" << endl;

    }
    
    //重心距離計算 loop
    for (int num = 0; num < 1; num++)
    {
        for (int i = num + 1; i < NPcture; i++)
        {
            vector<grain> distGrain;
            string pos1 = int2string_0set(num, 3);
            string pos2 = int2string_0set(i, 3);

            GrainMatching(CenterOfBrightnessAll[num], CenterOfBrightnessAll[i], distGrain, 0, 0, 0, 0);
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
    }
    return 0;
}

string int2string_0set(int input, int digit) {
    std::ostringstream ss;
    ss << std::setw(digit) << std::setfill('0') << input;
    string s(ss.str());

    return s;
}

void gaussianfilter(const cv::Mat& src, cv::Mat& dst, int size, double sigma)
{
    cv::Mat temp;
    cv::GaussianBlur(src, temp, cv::Size(size, size), (sigma, sigma));
    cv::subtract(temp, src, dst);
}
void gaussianfilterminus(const cv::Mat& src, cv::Mat& dst, int size, double sigma, int minus)
{
    cv::Mat temp;
    cv::GaussianBlur(src, temp, cv::Size(size, size), (sigma, sigma));
    cv::subtract(temp, src, temp);
    cv::subtract(temp, minus, dst);
}

void CalculateBrightnessCenter(cv::Mat src, cv::Mat labels, cv::Mat stats, int nLabs, vector<grain>& out, string filepath)
{
    string file = filepath + "_stats.csv";
    std::ofstream ofs;
    ofs.open(file, std::ios::out);
    //一行目(ラベルの記述)
    ofs << "label,left,top,height,width,area,centerX,centerY,flag" << endl;

    for (int i = 1; i < nLabs; i++)
    {
        //label情報の参照
        int* param = stats.ptr<int>(i);
        int left = param[cv::ConnectedComponentsTypes::CC_STAT_LEFT];
        int top = param[cv::ConnectedComponentsTypes::CC_STAT_TOP];
        int height = param[cv::ConnectedComponentsTypes::CC_STAT_HEIGHT];
        int width = param[cv::ConnectedComponentsTypes::CC_STAT_WIDTH];
        int area = param[cv::ConnectedComponentsTypes::CC_STAT_AREA];

        ofs << i << "," << left << "," << top << "," << height << "," << width << "," << area << ",";

        //輝度値の全探索
        double BrightSum = 0;
        double BrightX = 0;
        double BrightY = 0;
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                if (labels.at<int>(top + y, left + x) != i) { continue; } //label番号の違うものをスキップ
                int bright = src.at<unsigned char>(top + y, left + x);
                BrightSum += bright;
                BrightX += (double)bright * x;
                BrightY += (double)bright * y;
            }
        }
        grain push;
        push.x = (double)left + BrightX / BrightSum;
        push.y = (double)top + BrightY / BrightSum;
        if (height == 1)
        {
            push.flg = 1;
        }
        if (width == 1)
        {
            push.flg = 2;
        }
        if (area == 1)
        {
            push.flg = 3;
        }
        ofs << push.x << "," << push.y << "," << push.flg << endl;
        out.push_back(push);

    }
}

void GrainMatching(const vector<grain>& input1, const vector<grain>& input2, vector<grain>& out, int type, double centerX, double centerY, double cut_pixel)
{
    /*type = 0 flag=3をpass, type=1 カット, type=2 全部入れる*/

    int size1 = input1.size();
    int size2 = input2.size();

    cout << "total grains " << size1 << " vs " << size2 << endl;

    for (int i = 0; i < input1.size(); i++)
    {
        for (int j = 0; j < input2.size(); j++)
        {
            int flg1 = input1.at(i).flg;
            int flg2 = input2.at(j).flg;
            grain dist;
            if (type != 2)
            {
                if (flg1 == 3 || flg2 == 3) continue;
            }
            //このフラグ判断ベン図的に怪しい
            if (flg1 == 1 || flg2 == 1)
            {
                dist.flg = 1;
            }
            if (flg1 == 2 || flg2 == 2)
            {
                dist.flg = 2;
            }
            if (flg1 == 3 || flg2 == 3)
            {
                dist.flg = 3;
            }
            dist.x = input1.at(i).x - input2.at(j).x;
            dist.y = input1.at(i).y - input2.at(j).y;
            if (type == 1)
            {
                if (fabs(dist.x - centerX) > cut_pixel) continue;
                if (fabs(dist.y - centerY) > cut_pixel) continue;
            }
            out.push_back(dist);
        }
        if (i == input1.size() - 1)
        {
            //cout << "\r" << i << "/" << input1.size() << string(12, ' ') << endl;
        }
        else
        {
            //cout << "\r" << i << "/" << input1.size() << string(12, ' ');
        }
       
    }
}

void calc_max_bin(const vector<grain>& input, vector<grain>& output, int xbin, double xmin, double xmax, int ybin, double ymin, double ymax)
{
    double x_wbin = (xmax - xmin) / xbin;
    double y_wbin = (ymax - ymin) / ybin;
    vector<vector<vector<grain>>> box(xbin, vector<vector<grain>>(ybin));
    for (int i = 0; i < input.size(); i++)
    {
        //範囲外にあったらpass
        if (input[i].x < xmin || input[i].x > xmax || input[i].y < ymin || input[i].y > ymax) { continue; }
        grain tmp;
        tmp.x = input[i].x;
        tmp.y = input[i].y;
        tmp.flg = input[i].flg;
        int a = (int)floor((input[i].x - xmin) / x_wbin); //代入するbinの計算
        int b = (int)floor((input[i].y - ymin) / y_wbin);
        box[a][b].push_back(tmp); //bin詰め
    }
    int maxbinsize = 0;
    int maxbinx, maxbiny;
    for (int i = 0; i < xbin; i++)
    {
        for (int j = 0; j < ybin; j++)
        {
            int size = box[i][j].size();
            if (size > maxbinsize)
            {
                maxbinsize = size;
                maxbinx = i;
                maxbiny = j;
            }
        }
    }
    output = box[maxbinx][maxbiny];
    for (int i = -1; i < 2; i++)
    {
        for (int j = -1; j < 2; j++)
        {
            if (i == 0 && j == 0) { continue; }
            output.insert(output.end(), box[maxbinx + i][maxbiny + j].begin(), box[maxbinx + i][maxbiny + j].end());
        }
    }
}

void grain2csv(string filepath, const vector<grain>& input, vector<string> label)
{
    string file = filepath + ".csv";
    std::ofstream ofs;
    FILE* fp = std::fopen(file.c_str(), "wb");
    //一行目(ラベルの記述)
    for (int i = 0; i < label.size() - 1; i++)
    {
        fputs((label.at(i) + ",").c_str(), fp);
    }
    fputs((label.at(label.size() - 1) + "\r\n").c_str(), fp);

    auto sz = input.size();
    for (int i = 0; i < sz; i++)
    {
        fprintf(fp, "%g,%g,%d\r\n", input[i].x, input[i].y, input[i].flg);
    }
    fclose(fp);
}

void grain2csv_skip(string filepath, const vector<grain>& input, vector<string> label)
{
    string file = filepath + ".csv";
    std::ofstream ofs;
    FILE* fp = std::fopen(file.c_str(), "wb");
    //一行目(ラベルの記述)
    for (int i = 0; i < label.size() - 1; i++)
    {
        fputs((label.at(i) + ",").c_str(), fp);
    }
    fputs((label.at(label.size() - 1) + "\r\n").c_str(), fp);

    auto sz = input.size();
    for (int i = 0; i < sz; i++)
    {
        if (input[i].flg != 0) { continue; }
        fprintf(fp, "%g,%g,%d\r\n", input[i].x, input[i].y, input[i].flg);
    }
    fclose(fp);
}

void grain2csv0(string filepath, const vector<grain>& input, vector<string> label)
{
    string file = filepath + ".csv";
    std::ofstream ofs;
    ofs.open(file, std::ios::out);
    //一行目(ラベルの記述)
    for (int i = 0; i < label.size() - 1; i++)
    {
        ofs << label.at(i) << ",";
    }
    ofs << label.at(label.size() - 1) << endl;

    for (int i = 0; i < input.size(); i++)
    {
        ofs << input.at(i).x << "," << input.at(i).y << "," << input.at(i).flg << endl;
    }
}