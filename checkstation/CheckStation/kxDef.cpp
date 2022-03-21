#include "stdafx.h"
#include "kxDef.h"
#include "tbb/tbb.h"
#include "tbb/scalable_allocator.h"
using namespace tbb;

kxCImageBuf::kxCImageBuf()
{
	buf = NULL;
	nWidth = 0;
	nHeight = 0;
	nPitch = 0;
	nChannel = 0;
	bAuto = false;
}
kxCImageBuf::kxCImageBuf( int width, int height, int nChannel )
{
	buf = NULL;
    Init( width, height, nChannel );
}
kxCImageBuf::~kxCImageBuf()
{
	Release();
}
kxCImageBuf::kxCImageBuf( const kxCImageBuf& Obj )
{
	Clone( Obj );
}
kxCImageBuf& kxCImageBuf::operator = ( const kxCImageBuf& Obj )
{
	if( this != &Obj )
	{
		Release();
		Clone( Obj );
	}
	return *this;
}
void kxCImageBuf::Release()
{
	if( bAuto && buf )
	{
		ippDelete( buf );
		buf = NULL;
	}
	bAuto = false;
}

void kxCImageBuf::SetImageBuf( const unsigned char* pBuf, int width, int height, int pitch, int nChan, bool bCopy)
{
    KxCallStatus hCall ;
    SetImageBuf(pBuf, width, height, pitch, nChan, bCopy, hCall);
}
void kxCImageBuf::SetImageBuf( const unsigned char* pBuf, int width, int height, int pitch, int nChan, bool bCopy, KxCallStatus& hCall )
{
  	Release();
	nWidth = width;
	nHeight = height;
	bAuto = bCopy;
	nChannel = nChan;
	IppStatus status;
	if( bAuto )
	{
		nPitch = nWidth * nChannel;
		//buf = ippNew( nPitch*nHeight);
		long long bufsize = (long long)nPitch * nHeight;
		buf = ippNew(bufsize);
		IppiSize  roiSize;
		roiSize.width = width;
		roiSize.height = height;
        if( nChannel == 1 )
		{
		    status = ippiCopy_8u_C1R( pBuf, pitch, buf, nPitch, roiSize );
			if (check_sts(status, "CopyC1Err", hCall))
			{
				return;
			}
		}
		else if (nChannel == 3)
		{

			status = ippiCopy_8u_C3R(pBuf, pitch, buf, nPitch, roiSize);


			if (check_sts(status, "CopyC3Err", hCall))
			{
				return;
			}
		}
		else
		{
			for( int y = 0; y < height; y++ )
			{
				memcpy( buf + y*nPitch, pBuf + y*pitch, nWidth*nChannel );
			}
		}
	}
	else
	{
		buf = const_cast<unsigned char*>(pBuf);
		nPitch = pitch;
	}
}

void kxCImageBuf::SetImageBuf(const kxCImageBuf& srcimg, bool bCopy)
{
	SetImageBuf(srcimg.buf, srcimg.nWidth, srcimg.nHeight, srcimg.nPitch, srcimg.nChannel, bCopy);
}



