#pragma once
#include <msgpack.hpp>

struct Position
{
	float x;
	float y;
	float z;
	MSGPACK_DEFINE(x, y, z);
	bool operator==(const Position& refRight)const
	{
		return x == refRight.x && z == refRight.z;
	}
	void operator+=(const Position& refRight)
	{
		x += refRight.x;
		z += refRight.z;
	}
	Position operator/(const float)const;
	Position operator-(const Position& refRight)const
	{
		Position pos(*this);
		pos.x -= refRight.x;
		pos.z -= refRight.z;
		return pos;
	}
	Position operator*(float f)const;
	Position operator+(const Position& refRight)const
	{
		Position pos(*this);
		pos.x += refRight.x;
		pos.z += refRight.z;
		return pos;
	}
	bool DistanceLessEqual(const Position& refPos, float fDistance)const;

	float DistancePow2(const Position& refPos)const;
	float Distance(const Position& refPos)const;

	float LengthPow2() const;

	float Length() const;

	/// <summary>
	/// 向量归一化(Vector Normalization)
	/// </summary>
	Position 归一化()const;
};
extern const Position g_posZero;//(0,0,0)
struct Rect
{
	Position pos左上;
	Position pos右下;
	float 宽()const
	{
		return pos右下.x - pos左上.x;
	}
	int32_t 宽Int32()const
	{
		return (int32_t)宽();
	}
	float 高()const
	{
		return pos右下.z - pos左上.z;
	}
	int32_t 高Int32()const
	{
		return (int32_t)高();
	}
	bool 包含此点(const Position& pos)const
	{
		return
			pos左上.x < pos.x && pos.x < pos右下.x &&
			pos左上.z < pos.z && pos.z < pos右下.z;
	}
};

template <class _Traits>
std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const Position& _ref)
{
	return _Ostr << "(" << _ref.x << "," << _ref.y << "," << _ref.z << ")";
}

Position 在方块里随机一个点(const uint16_t u16半径);