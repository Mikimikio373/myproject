#include <iostream>
#include <fstream>

#include "MyTrackStruct.hpp"

using std::cout;
using std::endl;
using std::string;
using std::vector;

void pixel_reduction(phst::ClusterdMicroTrack& track, double ResizeFactor, int width_in = 2048, int height_in = 1088, int width_out = 3072, int height_out = 1632) {
	//実効的なreiszeの計算
	int resize_width = std::nearbyint(width_in * ResizeFactor);
	int resize_height = std::nearbyint(height_in * ResizeFactor);
	//奇数だったら、改めて四捨五入前から差分を取り一番近い偶数になるように加減(完全奇数の1だったら2になるように)
	if (resize_width % 2 != 0) {
		if ((double)resize_width - (width_in * ResizeFactor) > 0) { resize_width--; }
		else { resize_width++; }
	}
	if (resize_height % 2 != 0) {
		if ((double)resize_height - (height_in * ResizeFactor) > 0) { resize_height--; }
		else { resize_height++; }
	}
	//実効的なresize factorの計算
	float RF_w = (float)resize_width / width_in;
	float RF_h = (float)resize_height / height_in;

	int roi_left = (width_out - resize_width) / 2;
	int roi_top = (height_out - resize_height) / 2;

	float theory_px = (track.px - roi_left) / RF_w;
	float theory_py = (track.py - roi_top) / RF_h;
	//cout << track.px << ", " << track.py << endl;
	track.px = (track.px - (float)width_out / 2) / RF_w + (float)width_in / 2;
	track.py = (track.py - (float)height_out / 2) / RF_h + (float)height_in / 2;
	track.ax /= RF_w;
	track.ay /= RF_h;
	if (track.px <0 || track.px > 2048 || track.py < 0 || track.py > 1088)
	{
		cout << track.px << ", " << track.py << endl;
	}
	//cout << track.px << ", " << track.py << endl;
}

int main() {

	int width_in = 2048;
	int height_in = 1088;
	int width_out = 3072;
	int height_out = 1632;
	double ResizeFactor = 1.41421356237309;
	std::vector<phst::ClusterdMicroTrack> vtrack;
	std::string track_path = "A:/Test/0005/DATA/00_00/TrackHit2_0_00000000_0_000.dat";

	std::ifstream track_data(track_path, std::ios::in | std::ios::binary);
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

	for (int i = 0; i < vtrack.size(); i++)
	{
		pixel_reduction(vtrack[i], ResizeFactor);
	}



	return 0;
}