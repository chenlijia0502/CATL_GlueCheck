#include "stdafx.h"
#include "CalcuSurfacePlaneness.h"
#include "global.h"
#include "KxReadXml2.h"


CCalcuPlaneness::CCalcuPlaneness()
{

}

CCalcuPlaneness::~CCalcuPlaneness()
{

}


void CCalcuPlaneness::CalcuSurfacePlaness(const cv::Mat& points,  cv::Mat& plane)
{
	/*
	 *  最小二乘拟合平面，平面方程：Ax+By+Cz=D
	 *  A = plane.at<float>(0,0)
	 *  B = plane.at<float>(1,0)
	 *  C = plane.at<float>(2,0)
	 *  D = plane.at<float>(3,0)
	*/
	int rows = points.rows;
	int cols = points.cols;

	cv::Mat centroid = cv::Mat::zeros(1, cols, CV_32FC1);
	for (int i = 0; i < cols; i++) {
		for (int j = 0; j < rows; j++) {
			centroid.at<float>(0, i) += points.at<float>(j, i);
		}
		centroid.at<float>(0, i) /= rows;

	}

	cv::Mat points2 = cv::Mat::ones(rows, cols, CV_32FC1);
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			points2.at<float>(i, j) = points.at<float>(i, j) - centroid.at<float>(0, j);
		}
	}
	cv::Mat A, W, U, V;
	cv::gemm(points2, points, 1, NULL, 0, A, CV_GEMM_A_T);
	cv::SVD::compute(A, W, U, V);


	plane = cv::Mat::zeros(cols + 1, 1, CV_32FC1);
	for (int c = 0; c < cols; c++) {
		plane.at<float>(c, 0) = V.at<float>(cols - 1, c);
		plane.at<float>(cols, 0) += plane.at<float>(c, 0)*centroid.at<float>(0, c);
	}

}

int CCalcuPlaneness::Check(const cv::Mat& SrcImg, cv::Mat& DstImg, Json::Value &checkresult)
{
	/*kxCImageBuf threshimg;
	m_hFun.KxThreshImage(SrcImg, threshimg, 100, 255);
	m_hBlobFun.ToBlobByCV(threshimg);
	if (m_hBlobFun.GetBlobCount() > 0)
	{
		CKxBlobAnalyse::SingleBlobInfo blobinfo;
		blobinfo = m_hBlobFun.GetSortSingleBlob(0);
		Json::Value single;
		single["Dots"] = blobinfo.m_nDots;
		single["Energy"] = 0;
		single["pos"].append(blobinfo.m_rc.left);
		single["pos"].append(blobinfo.m_rc.top);
		single["pos"].append(blobinfo.m_rc.Width());
		single["pos"].append(blobinfo.m_rc.Height());
		checkresult["defect feature"].append(single);
		checkresult["defect num"] = 1;
	}
	DstImg = threshimg;*/

	return 1;
}


