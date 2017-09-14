// 图像提取.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/core/ocl.hpp"
using namespace cv;

#include <vector>
#include <iostream>
using namespace std;


int main()
{
	if(false)
	{
		// 车牌号 plate number
		Mat img, gray;
		img = imread("10.jpg", 1);
		cvtColor(img, gray, COLOR_BGR2GRAY);
		GaussianBlur(gray, gray, Size(7, 7), 1.5);
		Mat canny_gray;
		Canny(gray, canny_gray, 0, 50);			// Canny边缘检测
		imshow("canny", canny_gray);

		Mat grad_x;
		Mat abs_grad_dx2;
		Sobel(gray, grad_x, CV_16S, 2, 0, 3);	 // 单独求 x方向梯度  x阶数ddepth=2   size=3 
		// Sobel(gray, grad_x, CV_16S, 1, 0, 3);
		convertScaleAbs(grad_x, abs_grad_dx2);
		imshow("sobel_dx2", abs_grad_dx2);

		Mat grad_y;
		Mat abs_grad_dy2;
		Sobel(gray, grad_y, CV_16S, 0, 2, 3);
		convertScaleAbs(grad_y, abs_grad_dy2);	// 1.dst = src * alpha + belta 2.转换成绝对值 3.转换成8bit 超过截断到256
		imshow("sobel_dy2", abs_grad_dy2);

		Mat grad;
		addWeighted(abs_grad_dx2, 0.5, abs_grad_dy2, 0.5, 0, grad);
		imshow("grad_dx2_dy2", grad);			// 合并梯度(近似)  ( ||x|| + ||y|| ) / 2 	

		Mat gradThre;
		double act_threshold = threshold(grad, gradThre, 80, 255, THRESH_BINARY | THRESH_OTSU);
		char name[256]; sprintf(name,"%s-计算阀值=%f" ,"大津法二值化" , act_threshold ); //act_threshold = 13 
		imshow(name,gradThre);


		waitKey(0);
	}

	//if(false)
	{
		Mat img, gray;
		img = imread("1.jpg", 1);
		cvtColor(img, gray, COLOR_BGR2GRAY);			 
		Mat gaussian;
		GaussianBlur(gray, gaussian, Size(3, 3), 0);	gray.release();
		Mat median;
		medianBlur(gaussian, median, 5);				gaussian.release();
		Mat sobel;
		Sobel(median, sobel, CV_8U, 1, 0, 3);			median.release();
		
		Mat binary;
		threshold(sobel, binary, 170, 255, THRESH_BINARY); sobel.release();

		//imshow("二值化", binary);

		// 膨胀和腐蚀操作的核函数
		Mat element1 = getStructuringElement(MORPH_RECT, cv::Size(9, 1));// 形态学处理的核心就是定义结构元素
		Mat element2 = getStructuringElement(MORPH_RECT, cv::Size(8, 6));
		// 膨胀一次，让轮廓突出
		Mat dilation;
		dilate(binary, dilation, element2, Point(-1, -1), 1 /* iterations */); binary.release();
		//imshow("膨胀一次", dilation);
		// 腐蚀一次，去掉细节
		Mat erosion;
		erode(dilation, erosion, element1, Point(-1, -1), 1); dilation.release();
		//imshow("腐蚀一次", erosion);
		// 再次膨胀，让轮廓明显一些
		Mat dilation2;
		dilate(erosion, dilation2, element2, Point(-1, -1), 3); erosion.release();

		imshow("形态学-膨胀和腐蚀", dilation2);

		vector< vector<Point> > contours;
		vector<Vec4i> hierarchy;
		findContours(dilation2, contours, hierarchy, CV_RETR_EXTERNAL /*CV_RETR_TREE*/, CV_CHAIN_APPROX_SIMPLE, Point(0, 0)/*offset*/);
		Scalar color = CV_RGB(255, 0, 0);
		for (int i = 0; i < contours.size(); i++)
		{
			Rect aRect = boundingRect( contours[i] ); // 使用边界框的方式  
			int tmparea = aRect.height * aRect.height;
			if (((double)aRect.width / (double)aRect.height > 2) 
				&& ((double)aRect.width / (double)aRect.height < 6 )	// 车牌特点: 比例
				&& tmparea >= 200 && tmparea <= 25000 ){				// 车牌特点: 车牌 面积 
				rectangle(img,  // 画矩形框 
					cvPoint(aRect.x, aRect.y),  /* 左上角 右小角*/
					cvPoint(aRect.x + aRect.width, aRect.y + aRect.height), 
					color, 2 /*线宽*/);
			}
		}
		imshow("检测图片 " , img);

		/*
			4.jpg  10.jpg 效果OK
			2.jpg  5.jpg  6.jpg  车牌会分成两段 
			1.jpg  7.jpg  车牌不完整提取
			3.jpg  车牌分成很多小段 每个字都是一段 所以没法锁定车牌
		*/
		waitKey(0);
	}


	// 测试 convertScaleAbs 函数
	cv::Mat m1(1, 2, CV_16S, cv::Scalar(0)); // height 1 width 2 
	m1.at<int16_t>(0, 0) = -800;
	m1.at<int16_t>(0, 1) = 340;
	cout << "height  " << m1.rows << " width " << m1.cols << std::endl;
	Mat m2;
	convertScaleAbs(m1, m2); // 
	std::cout << "ScaleAbs " << m2 << std::endl;
	// -120,120		--> 120 , 120 
	// -250,120		--> 250 , 120
	// -300,120		--> 255 , 120 
	// -800,340		--> 255 , 255 

    return 0;
}

