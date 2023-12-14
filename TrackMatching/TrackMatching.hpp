#pragma once
#include <stdio.h>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <sstream>

#include <nlohmann/json.hpp> //C:\Users\flab\Documents\json-develop\include

using std::cout;
using std::endl;
using std::cerr;
using std::vector;
using std::string;
using std::ifstream;
using std::ofstream;
using json = nlohmann::json;

class MicroTrack
{
public:
	double x = 0;
	double y = 0;
	double ax = 0;
	double ay = 0;
	uint16_t ph = 0; /// hit数の最大値 :2
	uint16_t vol = 0; /// hit数の合計 :2
};

class MatchedTrack
{
public:
	double x1 = 0;
	double y1 = 0;
	double ax1 = 0;
	double ay1 = 0;
	uint16_t ph1 = 0; /// hit数の最大値 :2
	uint16_t vol1 = 0; /// hit数の合計 :2
	double x2 = 0;
	double y2 = 0;
	double ax2 = 0;
	double ay2 = 0;
	uint16_t ph2 = 0; /// hit数の最大値 :2
	uint16_t vol2 = 0; /// hit数の合計 :2
	double dx = 0;
	double dy = 0;
	double dax = 0;
	double day = 0;
};

class aff_coef
{
public:
	double a = 1;
	double b = 0;
	double c = 0;
	double d = 1;
	double p = 0;
	double q = 0;
};

namespace phst {
	class ClusterdMicroTrack // 24byte
	{
	public:
		float px = 0; // pixel center is the 7th layer(0, 1, ...) :4
		float py = 0; // pixel center is the 7th layer(0, 1, ...) :4
		float ax = 0; // pixel / 15 layer :4
		float ay = 0; // pixel / 15 layer :4
		uint16_t ph = 0; /// hit数の最大値 :2
		uint16_t num = 0; /// 空 :2
		uint16_t vol = 0; /// hit数の合計 :2
		uint16_t vola = 0; /// 空
		std::string object()const { return "px/F:py:ax:ay:ph/s:num:vol:vola"; }
		std::string name()const { return "ClusterdMicroTrack"; }
		static std::string first_line() { return "#px[pix] py[pix] ax[pix/15pic] ay[pix/15pic] ph num vol vola"; }
		int pc_vol()const { return vola; }
		int pc_ph()const { return num & 0x1f; }
		int pc_area()const { return num >> 5; }
		void get_rotation(double rotation[2])const
		{
			const double rad = -atan2(ay, ax);
			rotation[0] = cos(rad);
			rotation[1] = sin(rad);
		}
		bool operator==(const ClusterdMicroTrack& lhs) const {
			if (lhs.px != this->px)return false;
			if (lhs.py != this->py)return false;
			if (lhs.ax != this->ax)return false;
			if (lhs.ay != this->ay)return false;
			if (lhs.ph != this->ph)return false;
			if (lhs.num != this->num)return false;
			if (lhs.vol != this->vol)return false;
			if (lhs.vola != this->vola)return false;
			return true;
		}
		//radial方向(飛跡をX軸方向に向けたときのX方向)
		double rax(double rotation[])const
		{
			return ax * rotation[0] + ay * -rotation[1];
		}
		//lateral方向(飛跡をX軸方向に向けたときのX方向)
		double ray(double rotation[])const
		{
			return ax * rotation[1] + ay * rotation[0];
		}
		//radial方向(飛跡をX軸方向に向けたときのX方向)
		double rpx(double px, double py, double rotation[])const
		{
			return (this->px - px) * rotation[0] + (this->py - py) * -rotation[1];
		}
		//lateral方向(飛跡をX軸方向に向けたときのX方向)
		double rpy(double px, double py, double rotation[])const
		{
			return (this->px - px) * rotation[1] + (this->py - py) * rotation[0];
		}
		static void rl2xy(const double& r, const double& l, double rotation[2], double& x, double& y) {
			x = r * rotation[0] * l * rotation[1];
			y = r * -rotation[1] * l * rotation[0];
		}
	};
}

namespace tools
{
	//stringをある特定の一文字で分割する関数(ネットでいくらでも転がっているテンプレ)
	vector<string> split(string& input, char delimiter)
	{
		std::istringstream stream(input);
		string field;
		vector<string> result;
		while (getline(stream, field, delimiter)) {
			result.push_back(field);
		}
		return result;
	};

	double get_vec_mean(vector<double>& input) {
		double mean = std::accumulate(input.begin(), input.end(), 0) / input.size();
		return mean;
	};
}


namespace tmlib
{
	string int2string_0set(int input, int digit) {
		std::ostringstream ss;
		ss << std::setw(digit) << std::setfill('0') << input;
		string s(ss.str());

		return s;
	}

