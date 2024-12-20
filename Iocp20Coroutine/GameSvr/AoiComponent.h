#pragma once
#include <unordered_map>
#include "SpEntity.h"
class Space;
class AoiComponent
{
public:
	static void Add(Space& refSpace, Entity& refEntity);
	void OnDestory();
	std::unordered_map<uint64_t, WpEntity> m_map���ܿ�����;
	std::unordered_map<uint64_t, WpEntity> m_map�ܿ����ҵ�;
	int m_i32��Ұ��Χ = 0;//�ӿ�����Ϊ��Ļ��Χ�������Ⱦ��䷶Χ��һ��
private:
	AoiComponent(Space& refSpace, Entity& refEntity);
	Space& m_refSpace;
	Entity& m_refEntity;
	
};

