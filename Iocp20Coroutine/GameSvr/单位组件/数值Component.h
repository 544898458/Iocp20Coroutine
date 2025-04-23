#pragma once
#include <map>
class Entity;
enum ��������;

/// <summary>
/// ��¼����������������
/// </summary>
class ��ֵComponent
{
public:
	��ֵComponent(Entity& refEntity);
	static ��ֵComponent& AddComponent(Entity& refEntity);
	static int Get(const Entity& refEntity, const �������� ����);
	static int Set(Entity& refEntity, const �������� ����, const int ��ֵ);
	static int �ı�(Entity& refEntity, const �������� ����, const int �仯);
	int Get(const �������� ����)const;
	int Set(const �������� ����,const int ��ֵ);
	int �������(const ��������& ����);
	int �ı�(const �������� ����, const int �仯);
	int ȷ�������ڷ�Χ��(const �������� ����);
private:
	Entity& m_refEntity;
	std::map<��������, int> m_map������ֵ;
};

