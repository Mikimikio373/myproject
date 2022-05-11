#include <opencv2/opencv.hpp>
#include <iostream>
#include <direct.h>
#include <filesystem>
#include <fstream>
#include <sstream>

//外部インクルード
#include <nlohmann/json.hpp> //C:\Users\flab\Documents\json-develop\include

using std::cout;
using std::endl;
using std::string;
using std::vector;
using json = nlohmann::json;

vector<string> split(string str, string del);
template <class T> void read_vbin(std::string filepath, std::vector<std::vector<T>>& vvin);
std::vector<cv::Mat> read_spng(std::string filepath, int start, int npic);


int main()
{
    string json_name = "V00000000_L0_VX0000_VY0000_0_030.json";
    string spng_name = "V00000000_L0_VX0000_VY0000_0_030.spng";
    string output_dir = "png";
    int snap_num = 30;

    //if (std::filesystem::exists(output_dir) == true)
    //{
    //    cout << "This folder already exists. Can I overwrite it? y/n" << endl;
    //    string answer;
    //    std::cin >> answer;
    //    if (answer == "y")
    //    {
    //        std::filesystem::create_directory(output_dir);
    //    }
    //    else if (answer == "n")
    //    {
    //        cout << "This program ended" << endl;
    //        return 1;
    //    }
    //    else
    //    {
    //        cout << "error! prease input y/n" << endl;
    //        return -2;
    //    }
    //}
    //else
    //{
    //    std::filesystem::create_directory(output_dir);
    //}
    
    if (!std::filesystem::exists(json_name)) {
        cout << "not exist json file:  " << json_name << endl;
        return -2;//ファイルがない場合の例外処理
    }
    if (!std::filesystem::exists(spng_name)) {
        cout << "not exist spng file:  " << spng_name << endl;
        return -2;//ファイルがない場合の例外処理
    }   
    std::vector<cv::Mat> vsrc = read_spng(spng_name, 0, snap_num);

    cv::imshow("window1", vsrc[0]);
    cv::waitKey(0);
    
}

vector<string> split(string str, string del) {
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

template <class T> void read_vbin(std::string filepath, std::vector<std::vector<T>>& vvin)
{
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs.is_open()) { std::cout << "cannot open " << filepath << std::endl; throw std::exception(); }
    int num = 0;
    while (true)
    {
        uint64_t i64;
        ifs.read((char*)&i64, sizeof(uint64_t));
        cout << i64 << endl;
        if (ifs.eof()) { break; }

        std::vector<T> vin;
        if (i64 == -1)
        {
            vvin.push_back(vin);
            continue;
        }
        if (i64 > vin.max_size()) { throw std::exception(""); }
        vin.reserve(i64);
        for (uint64_t j = 0; j < i64; j++)
        {
            T p;
            ifs.read((char*)&p, sizeof(T));
            vin.push_back(p);
        }
        vvin.push_back(vin);
        cout << num << "push backed" << endl;
        num++;
    }
    ifs.close();
}

std::vector<cv::Mat> read_spng(std::string filepath, int start, int npic) {
    std::vector<std::vector<uchar>> stacked_png;
    read_vbin(filepath, stacked_png);

    if (npic < 0) { npic = stacked_png.size(); }
    if (start + npic > stacked_png.size()) { throw std::exception(); }
    std::vector<cv::Mat> vmat;
    for (int i = start; i < start + npic; i++) {
        vmat.emplace_back(cv::imdecode(stacked_png[i], -1));
    }
    return vmat;
}