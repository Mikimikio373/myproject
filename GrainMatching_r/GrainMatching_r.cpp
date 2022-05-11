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
void grain2csv(string filepath, const vector<grain>& input, vector<string> label);
vector<vector<double>> MatchingCount(const vector<vector<double>>& input, const vector<vector<double>>& pair, double thr_pixel);

vector<string> label_center = { "centerX", "centerY", "flag" };
vector<string> label_dist = { "distX","distY","flag" };
string basepath = "IMAGE00_AREA-1/png";
string savepath = "GrainMatching_loop";
int minus = 20;



int main(int argc, char* argv[])
{
    for (int i = 0; i < argc; i++)
    {
        cout << argv[i] << endl;
    }

    
    if (argc != 5)
    {
        cout << "argc = " << argc << endl;
        cout << "command line error, please filepath NPcture shift_x shift_Y" << endl;
        return 3;
    }
    

    std::string working_dir = argv[1];
    std::filesystem::current_path(working_dir);
    std::filesystem::path path = std::filesystem::current_path();
    cout << "current_path: " << path << endl;
    int NPcture = atoi(argv[2]);
    int shift_X = atoi(argv[3]);
    int shift_Y = atoi(argv[4]);

    int layer = 0;
    int vx = 0;
    int vy = 1;
    std::filesystem::create_directory(savepath);
    string layer_s = std::to_string(layer);
    string vx_s = int2string_0set(vx, 4);
    string vy_s = int2string_0set(vy, 4);
    int nLabs;
    cv::Mat labels, stats, centroids;
    vector<double> alldata;
    vector<cv::Mat> vec_ori, vec_gauss, vec_thr, vec_bitwise;
    vector<vector<grain>> CenterOfBrightnessAll;
    vector<vector<double>> MatchingGrain;
    

    int size = 15;

    //ずれ計算
    cv::Mat mat_ori1, mat_temp1, mat_gauss1, mat_thr1;
    cv::Mat mat_ori2, mat_temp2, mat_gauss2, mat_thr2;

    //基準画像の輝度重心計算
    int startvx = 1;
    int startvy = 0;
    string s_startvx = int2string_0set(startvx, 4);
    string s_startvy = int2string_0set(startvy, 4);

    string pngname1 = "L0_VX" + s_startvx + "_VY" + s_startvy;
    string pngpath1 = basepath + "/" + pngname1 + "_" + std::to_string(NPcture) + ".png";

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
    for (int a = 0; a < shift_X; a++)
    {
        for (int b = 0; b < shift_Y; b++)
        {
            int vx = startvx + a;
            int vy = startvy + b;
            string s_vx = int2string_0set(vx, 4);
            string s_vy = int2string_0set(vy, 4);
            string pngname2 = "L0_VX" + s_vx + "_VY" + s_vy;
            string pngpath2 = basepath + "/" + pngname2 + "_" + std::to_string(NPcture) + ".png";
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
            csvpath = savepath + "/dist_" + s_startvx + s_startvy + "vs" + s_vx + s_vy;
            grain2csv(csvpath, distGrain, label_dist);
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
void GrainMatchinghash(const vector<grain>& input_hash, const vector<grain>& input_pair, int cut_side, int cut_pixel, int hash_size, int siftX, int siftY)
{
    int pixel_bottom = 1088;
    int pixel_right;
    double hash_top = 0;
    double hash_bottom = hash_size;
    double hash_left, hash_right;
    string hashpath = savepath + "/hashcsv_" + "minus" + to_string(minus) + "_" + to_string(hash_size) + "_" + to_string(siftX + 1) + "by" + to_string(siftY + 1);
    std::filesystem::create_directory(hashpath);

    if (cut_side == 0)
    {
        pixel_right = 2048;
    }
    else
    {
        pixel_right = 2048 - cut_pixel;
    }
    double dist_x = (double)(pixel_right - hash_size) / siftX;
    double dist_y = (double)(pixel_bottom - hash_size) / siftY;

    while (hash_bottom <= pixel_bottom)
    {
        if (cut_side == 0)
        {
            hash_left = cut_pixel;
        }
        else
        {
            hash_left = 0;
        }
        hash_right = hash_left + hash_size;

        while (hash_right <= pixel_right)
        {
            vector<grain> hashCenter;
            string s_top = to_string((int)hash_top);
            string s_left = to_string((int)hash_left);
            string csvpath = hashpath + "/top_" + s_top + "left_" + s_left;

            for (int i = 0; i < input_hash.size(); i++)
            {
                grain temp = input_hash.at(i);
                if (temp.flg == 3) continue;
                if (temp.x < hash_left) continue;
                if (temp.x > hash_right) continue;
                if (temp.y < hash_top) continue;
                if (temp.y > hash_bottom) continue;
                hashCenter.push_back(temp);
            }
            if (hashCenter.size() == 0)
            {
                //cout << csvpath << " non data" << endl;
                csvpath += "_non";
            }
            vector<grain> hashDist;
            GrainMatching(hashCenter, input_pair, hashDist, 1, -158.1, 0, 5);
            grain2csv(csvpath, hashDist, label_dist);
            cout << csvpath << " ended" << endl;

            hash_right += dist_x;
            hash_left += dist_x;
        }
        //cout << "top:" << to_string((int)hash_top) << " left:" << to_string((int)hash_left) << " eneded" << endl;;
        hash_top += dist_y;
        hash_bottom += dist_y;
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