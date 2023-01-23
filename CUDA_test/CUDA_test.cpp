#include <opencv2/opencv.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/highgui.hpp>

int main() {
	cv::Mat img;
	std::string filename = "R:\\minami\\20220704_minamichem\\003\\IMAGE00_AREA-1\\png\\L0_VX0008_VY0018\\L0_VX0008_VY0018_13.png";
	img = cv::imread(filename, 0);
	cv::imshow("window", img);
	cv::waitKey(0);

}