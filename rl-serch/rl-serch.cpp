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

class calc_rl
{
public:
	void calc(double ax, double ay);
	double ux, uy, vx, vy;
};

void calc_rl::calc(double ax, double ay)
{
	double nolm = sqrt(ax * ax + ay * ay);
	ux = ax / nolm;
	uy = ay / nolm;
	vx = -uy;
	vy = ux;
}

rl_data substitution(originaltxt pred, originaltxt eval, calc_rl rl)
{
	rl_data out;
	out.x = pred.px;
	out.y = pred.py;
	out.ax = pred.ax;
	out.ay = pred.ay;
	out.predph = pred.ph;
	out.predvol = pred.vol;
	out.drR = (eval.px - pred.px) * rl.ux + (eval.py - pred.py) * rl.uy;
	out.drL = (eval.px - pred.px) * rl.vx + (eval.py - pred.py) * rl.vy;
	out.dtR = (eval.ax - pred.ax) * rl.ux + (eval.ay - pred.ay) * rl.uy;
	out.dtL = (eval.ax - pred.ax) * rl.vx + (eval.ay - pred.ay) * rl.vy;
	out.ph = eval.ph;
	out.vol = eval.vol;

	return out;
}



int main()
{

	originaltxt txt;
	std::vector<originaltxt> v_txt;
	string ori_txtname = "L0_VX0012_VY0027.txt";
	string out_csv_name = "..\\rl-serch.csv";
	std::ifstream ori_txt(ori_txtname, std::ios::in);

	double drR_cut = 30;
	double drL_cut = 30;
	double dtR_cut = 0.7;
	double dtL_cut = 0.3;
	int n_cut = 2000;
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

	int flag[100];
	for (int n = 0; n < 100; n++)
	{
		flag[n] = 0;
	}


	for (int i = 0; i < v_txt.size(); i++)
	{
		if (v_txt.at(i).ax == 0 && v_txt.at(i).ay == 0) { continue; }
		if (sqrt(v_txt.at(i).ax * v_txt.at(i).ax + v_txt.at(i).ay * v_txt.at(i).ax) < 0.4 && v_txt.at(i).vol <= 3) { continue; }
		calc_rl pred;
		pred.calc(v_txt.at(i).ax, v_txt.at(i).ay);
		//cout << v_txt.at(i).ax << v_txt.at(i).ay << endl;
		//cout << pred.ux << pred.uy << pred.vx << pred.vy << endl;
		int n = 0;
		//double pred_axy = sqrt(v_txt.at(i).ax * v_txt.at(i).ax + v_txt.at(i).ay * v_txt.at(i).ay);
		//int flag_n = (int)floor(pred_axy / 0.05);
		//flag[flag_n] += 1;
		//if (flag[flag_n] > 50) { continue; }
		//cout << flag[flag_n] << endl;
		for (int j = i + 1; j < v_txt.size(); j++)
		{
			if (sqrt(v_txt.at(j).ax * v_txt.at(j).ax + v_txt.at(j).ay * v_txt.at(j).ax) < 0.4 && v_txt.at(j).vol <= 3) { continue; }
			rl_data rl_track = substitution(v_txt.at(i), v_txt.at(j), pred);
			
			//領域外のものを飛ばす
			if (fabs(rl_track.drR) > drR_cut) { continue; }
			if (fabs(rl_track.drL) > drL_cut) { continue; }
			if (fabs(rl_track.dtR) > dtR_cut) { continue; }
			if (fabs(rl_track.dtL) > dtL_cut) { continue; }
			n++; //統計数カウント
			//if (n > n_cut) { break; }
			out_csv << rl_track.x << "," << rl_track.y << "," << rl_track.ax << "," << rl_track.ay << ","
				<< rl_track.predph << "," << rl_track.predvol << "," << rl_track.drR << "," << rl_track.drL << ","
				<< rl_track.dtR << "," << rl_track.dtL << "," << rl_track.ph << "," << rl_track.vol<<"," << v_txt.at(j).ax << "," << v_txt.at(j).ay << endl;
		}
		if (i % 1000 == 0)
		{
			cout << "line" << i << "ended" << endl;
		}
		
	}
}

