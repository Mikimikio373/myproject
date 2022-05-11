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
void GrainMatchinghash(const vector<grain>& input_hash, const vector<grain>& input_pair, int cut_side, int cut_pixel, int hash_size, int siftX, int siftY);
void grain2csv(string filepath, const vector<grain>& input, vector<string> label);
vector<vector<double>> MatchingCount(const vector<vector<double>>& input, const vector<vector<double>>& pair, double thr_pixel);

vector<string> label_center = { "centerX", "centerY", "flag" };
vector<string> label_dist = { "distX","distY","flag" };
string basepath = "IMAGE00_AREA-1/png";
int minus = 20;



int main()
{
    int layer = 0;
    //for (int vx = 0; vx < 10; vx++)
    //{

    //}
    int vx = 0;
    int vy = 1;
    int NPcture = 96;
    string layer_s = std::to_string(layer);
    string vx_s = int2string_0set(vx, 4);
    string vy_s = int2string_0set(vy, 4);
    string foldername = "L" + layer_s + "_VX" + vx_s + "_VY" + vy_s;
    string savepath = basepath + "/" + foldername + "/GrainMatching4vib";
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
        string pngpath = basepath + "/" + foldername + "/" + foldername + "_" + std::to_string(num) + ".png";
        if (std::filesystem::exists(pngpath) == false)
        {
            cout << "there is not png file" << pngpath << endl;
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
            string csvpath = savepath + "/dist_" + pos1 + "_" + pos2;
            grain2csv(csvpath, distGrain, label_dist);
            cout << csvpath << " ended" << endl;
        }
    }
    
    
    /*
    //ずれ計算
    cv::Mat mat_ori1, mat_temp1, mat_gauss1, mat_thr1;
    cv::Mat mat_ori2, mat_temp2, mat_gauss2, mat_thr2;

    //基準画像の輝度重心計算
    int startvx = 1;
    int startvy = 0;
    string s_startvx = int2string_0set(startvx, 4);
    string s_startvy = int2string_0set(startvy, 4);

    string pngname1 = "L0_VX" + s_startvx + "_VY" + s_startvy;
    //string pngpath1 = basepath + "/" + pngname1 + "/" + pngname1 + "_95.png";
    string pngpath1 = basepath + "/" + pngname1 +  "_95.png";

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
    string csvpath = savepath + "/centerVX" + s_startvx +"_VY" + s_startvy;
    CalculateBrightnessCenter(mat_gauss1, labels, stats, nLabs, CenterOfBrightness1, csvpath);
    cout << csvpath << " ended" << endl;

    //二重ループでstage stepを変えて輝度重心計算→GrainMatching
    for (int a = 0; a < 21; a++)
    {
        for (int b = 0; b < 13; b++)
        {
            int vx = startvx + a;
            int vy = startvy + b;
            string s_vx = int2string_0set(vx, 4);
            string s_vy = int2string_0set(vy, 4);
            string pngname2 = "L0_VX" + s_vx + "_VY" + s_vy;
            //string pngpath2 = basepath + "/" + pngname2 + "/" + pngname2 + "_95.png";
            string pngpath2 = basepath + "/" + pngname2 + "_95.png";
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
            csvpath = savepath + "/centerVX" + s_vx + "_VY" + s_vy;
            CalculateBrightnessCenter(mat_gauss2, labels, stats, nLabs, CenterOfBrightness2, csvpath);
            cout << csvpath << " ended" << endl;

            vector<grain> distGrain;
            GrainMatching(CenterOfBrightness1, CenterOfBrightness2, distGrain, 0, 0, 0, 0);
            csvpath = savepath + "/dist_" + s_startvx + s_startvy +"vs" + s_vx + s_vy;
            grain2csv(csvpath, distGrain, label_dist);
            cout << csvpath << " ended" << endl;
        }
    }
    */

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
            cout << "\r" << i << "/" << input1.size() << string(12, ' ') << endl;
        }
        else
        {
            cout << "\r" << i << "/" << input1.size() << string(12, ' ');
        }
       
    }
}

void grain2csv(string filepath, const vector<grain>& input, vector<string> label)
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