#pragma once
#include <unordered_map>
#include <unordered_set>
#include "SpEntity.h"
class Space;
struct Position;
class AoiComponent
{
public:
	static void Add(Space& refSpace, Entity& refEntity);
	static std::tuple<int, int, int> ����(const Position& refPos);
	static std::tuple<int, int, int> ����(const Entity& refEntity);
	std::unordered_map<uint64_t, WpEntity> �ܿ����ĸ������Entity() const;
	std::unordered_set<int32_t> �ܿ����ĸ���() const;
	std::unordered_set<int32_t> �ܿ����ĸ���(const Position& pos) const;
	void OnBeforeChangePos(const Position& posNew);
	void ����Space();
	void �뿪Space();
	void Add����(Entity& refEntity);
	void ������(Entity& refEntity����);
	void OnDestory();
	std::unordered_map<uint64_t, WpEntity> m_map���ܿ�����;
	std::unordered_map<uint64_t, WpEntity> m_map�ܿ����ҵ�;
	int m_i32��Ұ��Χ = 0;//�ӿ�����Ϊ��Ļ��Χ�������Ⱦ��䷶Χ��һ��
private:
	AoiComponent(Space& refSpace, Entity& refEntity);
	Space& m_refSpace;
	Entity& m_refEntity;

};

