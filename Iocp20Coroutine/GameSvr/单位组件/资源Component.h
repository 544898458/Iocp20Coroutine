#pragma once
class Space;
enum ��λ����;
struct Position;
class ��ԴComponent
{
public:
	static void Add(Space& refSpace, const ��λ���� ����, const Position& refPosition);
	��ԴComponent(const ��λ���� ����) :m_����(����) {}
	const ��λ���� m_����;
	int m_�ɲɼ����� = 200;
};

