#include <filesystem>
#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
//#include "picojson.h"

//外部インクルード
#include <nlohmann/json.hpp> //C:\Users\flab\Documents\json-develop\include

using std::cout;
using std::endl;
using json = nlohmann::json;


class originaltxt {
public:
	double px, py;
	double ax, ay;
	int ph;
	int vol;
	int num; //基本的に使わない
	int vola; //基本的に使わない
};

class txt4fvxx {
public:
	int line;
	int ph;
	double ax;
	double ay;
	double x;
	double y;
	double z1;
	double z2;
};

int main(int argc, char* argv[]) {

	for (int i = 0; i < argc; i++)
	{
		cout << argv[i] << endl;
	}

	if (argc != 6)
	{
		cout << "argc = " << argc << endl;
		cout << "command line error, please filepath nx ny npicture PL" << endl;
		return 3;
	}

	std::string filepath = argv[1];
	filepath = filepath + "/";
	int nx = atoi(argv[2]);
	int ny = atoi(argv[3]);
	int npicture = atoi(argv[4]);
	int pl = atoi(argv[5]);
	int pos1 = pl * 10 + 1;
	int pos2 = pl * 10 + 2;

	int nlayer = 2;
	int sum_size = nx * ny;
	int n = 0;
	int line_num1 = 0;
	int line_num2 = 0;
	int first, last;
	int line1 = 1;
	int line2 = 1;
	std::string foldername = "mt2f/";
	std::string string_n, string_vx, string_vy;
	originaltxt txt;
	txt4fvxx fv;

	if (!std::filesystem::exists(filepath))
	{
		cout << "not exist such folder:: "<< filepath << endl;
		return 1;
	}
	std::filesystem::create_directories(filepath + foldername);
	std::ostringstream ss_pos1, ss_pos2;
	ss_pos1 << std::setw(4) << std::setfill('0') << pos1;
	ss_pos2 << std::setw(4) << std::setfill('0') << pos2;
	std::string string_pos1(ss_pos1.str());
	std::string string_pos2(ss_pos2.str());
	std::string filename1 = "mt2f" + string_pos1 + ".txt";
	std::string filename2 = "mt2f" + string_pos2 + ".txt";
	std::ofstream file1, file2;
	file1.open((filepath + foldername + filename1), std::ios::out);
	file1 << "0 0 0 0 "<< pl << " 0 " << pos1 << " 0 0 \n" << "0 0 0 0 \n" << "0 0 0 0 0 0 0 0 \n" << "0 0 0 0 0 0 0 -1 0 0 0 0" << endl;
	file2.open((filepath + foldername + filename2), std::ios::out);
	file2 << "0 0 0 0 " << pl << " 0 " << pos2 << " 0 0 \n" << "0 0 0 0 \n" << "0 0 0 0 0 0 0 0 \n" << "0 0 0 0 0 0 0 -1 0 0 0 0" << endl;

	fv.line = 0;
	line1 = 1;
	line2 = 1;
	n = 0;
	for (int vy = 0; vy < ny; vy++) 
	{
		for (int layer = 0; layer < nlayer; layer++) 
		{
			for (int vx = 0; vx < nx; vx++, n++) 
			{
				//数字の文字化
				std::ostringstream string_n_ss, string_vx_ss, string_vy_ss;
				string_n_ss << std::setfill('0') << std::setw(8) << n;
				string_vx_ss << std::setfill('0') << std::setw(4) << vx;
				string_vy_ss << std::setfill('0') << std::setw(4) << vy;
				string_n = string_n_ss.str();
				string_vx = string_vx_ss.str();
				string_vy = string_vy_ss.str();
				string_n_ss.clear();
				string_vx_ss.clear();
				string_vy_ss.clear();

				std::string json_filename = "../IMAGE00_AREA-1/V" + string_n + "_L" + std::to_string(layer) +"_VX" + string_vx + "_VY" + string_vy + "_0_0" + std::to_string(npicture) + ".json";
				//std::string json_filename = "V" + string_n + "_VX" + string_vx + "_VY" + string_vy + "_zfilter.json";
				std::string ori_txtname = filepath + "L" + std::to_string(layer) + "_VX" + string_vx + "_VY" + string_vy + ".txt";
				if (!std::filesystem::exists(json_filename)) {
					cout << "not exist json file:  " << json_filename << endl;
					return 1;//ファイルがない場合の例外処理
				}
				if (!std::filesystem::exists(ori_txtname))
				{
					cout << "not exist microtrack txt file:  "<< ori_txtname << endl;
					return 1;
				}

				std::ifstream ori_txt(ori_txtname, std::ios::in);
				std::ifstream json_fs(json_filename, std::ios::in);
				json j;
				json_fs >> j;
				json_fs.close();

				if (layer == 0)
				{

					last = j["Last"];
					first = last - 15;
					fv.z1 = j["Images"][last]["z"];
					fv.z1 *= 1000;
					fv.z2 = j["Images"][first]["z"];
					fv.z2 *= 1000;

					//最初の一行吐き出し
					for (int l = 0; l < 8; l++)
					{
						std::string out;
						ori_txt >> out;

					}
					while (1)
					{

						if (!ori_txt.eof())
						{
							ori_txt >> txt.px;
							ori_txt >> txt.py;
							ori_txt >> txt.ax;
							ori_txt >> txt.ay;
							ori_txt >> txt.ph;
							ori_txt >> txt.vol;
							ori_txt >> txt.num;
							ori_txt >> txt.vola;
							fv.line = line1;
							fv.ph = txt.ph * 10000 + txt.vol;
							fv.ax = txt.ax;
							fv.ay = txt.ay;
							fv.x = txt.px * 1000;
							fv.y = txt.py * 1000;


							file2 << fv.line << " 0 " << fv.ph << " " << fv.ax << " " << fv.ay << " " << fv.x << " " << fv.y << " " << fv.z1 << " " << fv.z2 << endl;
							line1++;

						}
						else
						{
							break;
						}
					}
					cout << filepath + " V" + string_n + "_L" + std::to_string(layer) + "_VX" + string_vx + "_VY" + string_vy + " ended" << endl;


				}

				else if (layer == 1)
				{
					first = j["First"];
					last = first + 15;
					fv.z1 = j["Images"][last]["z"];
					fv.z1 *= 1000;
					fv.z2 = j["Images"][first]["z"];
					fv.z2 *= 1000;

					//最初の一行吐き出し
					for (int l = 0; l < 8; l++)
					{
						std::string out;
						ori_txt >> out;

					}
					while (1)
					{

						if (!ori_txt.eof())
						{

							ori_txt >> txt.px;
							ori_txt >> txt.py;
							ori_txt >> txt.ax;
							ori_txt >> txt.ay;
							ori_txt >> txt.ph;
							ori_txt >> txt.vol;
							ori_txt >> txt.num;
							ori_txt >> txt.vola;
							fv.line = line2;
							fv.ph = txt.ph * 10000 + txt.vol;
							fv.ax = txt.ax;
							fv.ay = txt.ay;
							fv.x = txt.px * 1000;
							fv.y = txt.py * 1000;

							file1 << fv.line << " 0 " << fv.ph << " " << fv.ax << " " << fv.ay << " " << fv.x << " " << fv.y << " " << fv.z1 << " " << fv.z2 << endl;
							line2++;


						}
						else
						{
							break;
						}

					}
					cout << filepath + " V" + string_n + "_L" + std::to_string(layer) + "_VX" + string_vx + "_VY" + string_vy + " ended" << endl;


				}
				ori_txt.close();


			}
		}
	}
	file1.close();
	file2.close();
	cout << filepath << "ended" << endl;
	
}
