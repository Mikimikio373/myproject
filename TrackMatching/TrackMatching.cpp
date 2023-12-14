#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <filesystem>

//外部インクルード
#include <nlohmann/json.hpp> //C:\Users\flab\Documents\json-develop\include

#include "TrackMatching.hpp"


using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::to_string;
using std::ifstream;
using std::ofstream;
using json = nlohmann::json;

int main() {
	string basepath = "A:/Test/check_stage/20231111_graine_acrylic/";
	string AffineParam = basepath + "AffineParam_origin.json";
	string EachViewParam = basepath + "EachViewParam.json";
	string ValidViewHistry = basepath + "ValidViewHistory.json";
	json aff_param, evp, vvh;

	tmlib::read_json(AffineParam, aff_param);
	tmlib::read_json(EachViewParam, evp);
	tmlib::read_json(ValidViewHistry, vvh);

	//ひとまずテストでview=3844to3845を使う
	//"Nhit": 138,
	//"layer" : 0,
	//"shift_param_x" : [
	//	47.32843777,
	//		0.001813865944,
	//		0.0002278699633
	//] ,
	//"shift_param_y" : [
	//	44.89007301,
	//		0.002010930628,
	//		0.0002491777203
	//] ,
	//		"shift_x" : 0.001813865944,
	//		"shift_y" : 0.002010930628,
	//		"stage_x1" : 36,
	//		"stage_x2" : 45,
	//		"stage_y1" : 77.975,
	//		"stage_y2" : 77.975
	int sensor_num = 24;
	int imager1 = 11;
	int imager2 = 15;
	int view1 = 3844;
	int view2 = 3844;
	string right_sensor_data = basepath + "DATA/00_11/TrackHit2_0_" + tmlib::int2string_0set(view1, 8) + "_0_000.dat";
	string left_sensor_data = basepath + "DATA/01_00/TrackHit2_0_" + tmlib::int2string_0set(view2, 8) + "_0_000.dat";

	int index1 = sensor_num * (view1 - 1) + view1;
	cout << "view1" << endl;
	cout << vvh[view1]["ScanEachLayerParam"]["ThickOfLayer"] << endl;
	double thickness1 = vvh[view1]["ScanEachLayerParam"]["ThickOfLayer"];
	thickness1 = thickness1 / 1000.0; // um to mm
	cout << vvh[view1]["ScanLines"]["X"] << endl;
	double sx1 = vvh[view1]["ScanLines"]["X"];
	cout << vvh[view1]["ScanLines"]["Y"] << endl;
	double sy1 = vvh[view1]["ScanLines"]["Y"];

	int index2 = sensor_num * (view2 - 1) + view2;
	cout << "view2" << endl;
	cout << vvh[view2]["ScanEachLayerParam"]["ThickOfLayer"] << endl;
	double thickness2 = vvh[view2]["ScanEachLayerParam"]["ThickOfLayer"];
	thickness2 = thickness2 / 1000.0; // um to mm
	cout << vvh[view2]["ScanLines"]["X"] << endl;
	double sx2 = vvh[view2]["ScanLines"]["X"];
	cout << vvh[view2]["ScanLines"]["Y"] << endl;
	double sy2 = vvh[view2]["ScanLines"]["Y"];

	aff_coef aff1 = tmlib::read_affparam(aff_param, imager1);
	aff_coef aff2 = tmlib::read_affparam(aff_param, imager2);

	vector<phst::ClusterdMicroTrack> vtrack1 = tmlib::read_rawfile(right_sensor_data);
	cout << vtrack1.size() << endl;
	vector<phst::ClusterdMicroTrack> vtrack2 = tmlib::read_rawfile(left_sensor_data);
	cout << vtrack2.size() << endl;
	
	vector<MicroTrack> trance_vtrack1 = tmlib::trance_affine(vtrack1, aff1, sx1, sy1, thickness1, 0.1);
	cout << trance_vtrack1.size() << endl;
	vector<MicroTrack> trance_vtrack2 = tmlib::trance_affine(vtrack2, aff2, sx2, sy2, thickness1, 0.1);
	cout << trance_vtrack2.size() << endl;

	cout << "Track Matching" << endl;
	vector<MatchedTrack> vdtrack = tmlib::track_matching(trance_vtrack1, trance_vtrack2, 2000);
	cout << vdtrack.size() << endl;

	string out_path = basepath + "/TrackMatching_v" + to_string(view1) + "-i" + to_string(imager1) + "_v" + to_string(view2) + "-i" + to_string(imager2) + ".csv";
	cout << "write csv" << endl;
	tmlib::write_csv(vdtrack, out_path);
}