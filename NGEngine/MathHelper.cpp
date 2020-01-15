#include "stdafx.h"
#include "MathHelper.h"

using namespace DirectX;
const float MathHelper::Infinity = FLT_MAX;
const float MathHelper::Pi = 3.1415926535f;
float MathHelper::AngleFromXY(float x, float y)
{
	float theta = 0.0f;
	if(x>=0.0f)
	{
		theta = atanf(y / x);
		if (theta < 0.0f)
			theta += 2.0f * Pi;
	}else
	{
		theta = atanf(y / x) + Pi;
	}
	return theta;
}

DirectX::XMVECTOR MathHelper::RandUnitVec3()
{
	XMVECTOR One = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR Zero = XMVectorZero();
	while(true)
	{
		//  先在一个立方体里生成随机的点。
		XMVECTOR v = XMVectorSet(MathHelper::RandF(-1.0f, 1.0f),RandF(-1.0f,1.0f),  RandF(-1.0f, 1.0f), 0.0f);

		// 忽略那些在单位圆外的点，目的是为了得到单位圆内更加均匀的分布。否则会有更多的点分布在立方体的八个角落的方位上。
		if (XMVector3Greater(XMVector3LengthSq(v), One))
			continue;
		return XMVector3Normalize(v);		
	}
}
DirectX::XMVECTOR MathHelper::RandHemisphereUnitVec3(DirectX::XMVECTOR n)
{
	XMVECTOR One = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR Zero = XMVectorZero();
	while(true)
	{
		XMVECTOR v = XMVectorSet(RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), 0);
		if(XMVector3Greater(XMVector3LengthSq(v),One))
			continue;
		if (XMVector3Less(XMVector3Dot(n, v), Zero))
			continue;
		return XMVector3Normalize(v);		
	}
}



