#define _CRT_SECURE_NO_WARNINGS
#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <Windows.h>


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
void GrainMatchinghash(const vector<grain>& input_hash, const vector<grain>& input_pair, int cut_side, int cut_pixel, int hash_size, int siftX, int siftY);
void calc_max_bin(const vector<grain>& input, vector<grain>& output, int xbin, double xmin, double xmax, int ybin, double ymin, double ymax);
void grain2csv(string filepath, const vector<grain>& input, vector<string> label);
void grain2csv0(string filepath, const vector<grain>& input, vector<string> label);
vector<vector<double>> MatchingCount(const vector<vector<double>>& input, const vector<vector<double>>& pair, double thr_pixel);

vector<string> label_center = { "centerX", "centerY", "flag" };
vector<string> label_dist = { "distX","distY","flag" };
string basepath = "IMAGE00_AREA-1/png";
string savepath = "GrainMatching_loop";
int minus = 50;



int main()
{
    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq))      // 単位習得
        return 0;
    LARGE_INTEGER start, end;

    int size = 15;
    int nLabs;
    
    //ずれ計算
    cv::Mat mat_ori1, mat_temp1, mat_gauss1, mat_thr1;
    cv::Mat mat_ori2, mat_temp2, mat_gauss2, mat_thr2;
    cv::Mat labels, stats, centroids;

    //基準画像の輝度重心計算
    string pngpath1 = "R:\\minami\\20221017_forGrainMaching0.025\\type1\\Module1\\ver-1\\E\\IMAGE00_AREA-1\\png\\L0_VX0000_VY0000_59.png";
    string pngpath2 = "R:\\minami\\20221017_forGrainMaching0.025\\type1\\Module1\\ver-1\\E\\IMAGE00_AREA-1\\png\\L0_VX0001_VY0000_59.png";
    //string pngpath1 = "R:\\minami\\20221017_sensorali\\id0-1.jpg";
    //string pngpath2 = "R:\\minami\\20221017_sensorali\\id1-1.jpg";

    if (std::filesystem::exists(pngpath1) == false)
    {
        cout << "there is not png file " << pngpath1 << endl;
        return -1;
    }
    mat_ori1 = cv::imread(pngpath1, 0);
    gaussianfilterminus(mat_ori1, mat_gauss1, size, 0, minus);
    cv::threshold(mat_gauss1, mat_thr1, 1, 255, cv::THRESH_BINARY);
    nLabs = cv::connectedComponentsWithStats(mat_thr1, labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT); //ラベリング
    vector<grain> CenterOfBrightness1;
    string csvpath = "R:\\minami\\20221017_forGrainMaching0.025\\center_ver-1";
    //string csvpath = "R:\\minami\\20221017_sensorali\\center_id-0";
    CalculateBrightnessCenter(mat_gauss1, labels, stats, nLabs, CenterOfBrightness1, csvpath);
    cout << csvpath << " ended" << endl;

    if (std::filesystem::exists(pngpath2) == false)
    {
        cout << "there is not png file" << pngpath2 << endl;
        return -1;
    }
    mat_ori2 = cv::imread(pngpath2, 0);
    gaussianfilterminus(mat_ori2, mat_gauss2, size, 0, minus);
    cv::threshold(mat_gauss2, mat_thr2, 1, 255, cv::THRESH_BINARY);
    nLabs = cv::connectedComponentsWithStats(mat_thr2, labels, stats, centroids, 8, CV_32S, cv::CCL_DEFAULT); //ラベリング
    vector<grain> CenterOfBrightness2;
    csvpath = "R:\\minami\\20221017_forGrainMaching0.025\\center_ver-2";
    //csvpath = "R:\\minami\\20221017_sensorali\\center_id-12";
    CalculateBrightnessCenter(mat_gauss2, labels, stats, nLabs, CenterOfBrightness2, csvpath);
    cout << csvpath << " ended" << endl;

    vector<grain> distGrain;
    QueryPerformanceCounter(&start);
    GrainMatching(CenterOfBrightness1, CenterOfBrightness2, distGrain, 0, 0, 0, 0);
    QueryPerformanceCounter(&end);
    std::cout << "culculate = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";
    csvpath = "R:\\minami\\20221017_forGrainMaching0.025\\dist_ver-1vs2";
    //csvpath = "R:\\minami\\20221017_sensorali\\dist_id-0vs1";
    QueryPerformanceCounter(&start);
    grain2csv(csvpath, distGrain, label_dist);
    QueryPerformanceCounter(&end);
    std::cout << "output = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";

    vector<grain> distGrain_cut;
    QueryPerformanceCounter(&start);
    calc_max_bin(distGrain, distGrain_cut, 200, -2048, 2048, 100, -1088, 1088);
    QueryPerformanceCounter(&end);
    std::cout << "cutting time = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";
    csvpath = "R:\\minami\\20221017_forGrainMaching0.025\\dist_ver-1vs2_cut";
    QueryPerformanceCounter(&start);
    grain2csv(csvpath, distGrain_cut, label_dist);
    QueryPerformanceCounter(&end);
    std::cout << "output = " << (double)(end.QuadPart - start.QuadPart) / freq.QuadPart << "sec.\n";
    cout << csvpath << " ended" << endl;
   
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

vector<vector<double>> MatchingCount(const vector<vector<double>>& input, const vector<vector<double>>& pair, double thr_pixel)
{
    vector<vector<double>> output;
    for (int i = 0; i < input.size(); i++)
    {
        for (int j = 0; j < pair.size(); j++)
        {
            double distx = input.at(i).at(0) - pair.at(j).at(0);
            double disty = input.at(i).at(1) - pair.at(j).at(1);
            if (fabs(distx)< thr_pixel && fabs(disty) < thr_pixel)
            {
                vector<double> out = { input.at(i).at(0), input.at(i).at(1) };
                output.push_back(out);
                break;
            }
            else
            {
                continue;
            }
        }
    }
    return output;
}