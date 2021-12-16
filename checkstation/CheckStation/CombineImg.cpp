#include "stdafx.h"
#include "CombineImg.h"


void CCombineImg::Init(int nW, int nH, int ncol, int* nnums, kxRect<int>* modelroi)
{
	for (int i = 0; i < ncol; i++)
	{
		m_nEveryColImgnum[i] = nnums[i];
		
		m_rectmodel[i] = modelroi[i];

		m_ImgBigListA[i].Init(nW, nH*nnums[i], 3);

		m_ImgBigListB[i].Init(nW, nH*nnums[i], 3);

		m_nSingleH = nH;

		m_nSingleW = nW;

	}

	for (int i = 0; i < _MAX_SCAN_NUM; i++)
	{
		m_bstatus[i] = false;
	}
}


void CCombineImg::appendImg(kxCImageBuf& srcimg, int nImgIndex)
{

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

		ippiCopy_8u_C3R(srcimg.buf, srcimg.nPitch, m_ImgBigListA[ncolindex].buf + nImgIndex * m_nSingleH * m_ImgBigListA[ncolindex].nPitch, m_ImgBigListA[ncolindex].nPitch, imgsize);
		
	}
	else
	{
		nImgIndex -= m_nEveryColImgnum[ncolindex];

		int ntop = nImgIndex * m_nSingleH;

		IppiSize imgsize = { m_nSingleW, m_nSingleH };

		ippiCopy_8u_C3R(srcimg.buf, srcimg.nPitch, m_ImgBigListB[ncolindex].buf + nImgIndex * m_nSingleH * m_ImgBigListB[ncolindex].nPitch, m_ImgBigListB[ncolindex].nPitch, imgsize);

	}


	// 3. 当列满之后，对图像B进行翻转，并将某个标志位置为True
	if (ncurimgindex + 1 == m_nEveryColImgnum[ncolindex] * 2)
	{
		cv::Mat src = cv::Mat(m_ImgBigListB[ncolindex].nHeight, m_ImgBigListB[ncolindex].nWidth, CV_8UC(3), m_ImgBigListB[ncolindex].buf);

		cv::Mat dst;

		cv::flip(src, dst, 0);

		m_ImgBigListB[ncolindex].SetImageBuf(dst.data, dst.cols, dst.rows, dst.step, dst.channels(), false);

		m_bstatus[ncolindex] = true;


		// 4. 对列满的图像进行模板匹配，然后对齐

		//TODO 12.13调试到这个位置，进行测试试试

		MatchTemplateAndTransform(ncolindex);


	}
	

	


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


void CCombineImg::MatchTemplateAndTransform(int ncol)
{
	kxCImageBuf templateimg;

	templateimg.Init(m_rectmodel[ncol].Width(), m_rectmodel[ncol].Height(), 3);

	IppiSize copysize = { m_rectmodel[ncol].Width(), m_rectmodel[ncol].Height() };

	ippiCopy_8u_C3R(m_ImgBigListA[ncol].buf + m_rectmodel[ncol].top * m_ImgBigListA[ncol].nPitch + m_rectmodel[ncol].left * m_ImgBigListA[ncol].nChannel, m_ImgBigListA[ncol].nPitch, templateimg.buf, templateimg.nPitch, copysize);

	const int noffset = 1300;

	int ntop = gMax(m_rectmodel[ncol].top - noffset, 0);

	//int nH = m_rectmodel[ncol].bottom - ntop + 1;

	int nH = m_rectmodel[ncol].Height() * 2;

	kxCImageBuf matchimg;

	matchimg.Init(m_rectmodel[ncol].Width(), nH, 3);

	IppiSize bigsize = { matchimg.nWidth, matchimg.nHeight};

	ippiCopy_8u_C3R(m_ImgBigListB[ncol].buf + ntop * m_ImgBigListB[ncol].nPitch + m_rectmodel[ncol].left * m_ImgBigListB[ncol].nChannel, m_ImgBigListB[ncol].nPitch, matchimg.buf, matchimg.nPitch, bigsize);

	kxPoint<float> matchpos;

	float frate = m_hAlg.Matchtemplate(matchpos, matchimg, templateimg);

	if (frate > 0.6)
	{
		//int ncaptureoffset = nH - matchpos.y - templateimg.nHeight;
		int ncaptureoffset = m_rectmodel[ncol].top - (matchpos.y + ntop);// B的位置比A往上了这么多

		kxCImageBuf bigimg;

		bigimg.Init(m_ImgBigListB[ncol].nWidth, m_ImgBigListB[ncol].nHeight, 3);

		ippsSet_8u(0, bigimg.buf, bigimg.nPitch*bigimg.nHeight);

		IppiSize copybigsize = { m_ImgBigListB[ncol].nWidth, m_ImgBigListB[ncol].nHeight - ncaptureoffset };

		ippiCopy_8u_C3R(m_ImgBigListB[ncol].buf , m_ImgBigListB[ncol].nPitch,
			
			bigimg.buf + ncaptureoffset * bigimg.nPitch, bigimg.nPitch, copybigsize);

		m_ImgBigListB[ncol].SetImageBuf(bigimg, true);

	}

}