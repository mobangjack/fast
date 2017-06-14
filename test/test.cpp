#include <vector>
#include <iostream>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <fast/fast.h>

static inline void fastXY2KeyPoint(const fast::fast_xy& fast_xy, cv::KeyPoint& keypoint)
{
	keypoint.pt.x = fast_xy.x;
   keypoint.pt.y = fast_xy.y;
}

static inline void fastXYCorners2KeyPoints(const std::vector<fast::fast_xy> corners, std::vector<cv::KeyPoint>& keypoints)
{
	for (uint32_t i = 0; i < corners.size(); i++)
	{
		cv::KeyPoint keypoint;
   	keypoint.pt.x = corners[i].x;
   	keypoint.pt.y = corners[i].y;
      keypoints.push_back(keypoint);
   }
}

int main (int argc, char * argv[]) {
	cv::VideoCapture cap(0);
	if (!cap.isOpened()) {
		printf("fail to open camera\n");
		return -1;
	}
	cv::Mat img;
	std::vector<fast::fast_xy> corners;
	std::vector<cv::KeyPoint> keypoints;
	double tc = 0, dt = 0;
	while (1) {
		cap >> img;
		if (img.empty()) {
			break;
		}
		corners.clear();
		keypoints.clear();
		cv::resize(img, img, cv::Size(640, 480));
		cv::cvtColor(img, img, CV_BGR2GRAY);
		tc = (double)cv::getTickCount();
      fast::fast_corner_detect_10_sse2((fast::fast_byte *)(img.data), img.cols, img.rows, img.cols, 75, corners);
      dt = ((double)cv::getTickCount() - tc) / cv::getTickFrequency() * 1000;
      std::cout << dt << " ms/f" << std::endl;
      fastXYCorners2KeyPoints(corners, keypoints);
      //cv::Mat outImg;
      //img.copyTo(outImg);
   	drawKeypoints(img, keypoints, img, cv::Scalar(255, 0, 0));
   	cv::imshow("fast result", img);
   	if (cv::waitKey(1) == 'q') break;
	}
	return 0;
}
	
int __main (int argc, char * argv[]) {
	const std::string imgfilepath = std::string(argv[1]); //"/home/bj/Pictures/Webcam/2017-06-14-125417.jpg"; //std::string(TEST_DATA_DIR) + "/test1.png"
   const int n_trials = 1000;
   std::vector<fast::fast_xy> corners;
   cv::Mat img = cv::imread(imgfilepath, 0);
   cv::resize(img, img, cv::Size(640, 480));
   cv::Mat outImg;
   //cv::Mat downSampled; 
   //cv::resize(img, img, cv::Size(752, 480));
   //img = downSampled;

   printf("\nTesting PLAIN version\n");
   double time_accumulator = 0;
   for (int i = 0; i < n_trials; ++i) {
      corners.clear();
      double t = (double)cv::getTickCount();
      fast::fast_corner_detect_10((fast::fast_byte *)(img.data), img.cols, img.rows, img.cols, 75, corners);
      time_accumulator +=  ((cv::getTickCount() - t) / cv::getTickFrequency());
   }
   std::vector<cv::KeyPoint> keypoints;
   fastXYCorners2KeyPoints(corners, keypoints);
   drawKeypoints(img, keypoints, outImg, cv::Scalar(255, 0, 0));
   cv::imshow("fast plain result", outImg);
   printf("PLAIN took %f ms (average over %d trials).\n", time_accumulator/((double)n_trials)*1000.0, n_trials );
   printf("PLAIN version extracted %zu features.\n", corners.size());

#if __NEON__
   printf("\nTesting NEON version\n");
   time_accumulator = 0;
   for (int i = 0; i < n_trials; ++i) {
     corners.clear();
      double t = (double)cv::getTickCount();
      fast::fast_corner_detect_9_neon((fast::fast_byte *)(img.data), img.cols, img.rows, img.cols, 75, corners);
      time_accumulator +=  ((cv::getTickCount() - t) / cv::getTickFrequency());
   }
   printf("NEON version took %f ms (average over %d trials).\n", time_accumulator/((double)n_trials)*1000.0, n_trials);
   printf("NEON version extracted %zu features.\n", corners.size());
#endif
   
#if __SSE2__
   printf("\nTesting SSE2 version\n");
   time_accumulator = 0;
   for (int i = 0; i < n_trials; ++i) {
     corners.clear();
      double t = (double)cv::getTickCount();
      fast::fast_corner_detect_10_sse2((fast::fast_byte *)(img.data), img.cols, img.rows, img.cols, 75, corners);
      time_accumulator +=  ((cv::getTickCount() - t) / cv::getTickFrequency());
   }
   fastXYCorners2KeyPoints(corners, keypoints);
   drawKeypoints(img, keypoints, outImg, cv::Scalar(255, 0, 0));
   cv::imshow("fast sse2 result", outImg);
   printf("SSE2 version took %f ms (average over %d trials).\n", time_accumulator/((double)n_trials)*1000.0, n_trials);
   printf("SSE2 version extracted %zu features.\n", corners.size());
#endif

   printf("\nBENCHMARK version extracted 167 features.\n");
   cv::waitKey(0);
   return 0;
}
