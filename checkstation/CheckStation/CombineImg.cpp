#include "stdafx.h"
#include "CombineImg.h"


void CCombineImg::Init(int nW, int nH, int ncol, int* nnums, kxRect<int>* modelroi)
{
	m_nScanTimes = ncol;

	for (int i = 0; i < ncol; i++)
	{
		m_nEveryColImgnum[i] = nnums[i];
		
		m_ImgBigListA[i].Init(nW, nH*nnums[i], 3);

		m_ImgBigListB[i].Init(nW, nH*nnums[i], 3);

		m_nSingleH = nH;

		m_nSingleW = nW;

	}

	for (int i = 0; i < ncol * 2; i++)
	{
		m_rectmodel[i] = modelroi[i];
	}


	Clear();

	m_bisinit = true;

}


int CCombineImg::appendImg(kxCImageBuf& srcimg, int nImgIndex)
{
	//当匹配之后发现匹配错误返回0，否则返回1

	// 1. 确认是哪一列    nImgIndex是从0开始数的

	int ncolindex = 0;

	for (int i = 0; i < _MAX_SCAN_NUM; i++)
	{
		if (nImgIndex - m_nEveryColImgnum[i] * 2 < 0)
		{
			ncolindex = i;

			break;
		}

		nImgIndex = nImgIndex - m_nEveryColImgnum[i] * 2; 
	}


	// 2. 确认图像是正走那列，还是逆走那列

	int ncurimgindex = nImgIndex;

	if (nImgIndex < m_nEveryColImgnum[ncolindex])// 正走
	{
		int ntop = nImgIndex * m_nSingleH;

		IppiSize imgsize = {m_nSingleW, m_nSingleH};

		//std::cout << nImgIndex << "  " << nImgIndex * m_nSingleH << std::endl;


		ippiCopy_8u_C3R(srcimg.buf, srcimg.nPitch, m_ImgBigListA[ncolindex].buf + nImgIndex * m_nSingleH * m_ImgBigListA[ncolindex].nPitch, m_ImgBigListA[ncolindex].nPitch, imgsize);
		
	}
	else
	{
		nImgIndex -= m_nEveryColImgnum[ncolindex];

		int ntop = nImgIndex * m_nSingleH;

		IppiSize imgsize = { m_nSingleW, m_nSingleH };

		ippiCopy_8u_C3R(srcimg.buf, srcimg.nPitch, m_ImgBigListB[ncolindex].buf + nImgIndex * m_nSingleH * m_ImgBigListB[ncolindex].nPitch, m_ImgBigListB[ncolindex].nPitch, imgsize);

	}




	//tick_count tbb_start, tbb_end;

	// 3. 当列满之后，对图像B进行翻转，并将某个标志位置为True
	if (ncurimgindex + 1 == m_nEveryColImgnum[ncolindex] * 2)
	{
		cv::Mat src = cv::Mat(m_ImgBigListB[ncolindex].nHeight, m_ImgBigListB[ncolindex].nWidth, CV_8UC(3), m_ImgBigListB[ncolindex].buf);

		cv::Mat dst;

		//tbb_start = tick_count::now();

		cv::flip(src, dst, 0);

		//tbb_end = tick_count::now();

		//printf_s("flip cost : %f ms\n", (tbb_end - tbb_start).seconds() * 1000);

		m_ImgBigListB[ncolindex].SetImageBuf(dst.data, dst.cols, dst.rows, dst.step, dst.channels(), true);

		m_nCurScanTimes += 1;

		// 4. 对列满的图像进行模板匹配，然后对齐
		//tbb_start = tick_count::now();

		int ncaptureoffset1, ncaptureoffset2;
		
		bool matchstatus1 = MatchTemplateAndTransform(ncolindex, m_rectmodel[ncolindex * 2], ncaptureoffset1);

		bool matchstatus2 = MatchTemplateAndTransform(ncolindex, m_rectmodel[ncolindex * 2 + 1], ncaptureoffset2);

		if (matchstatus1 && matchstatus2)
		{
			std::cout << "匹配结果: " << ncaptureoffset1 << "    " << ncaptureoffset2 << std::endl;
			if (abs(ncaptureoffset1 - ncaptureoffset2) < 30)
			{

				kxCImageBuf bigimg;

				bigimg.Init(m_ImgBigListB[ncolindex].nWidth, m_ImgBigListB[ncolindex].nHeight, 3);

				ippsSet_8u(0, bigimg.buf, bigimg.nPitch*bigimg.nHeight);

				IppiSize copybigsize = { m_ImgBigListB[ncolindex].nWidth, m_ImgBigListB[ncolindex].nHeight - ncaptureoffset1 };

				IppStatus status = ippiCopy_8u_C3R(m_ImgBigListB[ncolindex].buf, m_ImgBigListB[ncolindex].nPitch,

					bigimg.buf + ncaptureoffset1 * bigimg.nPitch, bigimg.nPitch, copybigsize);

				IppiSize bigsize = { m_ImgBigListB[ncolindex].nWidth, m_ImgBigListB[ncolindex].nHeight };

				status = ippiCopy_8u_C3R(bigimg.buf, bigimg.nPitch, m_ImgBigListB[ncolindex].buf, m_ImgBigListB[ncolindex].nPitch, bigsize);

				m_bstatus[ncolindex] = true;

			}
			else
			{
				// kxprintf("上下匹配无法对应，可能出现匹配错误或图像丢图");
				m_bstatus[ncolindex] = false;// 没匹配到是要报错的
				return 0;
			}

		}
		else
		{
			// kxprintf("定位核无法匹配，查看原图确认采集情况");
			m_bstatus[ncolindex] = false;// 没匹配到是要报错的
			return 0;

		}


		//tbb_end = tick_count::now();

		//printf_s("matchimg cost : %f ms\n", (tbb_end - tbb_start).seconds() * 1000);
	}
	

	return 1;


}


