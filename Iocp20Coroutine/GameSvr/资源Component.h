#pragma once
class Space;
enum ��Դ����;
struct Position;
class ��ԴComponent
{
public:
	static void Add(Space& refSpace, const ��Դ���� ����, const Position& refPosition);
	��ԴComponent(const ��Դ���� ����) :m_����(����) {}
	const ��Դ���� m_����;
	int m_�ɲɼ����� = 200;
};

