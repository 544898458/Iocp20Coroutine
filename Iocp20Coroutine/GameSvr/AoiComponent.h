#pragma once
#include <unordered_map>
#include <set>
#include "SpEntity.h"
class Space;
struct Position;
class AoiComponent
{
public:
	/// <summary>
	/// 
	/// </summary>
	/// <param name="refSpace"></param>
	/// <param name="refEntity"></param>
	/// <param name="i32��Ұ��Χ">�����0���þ��䷶Χ������Ұ��Χ</param>
	static void Add(Space& refSpace, Entity& refEntity, const int32_t i32��Ұ��Χ);
	static std::tuple<int, int, int> ����(const Position& refPos);
	static std::tuple<int, int, int> ����(const Entity& refEntity);
	static int ����Id(const int32_t i32����X, const int32_t i32����Z);
	std::unordered_map<int, std::unordered_map<uint64_t, WpEntity>> �ܿ����ĸ������Entity() const;
	std::set<int32_t> �ܿ����ĸ���() const;
	std::set<int32_t> �ܿ����ĸ���(const Position& pos) const;
	std::vector<int32_t> �ܿ����ĸ���Vec() const;
	void �ܿ�����һ����˶�������();
	void �ܿ�����һ����˶���������();
	void ������Щ����(const std::vector<int32_t>& vec���������ĸ���Id);
	void ��������Щ����(const std::vector<int32_t>& vecɾ�����ٿ������ϸ���Id);
	void OnBeforeChangePos(const Position& posNew);
	void ����Space();
	void �뿪Space();
	void ����(Entity& refEntity����);
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

