#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <math.h>

using std::cout;
using std::endl;
using std::string;

class originaltxt {
public:
	double px, py;
	double ax, ay;
	int ph;
	int vol;
	int num; //基本的に使わない
	int vola; //基本的に使わない
};


int main()
{

	originaltxt txt;
	std::vector<originaltxt> v_txt;
	string ori_txtname0 = "L0_VX0014_VY0023.txt";
	string ori_txtname1 = "L1_VX0014_VY0023.txt";
	if (!std::filesystem::exists(ori_txtname0))
	{
		cout << "not exist txt file:  " << ori_txtname0 << endl;
		return 1;//ファイルがない場合の例外処理
	}
	string out_csv_name = ".\\mt2f\\L0_VX0014_VY0023.csv";
	std::ifstream ori_txt0(ori_txtname0, std::ios::in);
	std::ifstream ori_txt1(ori_txtname1, std::ios::in);

	for (int l = 0; l < 8; l++)
	{
		std::string out;
		ori_txt0 >> out;

	}
	while (1)
	{

		if (!ori_txt0.eof())
		{
			ori_txt0 >> txt.px;
			ori_txt0 >> txt.py;
			ori_txt0 >> txt.ax;
			ori_txt0 >> txt.ay;
			ori_txt0 >> txt.ph;
			ori_txt0 >> txt.vol;
			ori_txt0 >> txt.num;
			ori_txt0 >> txt.vola;
			txt.px *= 1000;
			txt.py *= 1000;

			v_txt.push_back(txt);
		}
		else
		{
			break;
		}
	}
	for (int l = 0; l < 8; l++)
	{
		std::string out;
		ori_txt1 >> out;

	}
	while (1)
	{

		if (!ori_txt1.eof())
		{
			ori_txt1 >> txt.px;
			ori_txt1 >> txt.py;
			ori_txt1 >> txt.ax;
			ori_txt1 >> txt.ay;
			ori_txt1 >> txt.ph;
			ori_txt1 >> txt.vol;
			ori_txt1 >> txt.num;
			ori_txt1 >> txt.vola;
			txt.px *= 1000;
			txt.py *= 1000;

			v_txt.push_back(txt);
		}
		else
		{
			break;
		}
	}

	ori_txt0.close();
	ori_txt1.close();

	std::ofstream out_csv(out_csv_name);



	for (int i = 0; i < v_txt.size() - 1; i++)
	{
		out_csv << v_txt.at(i).px << "," << v_txt.at(i).py << "," << v_txt.at(i).ax << "," << v_txt.at(i).ay << "," << v_txt.at(i).ph << "," << v_txt.at(i).vol << endl;
	}

	out_csv.close();
}

