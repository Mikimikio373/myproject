#pragma once
#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <sstream>
#include <fstream>

//外部インクルード
#include <nlohmann/json.hpp> //C:\Users\flab\Documents\json-develop\include

using std::cout;
using std::endl;
using std::vector;
using std::string;

class grain
{
public:
    double x = 0;
    double y = 0;
    int flg = 0;
};


vector<string> split(string str, char del) {
    int first = 0;
    int last = str.find_first_of(del);
    vector<string> result;
    while (first < str.size()) {
        string subStr(str, first, last - first);
        result.push_back(subStr);
        first = last + 1;
        last = str.find_first_of(del, first);
        if (last == string::npos) {
            last = str.size();
        }
    }
    return result;
}

//optionを探して代入して返す. 1: optionあり, -1: optionエラー
int getopt(vector<string>& input, int& minus, int& target_num_max, int& target_num_min) {

    int flg = 0;

    for (int i = 0; i < input.size(); i++)
    {
        if (input[i] == "-minus")
        {
            try
            {
                minus = std::stoi(input.at(i + 1));
                flg = 1;
            }
            catch (const std::exception& tmp)
            {
                return -1;
            }
        }
        else if (input[i] == "-autominus")
        {
            try
            {
                target_num_max = std::stoi(input.at(i + 1));
                target_num_min = std::stoi(input.at(i + 2));
                if (target_num_max <= target_num_min)
                {
                    throw "target_num_max <= target_num_min";
                }
                flg = 1;
            }
            catch (const std::exception& tmp)
            {
                return -1;
            }
        }
    }
    if (flg == 0) { return -1; }
    else { return 1; }
}

void get_affparam(string path, double& a, double& b, double& c, double& d) {
    if (std::filesystem::exists(path) == false)
    {
        std::cerr << "threre no file: " << path << endl;
        exit(-1);
    }

    std::ifstream ifs(path);
    string line;
    vector<vector<string>> data;
    while (std::getline(ifs, line))
    {
        vector<string> tmp = split(line, ',');
        data.emplace_back(tmp);
    }
    //abcdの代入
    a = stod(data[1][0]);
    b = stod(data[1][1]);
    c = stod(data[1][2]);
    d = stod(data[1][3]);
}

void get_stage(string path, double& sx, double& sy) {
    if (std::filesystem::exists(path) == false)
    {
        std::cerr << "threre no file: " << path << endl;
        exit(-1);
    }

    std::ifstream json_f(path, std::ios::in);
    nlohmann::json j;
    json_f >> j;
    json_f.close();
    sx = j["Images"][0]["x"];
    sy = j["Images"][0]["y"];
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

void change_coordinate(vector<grain>& input, double a, double b, double c, double d)
{
    for (int i = 0; i < input.size(); i++)
    {
        double x1 = a * input[i].x + b * input[i].y;
        double y1 = c * input[i].x + d * input[i].y;
        input[i].x = x1;
        input[i].y = y1;
    }
}
void change_coordinate_yflip(vector<grain>& input, double a, double b, double c, double d)
{
    for (int i = 0; i < input.size(); i++)
    {
        double x1 = a * input[i].x + b * (double)(input[i].y - 1088.0);
        double y1 = c * input[i].x + d * (double)(input[i].y - 1088.0);
        input[i].x = x1;
        input[i].y = y1;
    }
}

void pixel2stage(vector<grain>& input, double a, double b, double c, double d, double p, double q, double sx, double sy)
{
    double h_width = 1024.0;
    double h_height = 544.0;
    for (int i = 0; i < input.size(); i++)
    {
        double x = a * (input[i].x - h_width) + b * (input[i].y - h_height) + p + sx;
        double y = c * (input[i].x - h_width) + d * (input[i].y - h_height) + q + sy;
        input[i].x = x;
        input[i].y = y;
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
        fprintf(fp, "%.6f,%.6f,%d\r\n", input[i].x, input[i].y, input[i].flg);
    }
    fclose(fp);
    cout << "all out number of grain: " << input.size() << endl;
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
    int cnt = 0;
    for (int i = 0; i < sz; i++)
    {
        if (input[i].flg != 0) { continue; }
        fprintf(fp, "%.6f,%.6f,%d\r\n", input[i].x, input[i].y, input[i].flg);
        cnt++;
    }
    fclose(fp);
    cout << "skipped object number: " << cnt << endl;
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
            if (fabs(distx) < thr_pixel && fabs(disty) < thr_pixel)
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

vector<vector<string>> csv2vector(string filename, int ignore_line_num = 0) {
    //csvファイルの読み込み
    std::ifstream reading_file;
    reading_file.open(filename, std::ios::in);
    if (!reading_file) {
        std::vector<std::vector<std::string> > data;
        return data;
    }
    std::string reading_line_buffer;
    //最初のignore_line_num行を空読みする
    for (int line = 0; line < ignore_line_num; line++) {
        getline(reading_file, reading_line_buffer);
        if (reading_file.eof()) break;
    }

    //二次元のvectorを作成
    std::vector<std::vector<std::string> > data;
    while (std::getline(reading_file, reading_line_buffer)) {
        if (reading_line_buffer.size() == 0) break;
        std::vector<std::string> temp_data;
        temp_data = split(reading_line_buffer, ',');
        data.push_back(temp_data);
    }
    return data;
}
