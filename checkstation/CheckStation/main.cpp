// main.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "TestAsioTcpClient.h"
#include "CkxEnvironment.h"
#include "GrabPack.h"
#include "global.h"
#include "minidmp.h"
#include "kxParameter.h"
#include "SaveQue.h"
#include "zcudpclient.h"
#include "opencv2/opencv.hpp"

#include <boost/python.hpp>

//#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "json.h"
#include "Logger.h"

using namespace boost::python;
using namespace tbb;
using namespace cv;

void MatToKxImageBuf(cv::Mat& SrcImg, kxCImageBuf&DstImg)
{
	DstImg.SetImageBuf(SrcImg.data, SrcImg.cols, SrcImg.rows, SrcImg.step, SrcImg.channels(), false);
}


void test_python()
{
	try
	{
		Py_Initialize();
		if (!Py_IsInitialized())
			return ;
		int a = 10;
		object main_ns = import("__main__").attr("__dict__");
		object main_module = import("__main__");
		main_module.attr("x") = "0";
		main_module.attr("expression") = "(0 > 1) |(2 > 1)";
		char* funcdef = "try: \n"
			"\tTmp =  eval(str(expression))\n"
			"except: \n"
			"\tTmp =  -111111\n";
		exec(funcdef, main_ns, main_ns);
		object res = main_ns["Tmp"];
		int m_fResult = extract<int>(res);
		std::cout << m_fResult << std::endl;
	}
	catch (boost::python::error_already_set&)
	{
		//PyErr_Print();
		//return false;
	}
}
std::vector<Vec3b> colors;
void colorizeSegmentation(const Mat &score, Mat &segm)
{
	const int rows = score.size[2];
	const int cols = score.size[3];
	const int chns = score.size[1];
	if (colors.empty())
	{
		// Generate colors.
		colors.push_back(Vec3b());
		for (int i = 1; i < chns; ++i)
		{
			Vec3b color;
			for (int j = 0; j < 3; ++j)
				color[j] = (colors[i - 1][j] + rand() % 256) / 2;
			colors.push_back(color);
		}
	}
	else if (chns != (int)colors.size())
	{
		CV_Error(Error::StsError, format("Number of output classes does not match "
			"number of colors (%d != %zu)", chns, colors.size()));
	}
	Mat maxCl = Mat::zeros(rows, cols, CV_8UC1);
	Mat maxVal(rows, cols, CV_32FC1, score.data);
	for (int ch = 1; ch < chns; ch++)
	{
		for (int row = 0; row < rows; row++)
		{
			const float *ptrScore = score.ptr<float>(0, ch, row);
			uint8_t *ptrMaxCl = maxCl.ptr<uint8_t>(row);
			float *ptrMaxVal = maxVal.ptr<float>(row);
			for (int col = 0; col < cols; col++)
			{
				if (ptrScore[col] > ptrMaxVal[col])
				{
					ptrMaxVal[col] = ptrScore[col];
					ptrMaxCl[col] = (uchar)ch;
				}
			}
		}
	}
	segm.create(rows, cols, CV_8UC3);
	for (int row = 0; row < rows; row++)
	{
		const uchar *ptrMaxCl = maxCl.ptr<uchar>(row);
		Vec3b *ptrSegm = segm.ptr<Vec3b>(row);
		for (int col = 0; col < cols; col++)
		{
			ptrSegm[col] = colors[ptrMaxCl[col]];
		}
	}
}

//
//void test_opencv()
//{
//	using namespace cv;
//	using namespace dnn;
//	using namespace std;
//
//	//String model_pb = "D:\\github_save\\unet_class\\log\\123\\model\\model.pb";
//	//String test_image = "D:\\github_save\\unet_class\\dataset\\0001\\img.bmp";
//	String model_pb = "model.pb";
//	String test_image = "img.bmp";
//
//	// Load the network.
//	cv::dnn::Net net = readNet(model_pb);
//	//net.setPreferableBackend(DNN_BACKEND_OPENCV);
//	//net.setPreferableTarget(DNN_TARGET_CPU);
//
//	auto names = net.getLayerNames();
//	net.setPreferableBackend(DNN_BACKEND_CUDA);
//	net.setPreferableTarget(DNN_TARGET_CUDA);
//	// Get data.
//	Mat frame, blob;
//	frame = imread(test_image, 0);
//	blobFromImage(frame, blob, 1.0, Size(frame.cols, frame.rows), Scalar(), true, false);
//
//	// Forward.
//	net.setInput(blob);
//
//	Mat score = net.forward();
//	Mat segm;
//	colorizeSegmentation(score, segm);
//
//
//	/*std::ofstream fout("out.txt");
//	for (int i = 0; i < outs[0].total(); i++) fout << outs[0].at<float>(i) << endl;
//	fout.close();*/
//
//	vector<double> layersTimes;
//	double freq = getTickFrequency() / 1000;
//	double t = net.getPerfProfile(layersTimes) / freq;
//	string info = format("Inference time for a frame : %0.0f ms", t);
//
//	cout << info << endl;
//	cv::imshow("123", segm);
//	cv::waitKey(0);
//}
HWND hwnd = GetForegroundWindow();//使hwnd代表最前端的窗口 

void testjson()
{
	Json::Value single;

	char defectid[32];
	sprintf_s(defectid, "%d_%d", 1, 2);
	single["defectid"] = defectid;

	std::string a = single["defectid"].asString();

	char path[32];

	sprintf_s(path, "%s.bmp", a.c_str());

	std::cout << a.c_str();
}

int _tmain(int argc, _TCHAR* argv[])
{
	//test_python();
	//test_opencv();
	//testjson();
	//设置异常处理函数
	//::SetUnhandledExceptionFilter(GPTUnhandledExceptionFilter);
	//ShowWindow(hwnd, SW_MINIMIZE);//设置窗口最小化


	Initlog();
	g_Environment.ReadSystemParam();
	g_bdotcheckstatus = false;

	Graber::g_GetGrabPack().InitCamera();
	bool bInitIsOk = Graber::g_GetGrabPack().Init();
	int nStatus = bInitIsOk ? 1 : 0;
	if (bInitIsOk)
	{
		Graber::g_GetCamera()->ReverseScanDirection(0);// 代表相机正着走触发
		Graber::g_GetCamera()->OpenInternalTrigger(1);//外触发


		char szInfo[1024];
		sprintf_s(szInfo, 1024, "相机%d初始化成功", Config::g_GetParameter().m_nNetStationId);
		kxPrintf(KX_INFO, szInfo);
	}
	else
	{
		char szInfo[1024];
		sprintf_s(szInfo, 1024, "相机%d初始化失败", Config::g_GetParameter().m_nNetStationId);
		kxPrintf(KX_Err, szInfo);
	}


	std::ostringstream os;
	os.write(reinterpret_cast<const char *>(&nStatus), sizeof(int));
	std::string str = os.str();
	if (Net::IsExistNetObj())
	{
		Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_CAMERA_IS_READY), int(str.size()), str.c_str());
	}

	while(getchar() != 'q');
	Graber::g_GetCamera()->Close();
	spdlog::drop_all();

	return 0;
}
