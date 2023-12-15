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
	float x1 = 0;
	float y1 = 0;
	float ax1 = 0;
	float ay1 = 0;
	uint16_t ph1 = 0; /// hit数の最大値 :2
	uint16_t vol1 = 0; /// hit数の合計 :2
	float x2 = 0;
	float y2 = 0;
	float ax2 = 0;
	float ay2 = 0;
	uint16_t ph2 = 0; /// hit数の最大値 :2
	uint16_t vol2 = 0; /// hit数の合計 :2
	float dx = 0;
	float dy = 0;
	float dax = 0;
	float day = 0;
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

	void maxnumshift(json& scp, double step_dis, int &shiftnum_x, int &shiftnum_y) {
		step_dis /= 1000.0; //um2mm
		shiftnum_x = ceil((double) scp["ScanAreaParam"]["SideX"] / step_dis);
		shiftnum_y = ceil((double) scp["ScanAreaParam"]["SideY"] / step_dis);
	}

	vector<phst::ClusterdMicroTrack> read_rawfile(string file_path, float angcut = 180, uint16_t phcut = 0, uint16_t volcut = 0) {
		if (std::filesystem::exists(file_path) == false)
		{
			cerr << "There are no file: " << file_path << endl;
			exit(-2);
		}
		cout << "read: " << file_path << endl;
		vector<phst::ClusterdMicroTrack> vtrack;
		ifstream track_data(file_path, std::ios::in | std::ios::binary);
		phst::ClusterdMicroTrack track;
		int cnt = 0;
		while (1)
		{

			if (!track_data.eof())
			{
				track_data.read((char*)&track, sizeof(phst::ClusterdMicroTrack));
				cnt++;
				if (fabs(track.ax) > angcut) { continue; }
				if (fabs(track.ay) > angcut) { continue; }
				if (track.ph < phcut) { continue; }
				if (track.vol < volcut) { continue; }
				vtrack.push_back(track);
			}
			else
			{
				break;
			}
		}
		cout << "Number of all tracks: " << cnt << endl;
		cout << "Number of cut tracks: " << vtrack.size() << "\n\n" << endl;
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
	vector<MatchedTrack> TrackMatching(vector<phst::ClusterdMicroTrack>& a, vector<phst::ClusterdMicroTrack>& b, float x_center, float y_center, float x_range, float y_range) {
		vector<MatchedTrack> vdtrack;
		for (int i = 0; i < a.size(); i++)
		{
			for (int j = 0; j < b.size(); j++)
			{
				MatchedTrack dtrack;
				dtrack.dx = a[i].px - b[j].px;
				dtrack.dy = a[i].py - b[j].py;
				if (fabs(dtrack.dx - x_center) > x_range) { continue; }
				if (fabs(dtrack.dy - y_center) > y_range) { continue; }
				dtrack.dax = a[i].ax - b[j].ax;
				dtrack.day = a[i].ay - b[j].ay;
				dtrack.x1 = a[i].px;
				dtrack.y1 = a[i].py;
				dtrack.ax1 = a[i].ax;
				dtrack.ay1 = a[i].ay;
				dtrack.ph1 = a[i].ph;
				dtrack.vol1 = a[i].vol;
				dtrack.x2 = b[j].px;
				dtrack.y2 = b[j].py;
				dtrack.ax2 = b[j].ax;
				dtrack.ay2 = b[j].ay;
				dtrack.ph2 = b[j].ph;
				dtrack.vol2 = b[j].vol;
				vdtrack.push_back(dtrack);
			}
		}
		cout << "Number of Matched Track(first cut): " << vdtrack.size() << endl;
		return vdtrack;
	}

	vector<MatchedTrack> search_peak(const vector<MatchedTrack>& input, int xbin, float xmin, float xmax, int ybin, float ymin, float ymax)
	{
		float x_wbin = (xmax - xmin) / (float) xbin;
		float y_wbin = (ymax - ymin) / (float) ybin;
		vector<vector<vector<MatchedTrack>>> box(xbin, vector<vector<MatchedTrack>>(ybin));	//後でbinの中のトラックデータをvectorで渡したいから3次元
		cout << xbin << ", " << ybin << endl;
		for (int i = 0; i < input.size(); i++)
		{
			//範囲外にあったらpass
			if (input[i].dx < xmin || input[i].dx > xmax || input[i].dy < ymin || input[i].dy > ymax) { continue; }
			int a = floor((input[i].dx - xmin) / x_wbin); //代入するbinの計算
			int b = floor((input[i].dy - ymin) / y_wbin);
			//たまに計算誤差で範囲外に出てしまうため例外処理(要修正？難しいかも)
			if (a < 0 || a >= xbin) { continue; }
			if (b < 0 || b >= ybin) { continue; }
			box[a][b].push_back(input[i]); //bin詰め
		}
		int maxbinsize = 0;
		int maxbinx = 0;
		int	maxbiny = 0;
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
		vector<MatchedTrack> output = box[maxbinx][maxbiny];
		for (int i = maxbinx -1; i < maxbinx + 2; i++)
		{
			for (int j = maxbiny - 1; j < maxbiny + 2; j++)
			{
				if (i < 0) { continue; }
				if (j < 0) { continue; }
				if (i == 0 && j == 0) { continue; }
				output.insert(output.end(), box[i][j].begin(), box[i][j].end());
			}
		}

		return output;
	}

	void search_peak_pos(const vector<MatchedTrack>& input, int xbin, double xmin, double xmax, int ybin, double ymin, double ymax, double &binpmX, double &binpmY)
	{
		double x_wbin = (xmax - xmin) / (double)xbin;
		double y_wbin = (ymax - ymin) / (double)ybin;
		vector<vector<int>> box(xbin, vector<int>(ybin, 0));	//後でbinの中のトラックデータをvectorで渡したいから3次元
		for (int i = 0; i < input.size(); i++)
		{
			//範囲外にあったらpass
			if (input[i].dx < xmin || input[i].dx > xmax || input[i].dy < ymin || input[i].dy > ymax) { continue; }
			int a = floor((input[i].dx - xmin) / x_wbin); //代入するbinの計算
			int b = floor((input[i].dy - ymin) / y_wbin);
			box[a][b] += 1; //bin詰め
		}
		int maxbinsize = 0;
		int maxbinx = 0;
		int	maxbiny = 0;
		for (int i = 0; i < xbin; i++)
		{
			for (int j = 0; j < ybin; j++)
			{
				int size = box[i][j];
				if (size > maxbinsize)
				{
					maxbinsize = size;
					maxbinx = i;
					maxbiny = j;
				}
			}
		}
		binpmX = (double) maxbinx - xbin / 2;
		binpmY = (double) maxbiny - ybin / 2;
	}

	//originの位置からX,Yプラス1step動かしたときとマッチングをした時に±どっちに動くかを調べる。boolはプラスならtrue, マイナスならfalse
	void shift_pm(bool& shiftX_pm, bool& shiftY_pm, vector< phst::ClusterdMicroTrack> &origin_vtrack, int origin_view, int shiftnum_x, int layer, float angcut, string data_path, int binX = 25, float xrange = 50, int binY = 25, float yrange = 50) {
		cout << "chech Plus Minus" << endl;
		int view_puls = origin_view + shiftnum_x * layer + 1;
		string data = data_path + "TrackHit2_0_" + tmlib::int2string_0set(view_puls, 8) + "_0_000.dat";
		vector<phst::ClusterdMicroTrack> plus_vtrack = tmlib::read_rawfile(data, angcut);
		vector<MatchedTrack> puls_match = TrackMatching(origin_vtrack, plus_vtrack, 0, 0, xrange, yrange);
		double pmX = 0, pmY = 0;
		search_peak_pos(puls_match, binX, -xrange, xrange, binY, -yrange, yrange, pmX, pmY);
		shiftX_pm = (pmX > 0);
		shiftY_pm = (pmY > 0);
	}

	void write_csv(vector<MatchedTrack>& input, string out_path) {
		FILE* fp = std::fopen(out_path.c_str(), "wb");
		//一行目(ラベルの記述)
		fprintf(fp, "dx/D,dy/D,dax/D,day/D,x1/D,y1/D,ax1/D,ax1/D,ph1/I,vol1/I,x2/D,y2/D,ax2/D,ay2/D,ph2/I,vol2/I\r\n");

		auto sz = input.size();
		for (int i = 0; i < sz; i++)
		{
			fprintf(fp, "%g,%g,%g,%g,%g,%g,%g,%g,%d,%d,%g,%g,%g,%g,%d,%d\r\n", input[i].dx, input[i].dy, input[i].dax, input[i].day, input[i].x1, input[i].y1, input[i].ax1, input[i].ay1, input[i].ph1, input[i].vol1, input[i].x2, input[i].y2, input[i].ax2, input[i].ay2, input[i].ph2, input[i].vol2);
		}
		fclose(fp);
		cout << out_path << " written\n\n" << endl;
	}

	void write_csv_mt(vector<MicroTrack>& input, string out_path) {
		FILE* fp = std::fopen(out_path.c_str(), "wb");
		//一行目(ラベルの記述)
		fprintf(fp, "px/D,py/D,ax/D,ay/D,ph/I,vol/I\r\n");

		auto sz = input.size();
		for (int i = 0; i < sz; i++)
		{
			fprintf(fp, "%g,%g,%g,%g,%d,%d\r\n", input[i].x, input[i].y, input[i].ax, input[i].ay, input[i].ph, input[i].vol);
		}
		fclose(fp);
	}
}
