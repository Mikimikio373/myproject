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

class rl_data
{
public:
	double x, y, ax, ay, drR, drL, dtR, dtL;
	int predph, predvol, ph, vol;
};


int main()
{

	originaltxt txt;
	std::vector<originaltxt> v_txt;
	string ori_txtname = "L0_VX0000_VY0000.txt";
	string out_csv_name = "./rawrawTrack.csv";
	std::ifstream ori_txt(ori_txtname, std::ios::in);

	double dx_cut = 30;
	double dy_cut = 30;
	double dax_cut = 0.5;
	double day_cut = 0.5;
	int n_cut = 100;
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
			txt.px *= 1000;
			txt.py *= 1000;

			v_txt.push_back(txt);
		}
		else
		{
			break;
		}
	}

	ori_txt.close();

	std::ofstream out_csv(out_csv_name);


	for (int i = 1; i < v_txt.size(); i++)
	{
		if (i % 1000 == 0)
		{
			cout << "line" << i << "ended" << endl;
		}
		if (fabs(v_txt.at(i).px + 50000.) > 50.) { continue; }
		if (fabs(v_txt.at(i).py + 11000.) > 50.) { continue; }
		int n = 0;
		for (int j = i + 1; j < v_txt.size(); j++)
		{
			//if (sqrt(v_txt.at(j).ax * v_txt.at(j).ax + v_txt.at(j).ay * v_txt.at(j).ax) < 0.4 && v_txt.at(j).vol <= 3) { continue; }
			double dx = v_txt.at(j).px - v_txt.at(i).px;
			double dy = v_txt.at(j).py - v_txt.at(i).py;
			double dax = v_txt.at(j).ax - v_txt.at(i).ax;
			double day = v_txt.at(j).ay - v_txt.at(i).ay;
			//領域外のものを飛ばす
			if (fabs(dx) > dx_cut) { continue; }
			if (fabs(dy) > dy_cut) { continue; }
			if (fabs(dax) > dax_cut) { continue; }
			if (fabs(day) > day_cut) { continue; }
			n++; //統計数カウント
			//if (n > n_cut) { break; }
			out_csv << dx << "," << dy << "," << dax << "," << day << ","
				<< v_txt.at(i).px << "," << v_txt.at(i).py << "," << v_txt.at(i).ax << "," << v_txt.at(i).ay << ","
				<< v_txt.at(j).px << "," << v_txt.at(j).py << "," << v_txt.at(j).ax << "," << v_txt.at(j).ay << endl;
		}
		

	}
}