bool CCombineImg::IsColFull(int &nIndex)
{
	return m_bstatus[nIndex];
}


void CCombineImg::GetImg(kxCImageBuf& dstimgA, kxCImageBuf& dstimgB, int nIndex)
{
	dstimgA.SetImageBuf(m_ImgBigListA[nIndex]);

	dstimgB.SetImageBuf(m_ImgBigListB[nIndex]);

}


void CCombineImg::ResetCol(int &nIndex)
{
	m_bstatus[nIndex] = false;
}


bool CCombineImg::MatchTemplateAndTransform(int ncol, kxRect<int> matchrect, int &ncaptureoffset)
{
	kxCImageBuf templateimg;

	templateimg.Init(matchrect.Width(), matchrect.Height(), 3);

	IppiSize copysize = { matchrect.Width(), matchrect.Height() };

	ippiCopy_8u_C3R(m_ImgBigListA[ncol].buf + matchrect.top * m_ImgBigListA[ncol].nPitch + matchrect.left * m_ImgBigListA[ncol].nChannel, m_ImgBigListA[ncol].nPitch, templateimg.buf, templateimg.nPitch, copysize);

	const int noffset = 1300;

	int ntop = gMax(matchrect.top - noffset, 0);

	int nH = matchrect.bottom - ntop + 1;
	//int nH = noffset + matchrect.Height();

	kxCImageBuf matchimg;

	const int horizonoffset = 5;

	int nleft = max(0, matchrect.left - horizonoffset);

	int nright = min(m_ImgBigListB[ncol].nWidth - 1, matchrect.right + horizonoffset);

	int nw = nright - nleft + 1;

	matchimg.Init(nw, nH, 3);

	IppiSize bigsize = { matchimg.nWidth, matchimg.nHeight};

	ippiCopy_8u_C3R(m_ImgBigListB[ncol].buf + ntop * m_ImgBigListB[ncol].nPitch + nleft * m_ImgBigListB[ncol].nChannel, m_ImgBigListB[ncol].nPitch, matchimg.buf, matchimg.nPitch, bigsize);

	kxPoint<float> matchpos;

	float frate = m_hAlg.Matchtemplate(matchpos, matchimg, templateimg);

	
	//----------------------------------//
	//static int nsavenum = 0;

	//char savepath[32];

	//sprintf_s(savepath, "d:\\matchimg%d.bmp", nsavenum);

	//CKxBaseFunction fun;

	//fun.SaveBMPImage_h(savepath, matchimg);

	//sprintf_s(savepath, "d:\\templateimg%d.bmp", nsavenum);

	//fun.SaveBMPImage_h(savepath, templateimg);

	//nsavenum++;

	//----------------------------------//


	int ndata = matchrect.top - (matchpos.y + ntop);// B的位置比A往上了这么多

	if (frate > 0.4 && ndata >= 0)// ncaptureoffset 在理想的位置只应该大于0
	{
		ncaptureoffset = ndata;

		return true;


	}
	else
	{
		std::cout << "匹配结果为负，" << ncol << std::endl;


		return false;
	}

}


bool CCombineImg::IsAllFull()
{
	if (m_nCurScanTimes == m_nScanTimes)
	{
		return true;
	}
	else
	{
		return false;
	}
}


void CCombineImg::Clear()
{
	m_nCurScanTimes = 0;

	if (m_bisinit)
	{
		for (int i = 0; i < m_nScanTimes; i++)
		{
			m_bstatus[i] = false;

			ippsSet_8u(0, m_ImgBigListA[i].buf, m_ImgBigListA[i].nPitch * m_ImgBigListA[i].nHeight);

			ippsSet_8u(0, m_ImgBigListB[i].buf, m_ImgBigListB[i].nPitch * m_ImgBigListB[i].nHeight);

		}

	}
	else
	{
		for (int i = 0; i < _MAX_SCAN_NUM; i++)
		{
			m_bstatus[i] = false;
		}
	}

}

