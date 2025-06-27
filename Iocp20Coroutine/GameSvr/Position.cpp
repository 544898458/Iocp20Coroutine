#include "pch.h"
#include "Position.h"

/// <summary>
/// 平方就是2次方
/// </summary>
const float fExponent = 2.0f;

const Position g_posZero = { 0, 0, 0 };


Position Position::operator*(float f)const
{
	return { x * f, 0, z * f };
}

bool Position::DistanceLessEqual(const Position& refPos, float fDistance) const
{
	return this->DistancePow2(refPos) <= std::pow(fDistance, fExponent);
}

float Position::DistancePow2(const Position& refPos) const
{
	return std::pow(x - refPos.x, fExponent) + std::pow(z - refPos.z, fExponent);
}

float Position::Distance(const Position& refPos) const
{
	return std::sqrtf(DistancePow2(refPos));
}

float Position::LengthPow2()const
{
	return std::pow(x, fExponent) + std::pow(z, fExponent);
}

float Position::Length()const
{
	return std::sqrtf(LengthPow2());
}
/// <summary>
/// 向量归一化(Vector Normalization)
/// </summary>

Position Position::归一化()const
{
	const float f范数 = std::sqrtf(std::pow(x, fExponent) + std::pow(z, fExponent));
	if (0 == f范数)
	{
		LOG(ERROR) << "范数是0，无法归一化," << *this;
		return { x , 0, z };
	}

	return { x / f范数, 0, z / f范数 };
}