unsigned char* kxCImageBuf::GetImageBuf( int& width, int& height, int& pitch ,int& channel) const
{
	width = nWidth;
	height = nHeight;
	pitch = nPitch;
	channel = nChannel;
	return buf;
}
void kxCImageBuf::Init( int width, int height, int nChan )
{
	if( buf && nWidth == width && nHeight == height && nChannel== nChan)
		return;
	Release();

	nWidth = width;
	nHeight = height;
	nChannel = nChan;
	nPitch = nWidth * nChannel;
	bAuto = true;

	long long bufsize = (long long)nPitch * nHeight;
	buf = ippNew(bufsize);
	/*buf = ippNew( nPitch*nHeight);*/
}
BOOL kxCImageBuf::Read( FILE* fp )
{
	Release();
	if( fread( &nWidth, sizeof( int ), 1, fp ) != 1 )
		return FALSE;
	if( fread( &nHeight, sizeof( int ), 1, fp ) != 1 )
		return FALSE;
	if( fread( &nPitch, sizeof( int ), 1, fp ) != 1 )
		return FALSE;
	if( fread( &nChannel, sizeof( int ), 1, fp ) != 1 )
		return FALSE;
	int  nToken;
	if( fread( &nToken, sizeof( int ), 1, fp ) != 1 )
		return FALSE;
	if (nWidth<0 || nWidth>10000000 || nHeight<0 || nHeight>10000000 ||  nPitch<0 || nPitch>10000000)
	{
		return FALSE;
	}
    if( nToken )
	{
		//buf = ippNew( nPitch*nHeight);
  //      bAuto = true;
		//if( fread( buf, sizeof( unsigned char ), nPitch*nHeight, fp ) != nPitch*nHeight )
  //          return FALSE;
		long long bufsize = (long long)nPitch * nHeight;
		buf = ippNew(bufsize);
		bAuto = true;
		if (fread(buf, sizeof(unsigned char), bufsize, fp) != bufsize)
			return FALSE;
	}
	return TRUE;
}
BOOL kxCImageBuf::Write( FILE* fp )
{
	if( fwrite( &nWidth, sizeof( int ), 1, fp ) != 1 )
		return FALSE;
	if( fwrite( &nHeight, sizeof( int ), 1, fp ) != 1 )
		return FALSE;
	if( fwrite( &nPitch, sizeof( int ), 1, fp ) != 1 )
		return FALSE;
	if( fwrite( &nChannel, sizeof( int ), 1, fp ) != 1 )
		return FALSE;
	int  nToken = ( buf != NULL ? 1:0 );
	if( fwrite( &nToken, sizeof( int ), 1, fp ) != 1 )
		return FALSE;
    if( nToken )
	{
		//if( fwrite( buf, sizeof( unsigned char ), nPitch*nHeight, fp ) != nPitch*nHeight )
		//	return FALSE;
		long long bufsize = (long long)nPitch * nHeight;
		if( fwrite( buf, sizeof( unsigned char ), bufsize, fp ) != bufsize)
			return FALSE;
	}
    return TRUE;
}
void kxCImageBuf::Clone( const kxCImageBuf& Obj )
{
	nWidth = Obj.nWidth;
	nHeight = Obj.nHeight;
	nPitch = Obj.nPitch;
	nChannel = Obj.nChannel;
	bAuto = true; //Obj.bAuto;
	if( Obj.buf )
	{
		//buf = ippNew( nPitch*nHeight );
		//ippsCopy_8u( Obj.buf, buf, nPitch*nHeight );
		long long bufsize = (long long)nPitch * nHeight;
		buf = ippNew(bufsize);
		ippsCopy_8u( Obj.buf, buf, bufsize);
	}
	else
		buf = Obj.buf;
}



kxImg16u::kxImg16u()
{
	buf = NULL;
	nWidth = 0;
	nHeight = 0;
	nPitch = 0;
	bAuto = false;
}
kxImg16u::kxImg16u( int width, int height)
{
	buf = NULL;
	Init( width, height);
}
kxImg16u::~kxImg16u()
{
	Release();
}
void kxImg16u::Release()
{
	if( bAuto && buf )
	{
		delete []buf;
		buf = NULL;
	}
	bAuto = false;
}

void kxImg16u::Init( int width, int height)
{
	if( buf && nWidth == width && nHeight == height)
		return;
	Release();

	nWidth = width;
	nHeight = height;
	nPitch = nWidth * sizeof(Ipp16u);
	bAuto = true;

	buf = new Ipp16u[nWidth*nHeight];
}



kxImg32f::kxImg32f()
{
	buf = NULL;
	nWidth = 0;
	nHeight = 0;
	nPitch = 0;
	bAuto = false;
}
kxImg32f::kxImg32f( int width, int height)
{
	buf = NULL;
	Init( width, height);
}
kxImg32f::~kxImg32f()
{
	Release();
}
void kxImg32f::Release()
{
	if( bAuto && buf )
	{
		delete []buf;
		buf = NULL;
	}
	bAuto = false;
}

void kxImg32f::Init( int width, int height)
{
	if( buf && nWidth == width && nHeight == height)
		return;
	Release();

	nWidth = width;
	nHeight = height;
	nPitch = nWidth * sizeof(Ipp32f);
	bAuto = true;

	buf = new Ipp32f[nWidth*nHeight];
}