	void read_json(string json_path, json& out_json) {
		if (std::filesystem::exists(json_path) == false)
		{
			cerr << "There are no file: " << json_path << endl;
			exit(-2);
		}

		ifstream ifs(json_path, std::ios::in);
		cout << "read: " << json_path << endl;
		ifs >> out_json;
		ifs.close();
	}

	aff_coef read_affparam(json& aff_param, int imagerID) {
		aff_coef out;
		out.a = aff_param[imagerID]["Aff_coef"][0];
		out.b = aff_param[imagerID]["Aff_coef"][1];
		out.c = aff_param[imagerID]["Aff_coef"][2];
		out.d = aff_param[imagerID]["Aff_coef"][3];
		out.p = aff_param[imagerID]["Aff_coef"][4];
		out.q = aff_param[imagerID]["Aff_coef"][5];

		return out;
	}

	vector<phst::ClusterdMicroTrack> read_rawfile(string file_path) {
		if (std::filesystem::exists(file_path) == false)
		{
			cerr << "There are no file: " << file_path << endl;
			exit(-2);
		}
		cout << "read: " << file_path << endl;
		vector<phst::ClusterdMicroTrack> vtrack;
		ifstream track_data(file_path, std::ios::in | std::ios::binary);
		phst::ClusterdMicroTrack track;
		while (1)
		{

			if (!track_data.eof())
			{
				track_data.read((char*)&track, sizeof(phst::ClusterdMicroTrack));
				vtrack.push_back(track);
			}
			else
			{
				break;
			}
		}
		return vtrack;
	}

	vector<MicroTrack> trance_affine(vector<phst::ClusterdMicroTrack>& input, aff_coef& aff, double sx, double sy, double thickness, double ang_cut = 1.5) {
		double mid_width = 2048.0 / 2.0;
		double mid_height = 1088.0 / 2.0;
		vector<MicroTrack> out;
		for (int i = 0; i < input.size(); i++)
		{
			MicroTrack track;
			track.x = aff.a * (double)(input[i].px - mid_width) + aff.b * (double)(input[i].py - mid_height) + aff.p + sx;
			track.y = aff.c * (double)(input[i].px - mid_width) + aff.d * (double)(input[i].py - mid_height) + aff.q + sy;
			track.ax = aff.a * (double)input[i].ax + aff.b * (double)input[i].ay / thickness;
			track.ay = aff.c * (double)input[i].ax + aff.d * (double)input[i].ay / thickness;
			track.ph = input[i].ph;
			track.vol = input[i].vol;
			if (fabs(track.ax) > ang_cut) { continue; }
			if (fabs(track.ay) > ang_cut) { continue; }
			out.push_back(track);
		}
		return out;
	}

	// a - b
	vector<MatchedTrack> track_matching(vector<MicroTrack>& a, vector<MicroTrack>& b, double dpos_cut) {
		vector<MatchedTrack> vdtrack;
		for (int i = 0; i < a.size(); i++)
		{
			for (int j = 0; j < b.size(); j++)
			{
				MatchedTrack dtrack;
				dtrack.dx = a[i].x - b[j].x;
				dtrack.dy = a[i].y - b[j].y;
				if (fabs(dtrack.dx) > dpos_cut) { continue; }
				if (fabs(dtrack.dy) > dpos_cut) { continue; }
				dtrack.dax = a[i].ax - b[j].ax;
				dtrack.day = a[i].ay - b[j].ay;
				dtrack.x1 = a[i].x;
				dtrack.y1 = a[i].y;
				dtrack.ax1 = a[i].ax;
				dtrack.ay1 = a[i].ay;
				dtrack.ph1 = a[i].ph;
				dtrack.vol1 = a[i].vol;
				dtrack.x2 = b[j].x;
				dtrack.y2 = b[j].y;
				dtrack.ax2 = b[j].ax;
				dtrack.ay2 = b[j].ay;
				dtrack.ph2 = b[j].ph;
				dtrack.vol2 = b[j].vol;
				vdtrack.push_back(dtrack);
			}
		}

		return vdtrack;
	}

	void write_csv(vector<MatchedTrack> &input, string out_path) {
		FILE* fp = std::fopen(out_path.c_str(), "wb");
		//一行目(ラベルの記述)
		fprintf(fp, "dx/D,dy/D,dax/D,day/D,x1/D,y1/D,ax1/D,ax1/D,ph1/I,vol1/I,x2/D,y2/D,ax2/D,ay2/D,ph2/I,vol2/I");

		auto sz = input.size();
		for (int i = 0; i < sz; i++)
		{
			fprintf(fp, "%g,%g,%g,%g,%g,%g,%g,%g,%d,%d,%g,%g,%g,%g,%d,%d\r\n", input[i].dx, input[i].dy, input[i].dax, input[i].day, input[i].x1, input[i].y1, input[i].ax1, input[i].ay1, input[i].ph1, input[i].vol1, input[i].x2, input[i].y2, input[i].ax2, input[i].ay2, input[i].ph2, input[i].vol2);
		}
		fclose(fp);
	}
}
