#pragma once
class Space;
enum ��Դ����;
class ��ԴComponent
{
public:
	static void Add(Space& refSpace, const ��Դ���� ����, const Position& refPosition);
	��ԴComponent(const ��Դ���� ����) :m_����(����) {}
	const ��Դ���� m_����;
};

