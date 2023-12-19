#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <filesystem>

//外部インクルード
#include <nlohmann/json.hpp> //C:\Users\flab\Documents\json-develop\include

#include "TrackMatching4Aff.hpp"


using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::to_string;
using std::stoi;
using std::stod;
using std::ifstream;
using std::ofstream;
using json = nlohmann::json;

int main(int argc, char* argv[]) {
	if (argc != 5)
	{
		cerr << "exception : usage: GrainMatching.exe, basepath, nx, ny, imager_num" << endl;
		return -1;
	}
	string basepath = argv[1]; //"A:/Test/TrackMatch/0000"; //scanデータが吐き出されるところ。例えば'0000'
	int nx = stoi(argv[2]);
	int ny = stoi(argv[3]);
	int imager = stoi(argv[4]); //対象のimager番号(0～71)
	string ScanControllParam = basepath + "/ScanControllParam.json";
	string ValidViewHistry = basepath + "/ValidViewHistory.json";
	json scp, vvh;

	tmlib::read_json(ScanControllParam, scp);
	tmlib::read_json(ValidViewHistry, vvh);

	double pixel_size = 0.631; //1pixelのサイズ um/1pixel
	double step_dis = 25.0; //1steの距離 um
	float angcut = 12.0; //マッチングに使うトラックの角度カット pixel
	int phcut = 0; //マッチングに使うトラックのphcut
	int volcut = 0; //マッチングに使うトラックのvphcut
	int module = floor(imager / 12);
	int sensor = imager % 12;
	string sensor_pos = tmlib::int2string_0set(module, 2) + "_" + tmlib::int2string_0set(sensor, 2);
	string data_path = basepath + "/DATA/" + sensor_pos + "/";
	int shiftnum_x = 0, shiftnum_y = 0;
	tmlib::maxnumshift(scp, step_dis, shiftnum_x, shiftnum_y);
	int layer = (int)scp["ScanAreaParam"]["Layer"];
	double x_range = 50; //firstcutの時のx範囲 pixel
	double y_range = 50; //firstcutの時のy範囲 pixel
	double output_range = 2; //最終的にどれだけの範囲のデータを出力するか um
	


	//原点のトラックデータの読み込み
	int origin_vx = 0;
	int origin_vy = 0;
	int origin_view = origin_vy * shiftnum_x * layer + origin_vx; //基準とするview数
	string origin_data = data_path + "TrackHit2_0_" + tmlib::int2string_0set(origin_view, 8) + "_0_000.dat";
	cout << "read origin, vx: " << origin_vx << ", vy: " << origin_vy << ", Stage_X: " << vvh[origin_view]["Positions0"]["X"] << ", Stage_Y: " << vvh[origin_view]["Positions0"]["Y"] << endl;
	aff_coef aff;
	vector<phst::ClusterdMicroTrack> origin_vtrack = tmlib::read_rawfile(origin_data, angcut);

	//stageに対するpixelが±どっちなのか判断
	bool shiftpm_x = true, shiftpm_y = true;
	tmlib::shift_pm(shiftpm_x, shiftpm_y, origin_vtrack, origin_view, shiftnum_x, layer, angcut, data_path);
	cout << shiftpm_x << ", " << shiftnum_y << endl;
	

	for (int vy = 0; vy < ny; vy++)
	{
		for (int vx = 0; vx < nx; vx++)
		{
			int tar_view = vy * shiftnum_x * layer + vx; //L0側のみ
			double x_target = (double)vx * step_dis / pixel_size;
			double y_target = (double)vy * step_dis / pixel_size;
			if (!shiftpm_x) { x_target = -x_target; }
			if (!shiftpm_y) { y_target = -y_target; }
			string tar_sensor_data = data_path + "TrackHit2_0_" + tmlib::int2string_0set(tar_view, 8) + "_0_000.dat";
			cout << "read target, vx: " << vx << ", vy: " << vy << ", Stage_X: " << vvh[tar_view]["Positions0"]["X"] << ", Stage_Y: " << vvh[tar_view]["Positions0"]["Y"] << endl;
			vector<phst::ClusterdMicroTrack> tar_vtrack = tmlib::read_rawfile(tar_sensor_data, angcut);

			cout << "Track Matching" << endl;
			vector<MatchedTrack> vdtrack = tmlib::TrackMatching(origin_vtrack, tar_vtrack, x_target, y_target, x_range, y_range);
			int xbins = floor(x_range * 3 / output_range);
			int ybins = floor(y_range * 3 / output_range);
			vector<MatchedTrack> test;
			vector<MatchedTrack> output_dtrack = tmlib::search_peak(vdtrack, xbins, x_target - x_range, x_target + x_range, ybins, y_target - y_range, y_target + y_range);
			cout << "Number of Matched Track(second cut): " << output_dtrack.size() << endl;

			string out_path = basepath + "/TrackMatching4Aff/" + sensor_pos + "/v" + to_string(origin_view) + "_v" + to_string(tar_view) + ".csv";
			cout << "write csv" << endl;
			tmlib::write_csv(output_dtrack, out_path);
		}
	}
	
	return 0;
}