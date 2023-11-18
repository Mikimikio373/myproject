#pragma once
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
using std::ifstream;
using std::ofstream;
using json = nlohmann::json;

int main() {
	string basepath = "A:/Test/20231020unit4-88_1/";
	string AffineParam = basepath + "AffineParam_origin.json";
	string EachViewParam = basepath + "EachViewParam.json";
	string ValidViewHistry = basepath + "ValidViewHistory.json";
	json aff_param, evp, vvh;

	tmlib::read_json(AffineParam, aff_param);
	tmlib::read_json(EachViewParam, evp);
	tmlib::read_json(ValidViewHistry, vvh);

	//ひとまずテストでview=374to375を使う
	int sensor_num = 24;
	int imager1 = 11;
	int imager2 = 12;
	int view1 = 374;
	int view2 = 375;
	string right_sensor_data = basepath + "DATA/00_11/TrackHit2_0_" + tmlib::int2string_0set(view1, 8) + "_0_000.dat";
	string left_sensor_data = basepath + "DATA/01_00/TrackHit2_0_" + tmlib::int2string_0set(view2, 8) + "_0_000.dat";

	int index1 = sensor_num * (view1 - 1) + view1;
	cout << vvh[view1]["ScanEachLayerParam"]["ThickOfLayer"] << endl;
	double thickness1 = vvh[view1]["ScanEachLayerParam"]["ThickOfLayer"];
	cout << vvh[view1]["ScanLines"]["X"] << endl;
	double sx = vvh[view1]["ScanLines"]["X"];
	cout << vvh[view1]["ScanLines"]["Y"] << endl;
	double sy = vvh[view1]["ScanLines"]["Y"];

	aff_coef aff1 = tmlib::read_affparam(aff_param, imager1);

	vector<phst::ClusterdMicroTrack> vtrack1 = tmlib::read_rawfile(right_sensor_data);
	cout << vtrack1.size() << endl;
	
	vector<MicroTrack> trance_vtrack = tmlib::trance_affine(vtrack1, aff1, sx, sy, thickness1);

	for (int i = 0; i < 10; i++)
	{
		cout << trance_vtrack[i].ax << trance_vtrack[i].ay << endl;
	}
}