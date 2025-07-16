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
	/// ������һ��(Vector Normalization)
	/// </summary>
	Position ��һ��()const;
};
extern const Position g_posZero;//(0,0,0)
struct Rect
{
	Position pos����;
	Position pos����;
	float ��()const
	{
		return pos����.x - pos����.x;
	}
	int32_t ��Int32()const
	{
		return (int32_t)��();
	}
	float ��()const
	{
		return pos����.z - pos����.z;
	}
	int32_t ��Int32()const
	{
		return (int32_t)��();
	}
	bool �����˵�(const Position& pos)const
	{
		return
			pos����.x < pos.x && pos.x < pos����.x &&
			pos����.z < pos.z && pos.z < pos����.z;
	}
};

template <class _Traits>
std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const Position& _ref)
{
	return _Ostr << "(" << _ref.x << "," << _ref.y << "," << _ref.z << ")";
}

Position �ڷ��������һ����(const uint16_t u16�뾶);