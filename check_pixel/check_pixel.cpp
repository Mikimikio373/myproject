#include <filesystem>
#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>

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
	double factor_W = 1.5;
	double factor_H = 1.5;
	double a1 = 0.000631844 / factor_W;
	double b1 = -7.10246e-07 / factor_H;
	double c1 = 2.07341e-07 / factor_W;
	double d1 = -0.000631373 / factor_H;
	double under = a1 * d1 - b1 * c1;

	double a2 = d1 / under;
	double b2 = -b1 / under;
	double c2 = -c1 / under;
	double d2 = a1 / under;

	std::vector<originaltxt> vtxt;
	std::ifstream ori_txt("L0_VX0000_VY0000.txt", std::ios::in);
	for (int l = 0; l < 8; l++)
	{
		std::string out;
		ori_txt >> out;

	}
	while (1)
	{

		if (!ori_txt.eof())
		{
			originaltxt txt;
			ori_txt >> txt.px;
			ori_txt >> txt.py;
			ori_txt >> txt.ax;
			ori_txt >> txt.ay;
			ori_txt >> txt.ph;
			ori_txt >> txt.vol;
			ori_txt >> txt.num;
			ori_txt >> txt.vola;
			double tmp_x = txt.px;
			double tmp_y = txt.py;
			txt.px = a2 * tmp_x + b2 * tmp_y;
			txt.py = c2 * tmp_x + d2 * tmp_y;

			vtxt.push_back(txt);
		}
		else
		{
			break;
		}
	}
	ori_txt.close();
	std::cout << "input ended" << std::endl;

	std::ofstream outcsv("L0_VX0000_VY0000_pixel.csv");

	for (int i = 0; i < vtxt.size() - 1; i++)
	{
		outcsv << vtxt.at(i).px << "," << vtxt.at(i).py << "," << vtxt.at(i).ax << "," << vtxt.at(i).ay << "," << vtxt.at(i).ph
			<< "," << vtxt.at(i).vol << std::endl;

		if (i%1000 == 0)
		{
			std::cout << i << " /" << vtxt.size() << " ended" << std::endl;
		}
	}
	outcsv.close();
}

