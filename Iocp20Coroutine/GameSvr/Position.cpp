#include "pch.h"
#include "Position.h"

/// <summary>
/// ƽ������2�η�
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
/// ������һ��(Vector Normalization)
/// </summary>

Position Position::��һ��()const
{
	const float f���� = std::sqrtf(std::pow(x, fExponent) + std::pow(z, fExponent));
	if (0 == f����)
	{
		LOG(ERROR) << "������0���޷���һ��," << *this;
		return { x , 0, z };
	}

	return { x / f����, 0, z / f���� };
}