/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//class kxImageExtract      //对图像 抽点变换
std::string kxCImageExtract::GetExtractTypeMsg(int nType)
{
	std::string   str;
	switch( nType )
	{
	case _Type_NULL:
		str = "不做变换";
		break;
	case _Type_2:
		str = "2点取1点";
		break;
	case _Type_4:
		str = "4点取1点";
		break;
	case _Type_8:
		str = "8点取1点";
		break;
	case _Type_16:
		str = "16点取1点";
		break;
	}
	return str;
}
void kxCImageExtract::Release()
{
	if( m_pBuf && m_bAuto )
		delete []m_pBuf;
	m_bAuto = FALSE;
}
void kxCImageExtract::GetExtractMsg( int nType, int nSrcWidth, int nSrcHeight, int nSrcPitch, int& nWidth, int& nHeight, int& nPitch )
{//根据抽点类型， 返回 抽点后的图像 大小信息
	assert( nType == _Type_NULL || nType == _Type_2 || nType == _Type_4 || nType == _Type_8 || nType == _Type_16);
	nWidth = (nSrcWidth>>nType);
	nHeight = (nSrcHeight>>nType);
	if( nType != _Type_NULL )
	{
		nPitch = nWidth;
	}
	else
	{
		nPitch = nSrcPitch;
	}
}

void kxCImageExtract::Extract( const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nType, unsigned char* pTMP )
{
	assert( nType == _Type_NULL || nType == _Type_2 || nType == _Type_4 || nType == _Type_8 || nType == _Type_16);

	GetExtractMsg( nType, nWidth, nHeight, nPitch, m_nWidth, m_nHeight, m_nPitch );
	Release();
	if( nType == _Type_NULL )
	{
		m_pBuf = const_cast<unsigned char*>(buf);
		m_bAuto = FALSE;
		return;
	}
	if( pTMP == NULL )
	{
		m_pBuf = new unsigned char[m_nPitch*m_nHeight];
		m_bAuto = TRUE;
	}
	else
	{
		m_pBuf = const_cast<unsigned char*>(pTMP);
		m_bAuto = FALSE;
	}
	for( int y = 0; y < m_nHeight; y++ )
	{
		int  y1 = (y <<nType);
		for( int x = 0; x < m_nWidth; x++ )
		{
			m_pBuf[y*m_nPitch+x] = buf[y1*nPitch+(x<<nType)];
		}
	}
}

//   class kxCDivRect
void kxCDivRect::Release()
{
	if( m_pRect )
	{
		for( int i = 0; i < m_Row; i++ )
		{
			delete []m_pRect[i];
		}
		delete []m_pRect;
		m_pRect = NULL;
	}
	m_Col = 0;
	m_Row = 0;
}

BOOL kxCDivRect::Check( int nWidth, int nHeight, int nRow, int nCol )
{
	Release();
	if( nRow < 0 || nRow > nHeight )
		return FALSE;
	if( nCol < 0 || nCol > nWidth )
		return FALSE;

	m_Row = nRow;
	m_Col = nCol;
	//----------------------------------------------------------------------------------------
	m_pRect = new (kxRect<int>*[m_Row]);
	for( int y = 0; y < m_Row; y++ )
	{
		m_pRect[y] = new kxRect<int>[m_Col];
	}
	//----------------------------------------------------------------------------------------
	int  nColDiff = nWidth/m_Col;
	int  nRemCol = nWidth % m_Col;

	int  nRowDiff = nHeight/m_Row;
	int  nRemRow = nHeight % m_Row;

	m_pRect[0][0].left = 0;
	m_pRect[0][0].top = 0;
	m_pRect[0][0].right = m_pRect[0][0].left + nColDiff - 1;
	m_pRect[0][0].bottom = m_pRect[0][0].top + nRowDiff - 1;

	if( nRemCol )
	{
		m_pRect[0][0].right++;
		nRemCol--;
	}
	if( nRemRow )
	{
		m_pRect[0][0].bottom++;
		nRemRow--;
	}

	for( int x = 1; x < m_Col; x++ )
	{
		m_pRect[0][x].top = m_pRect[0][x-1].top;
		m_pRect[0][x].left = m_pRect[0][x-1].right + 1;
		m_pRect[0][x].right = m_pRect[0][x].left + nColDiff -1;
		m_pRect[0][x].bottom = m_pRect[0][x-1].bottom;

		if( nRemCol )
		{
			m_pRect[0][x].right++;
			nRemCol--;
		}
	}
	//------------------------------------------------------------------------------------
	for( int y = 1; y < m_Row; y++ )
	{
		for( int x = 0; x < m_Col; x++ )
		{
			m_pRect[y][x].left = m_pRect[y-1][x].left;
			m_pRect[y][x].right = m_pRect[y-1][x].right;
			m_pRect[y][x].top = m_pRect[y-1][x].bottom + 1;
			m_pRect[y][x].bottom = m_pRect[y][x].top + nRowDiff-1;

			if( nRemRow )
			{
				m_pRect[y][x].bottom++;
				nRemRow--;
			}
		}
	}
	return TRUE;
}

