#include "stdafx.h"
#include "D3DAppBase.h"

using namespace Microsoft::WRL;
using namespace std;
using namespace DirectX;

D3DAppBase::D3DAppBase(UINT width, UINT height, std::wstring name) :mWidth(width), mHeight(height), mTitle(name) 
{
	
	
}
D3DAppBase::~D3DAppBase()
{

}