BOOL kxCDivRect::Check1( int nWidth, int nHeight, int nRow, int nCol )
{
	//--将 nWidth X nHeight  分成 nGridX X nGirdY 个网格
	Release();
	if( nRow < 0 || nRow > nHeight )
		return FALSE;
	if( nCol < 0 || nCol > nWidth )
		return FALSE;

	m_Row = nRow;
	m_Col = nCol;
	//----------------------------------------------------------------------------------------
	m_pRect = new (kxRect<int>*[m_Row]);
	for( int y = 0; y < m_Row; y++ )
	{
		m_pRect[y] = new kxRect<int>[m_Col];
	}
	//----------------------------------------------------------------------------------------
	int  nColDiff = nWidth/m_Col;
	int  nRemCol = nWidth % m_Col;

	int  nRowDiff = nHeight/m_Row;
	int  nRemRow = nHeight % m_Row;

	m_pRect[0][0].left = 0;
	m_pRect[0][0].top = 0;
	m_pRect[0][0].right = m_pRect[0][0].left + nColDiff - 1;
	m_pRect[0][0].bottom = m_pRect[0][0].top + nRowDiff - 1;

	for( int x = 1; x < m_Col; x++ )
	{
		m_pRect[0][x].top = m_pRect[0][x-1].top;
		m_pRect[0][x].left = m_pRect[0][x-1].right + 1;
		m_pRect[0][x].right = m_pRect[0][x].left + nColDiff -1;
		m_pRect[0][x].bottom = m_pRect[0][x-1].bottom;
	}
	m_pRect[0][ m_Col-1 ].right += nRemCol;

	//------------------------------------------------------------------------------------
	for( int y = 1; y < m_Row; y++ )
	{
		for( int x = 0; x < m_Col; x++ )
		{
			m_pRect[y][x].left = m_pRect[y-1][x].left;
			m_pRect[y][x].right = m_pRect[y-1][x].right;
			m_pRect[y][x].top = m_pRect[y-1][x].bottom + 1;
			m_pRect[y][x].bottom = m_pRect[y][x].top + nRowDiff - 1;
		}
	}
	for( int x = 0; x < m_Col; x++ )
	{
		m_pRect[m_Row-1][x].bottom += nRemRow;
	}
	return TRUE;
}
BOOL kxCDivRect::Read( FILE* fp )
{
	Release();
	if( fread(&m_Row, sizeof(int), 1, fp ) != 1 )
		return FALSE;
	if( fread(&m_Col, sizeof(int), 1, fp ) != 1 )
		return FALSE;
	if( m_Row > 0 && m_Col > 0 )
	{
		m_pRect = new (kxRect<int>*[m_Row]);
		for( int y = 0; y < m_Row; y++ )
		{
			m_pRect[y] = new kxRect<int>[m_Col];
			if( fread( m_pRect[y], sizeof(kxRect<int>), m_Col, fp ) != m_Col )
			{
				Release();
				return FALSE;
			}
		}
	}
	return TRUE;
}
BOOL kxCDivRect::Write( FILE* fp )
{
	if( fwrite(&m_Row, sizeof(int), 1, fp ) != 1 )
		return FALSE;
	if( fwrite(&m_Col, sizeof(int), 1, fp ) != 1 )
		return FALSE;

	for( int y = 0; y < m_Row; y++ )
	{
		if( fwrite( m_pRect[y], sizeof(kxRect<int>), m_Col, fp ) != m_Col )
		{
			return FALSE;
		}
	}
	return TRUE;
}



/////=====
