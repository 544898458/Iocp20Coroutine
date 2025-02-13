#include "pch.h"
#include "AoiComponent.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "Space.h"
#include "AttackComponent.h"
#include "�ر�Component.h"

void AoiComponent::Add(Space& refSpace, Entity& refEntity, const int32_t i32��Ұ��Χ)
{
	refEntity.m_upAoi.reset(new AoiComponent(refSpace, refEntity));// std::make_unique<AoiComponent>();
	refEntity.m_upAoi->m_i32��Ұ��Χ = i32��Ұ��Χ;
	refEntity.m_upAoi->����Space();
}

void AoiComponent::�ܿ�����һ����˶�������()
{
	const auto [id, _, __] = ����(m_refEntity);
	�ܿ�����һ����˶�������(id);
}

void AoiComponent::�ܿ�����һ����˶�������(const int id)
{
	{
		const auto [iter, ok] = m_refEntity.m_refSpace.m_map����һ����[id].insert({ m_refEntity.Id,m_refEntity.weak_from_this() });
		_ASSERT(ok);
	}
	auto& refMapWp�ܿ�����һ�� = m_refEntity.m_refSpace.m_map�ܿ�����һ��[id];
	//{
	//	const auto [iter, ok] = refMapWp�ܿ�����һ��.insert({ m_refEntity.Id,m_refEntity.weak_from_this() });
	//	_ASSERT(ok);
	//}
	for (auto [k, wp] : refMapWp�ܿ�����һ��)
	{
		CHECK_WP_CONTINUE(wp);
		auto& refEntity = *wp.lock();
		CHECK_AOI_CONTINUE(refEntity);
		refEntity.m_upAoi->����(m_refEntity);
	}
}
void AoiComponent::�ܿ�����һ����˶���������()
{
	const auto [id, _, __] = ����(m_refEntity);
	auto& refMapWp�ܿ�����һ�� = m_refEntity.m_refSpace.m_map�ܿ�����һ��[id];
	for (auto [k, wp] : refMapWp�ܿ�����һ��)
	{
		CHECK_WP_CONTINUE(wp);
		auto& refEntity = *wp.lock();
		CHECK_AOI_CONTINUE(refEntity);
		refEntity.m_upAoi->������(m_refEntity);
	}

	{
		const auto sizeErase = m_refEntity.m_refSpace.m_map����һ����[id].erase(m_refEntity.Id);// , m_refEntity.weak_from_this() });
		_ASSERT(1 == sizeErase);
	}
	//{
	//	const auto [iter, ok] = refMapWp�ܿ�����һ��.insert({ m_refEntity.Id,m_refEntity.weak_from_this() });
	//	_ASSERT(ok);
	//}
}
void AoiComponent::����Space()
{
	�ܿ�����һ����˶�������();
	������Щ����(�ܿ����ĸ���Vec());
}


void AoiComponent::�뿪Space()
{
	if (m_refSpace.GetEntity(m_refEntity.Id).expired())
	{
		LOG(INFO) << "�����ڵر���";
		return;
	}
	��������Щ����(�ܿ����ĸ���Vec());
	�ܿ�����һ����˶���������();
}
void AoiComponent::����(Entity& refEntity����)
{
	_ASSERT(refEntity����.m_upAoi);
	if (!refEntity����.m_upAoi)//����ģ�ͣ��໥��ס
		return;

	{
		const auto [iter, ok] = refEntity����.m_upAoi->m_map�ܿ����ҵ�.insert({ m_refEntity.Id, m_refEntity.weak_from_this() });
		_ASSERT(ok);
	}
	{
		const auto [iter, ok] = m_refEntity.m_upAoi->m_map���ܿ�����.insert({ refEntity����.Id, refEntity����.weak_from_this() });
		_ASSERT(ok);

		if (m_refEntity.m_spAttack)
		{
			m_refEntity.m_spAttack->m_b�����µ�Ŀ�� = true;
		}
		else if (m_refEntity.m_sp�ر�) {
			for (auto sp : m_refEntity.m_sp�ر�->m_listSpEntity)
			{
				if (sp->m_spAttack)
				{
					sp->m_spAttack->m_b�����µ�Ŀ�� = true;
				}
			}
		}
	}
}

void AoiComponent::������(Entity& refEntity����)
{
	_ASSERT(refEntity����.m_upAoi);
	if (!refEntity����.m_upAoi)//����ģ�ͣ��໥��ס
		return;

	{
		const auto sizeErase = refEntity����.m_upAoi->m_map�ܿ����ҵ�.erase(m_refEntity.Id);// ] = m_refEntity.weak_from_this();
		_ASSERT(sizeErase);
	}
	{
		const auto sizeErase = m_refEntity.m_upAoi->m_map���ܿ�����.erase(refEntity����.Id);// ] = refEntity����.weak_from_this();
		_ASSERT(sizeErase);
	}
}

void AoiComponent::������Щ����(const std::vector<int32_t>& vec���������ĸ���Id)
{
	for (const auto id : vec���������ĸ���Id)
	{
		auto& refMapWp = m_refSpace.m_map����һ����[id];
		for (auto [k, wp] : refMapWp)
		{
			CHECK_WP_CONTINUE(wp);
			auto& refEntity = *wp.lock();
			CHECK_AOI_CONTINUE(refEntity);

			m_refEntity.m_upAoi->����(refEntity);
		}

		auto [iter, ok] = m_refSpace.m_map�ܿ�����һ��[id].insert({ m_refEntity.Id, m_refEntity.weak_from_this() });
		_ASSERT(ok);
	}
}

void AoiComponent::��������Щ����(const std::vector<int32_t>& vecɾ�����ٿ������ϸ���Id)
{
	for (const auto id : vecɾ�����ٿ������ϸ���Id)
	{
		auto& refMapWp = m_refSpace.m_map����һ����[id];
		for (auto [k, wp] : refMapWp)
		{
			CHECK_WP_CONTINUE(wp);
			auto& refEntity = *wp.lock();
			CHECK_AOI_CONTINUE(refEntity);

			m_refEntity.m_upAoi->������(refEntity);
		}

		const auto sizeɾ���� = m_refSpace.m_map�ܿ�����һ��[id].erase(m_refEntity.Id);// = m_refEntity.weak_from_this();
		_ASSERT(1 == sizeɾ����);
	}
}

void AoiComponent::OnAfterChangePos()
{

}

void AoiComponent::OnBeforeChangePos(const Position& posNew)
{
	const auto [idOld, _, ____] = ����(m_refEntity);
	const auto [idNew, ___, __] = ����(posNew);
	if (idOld == idNew)
		return;

	const auto set�ܿ���Old = �ܿ����ĸ���();
	const auto set�ܿ���New = �ܿ����ĸ���(posNew);
	std::vector<int32_t> vec���������ĸ���;
	decltype(vec���������ĸ���) vecɾ�������ٿ����ĸ���;
	std::set_difference(set�ܿ���New.begin(), set�ܿ���New.end(), set�ܿ���Old.begin(), set�ܿ���Old.end(), std::back_inserter(vec���������ĸ���));
	std::set_difference(set�ܿ���Old.begin(), set�ܿ���Old.end(), set�ܿ���New.begin(), set�ܿ���New.end(), std::back_inserter(vecɾ�������ٿ����ĸ���));

	��������Щ����(vecɾ�������ٿ����ĸ���);
	�ܿ�����һ����˶���������();

	������Щ����(vec���������ĸ���);
	�ܿ�����һ����˶�������(idNew);
}
void AoiComponent::OnDestory()
{
	�뿪Space();
	//for (auto [k, wp] : m_map���ܿ�����)
	//{
	//	_ASSERT(!wp.expired());
	//	if (wp.expired())
	//		continue;

	//	auto sp = wp.lock();
	//	_ASSERT(sp->m_upAoi);
	//	if (!sp->m_upAoi)
	//		continue;

	//	sp->m_upAoi->m_map�ܿ����ҵ�.erase(m_refEntity.Id);
	//}
	//m_map���ܿ�����.clear();

	//for (auto [k, wp] : m_map�ܿ����ҵ�)
	//{
	//	_ASSERT(!wp.expired());
	//	if (wp.expired())
	//		continue;

	//	auto sp = wp.lock();
	//	_ASSERT(sp->m_upAoi);
	//	if (!sp->m_upAoi)
	//		continue;

	//	sp->m_upAoi->m_map���ܿ�����.erase(m_refEntity.Id);
	//}
	//m_map�ܿ����ҵ�.clear();
}

AoiComponent::AoiComponent(Space& refSpace, Entity& refEntity) :m_refSpace(refSpace), m_refEntity(refEntity)
{
}

const uint8_t u8���������α߳� = 10;

int AoiComponent::����Id(const int32_t i32����X, const int32_t i32����Z)
{
	const uint16_t u16X����Ŵ��� = 10000;//z���겻�ܳ���9999
	const int32_t i32����ID = (u16X����Ŵ��� + i32����X) * u16X����Ŵ��� + (u16X����Ŵ��� + i32����Z);
	return i32����ID;
}
std::tuple<int, int, int> AoiComponent::����(const Position& refPos)
{
	const uint16_t u16X����Ŵ��� = 10000;//z���겻�ܳ���9999
	const int32_t i32����X = ((int)refPos.x) / u8���������α߳�;
	const int32_t i32����Z = ((int)refPos.z) / u8���������α߳�;
	const int32_t i32����ID = ����Id(i32����X, i32����Z);
	return { i32����ID ,i32����X, i32����Z };
}


std::tuple<int, int, int> AoiComponent::����(const Entity& refEntity)
{
	return ����(refEntity.Pos());
}


std::unordered_map<int, std::unordered_map<uint64_t, WpEntity>> AoiComponent::�ܿ����ĸ������Entity() const
{
	std::unordered_map<int, std::unordered_map<uint64_t, WpEntity>> map;
	const auto set = �ܿ����ĸ���();
	for (const auto id : set)
	{
		auto mapWp = m_refSpace.m_map����һ����[id];
		for (auto [k, wp] : mapWp)
		{
			auto [iter, ok] = map[id].insert({ k,wp });
			_ASSERT(ok);
		}
	}
	return map;
}

std::vector<int32_t> AoiComponent::�ܿ����ĸ���Vec() const
{
	std::vector<int32_t> vec;
	const auto set = �ܿ����ĸ���();
	std::copy(set.begin(), set.end(), std::back_inserter(vec));
	return vec;
}

std::set<int32_t> AoiComponent::�ܿ����ĸ���() const
{
	return �ܿ����ĸ���(m_refEntity.Pos());
}
std::set<int32_t> AoiComponent::�ܿ����ĸ���(const Position& pos) const
{
	std::set<int32_t> set;
	const auto [i32����Id, i32����X, i32����Z] = AoiComponent::����(pos);

	int i32��Ұ��Χ = m_i32��Ұ��Χ;
	if (m_refEntity.m_spAttack)
		i32��Ұ��Χ = std::max<int>((int)m_refEntity.m_spAttack->m_ս������.f�������, i32��Ұ��Χ);

	i32��Ұ��Χ /= u8���������α߳�;
	++i32��Ұ��Χ;//����ȥ��С���ˣ������һ��

	for (int x = i32����X - i32��Ұ��Χ; x <= i32����X + i32��Ұ��Χ; ++x)
	{
		for (int z = i32����Z - i32��Ұ��Χ; z <= i32����Z + i32��Ұ��Χ; ++z)
		{
			const auto id = ����Id(x, z);
			const auto [iter, ok] = set.insert(id);
			_ASSERT(ok); //boolֵ���ʾ�Ƿ�����˸�Ԫ��
		}
	}

	return set;
}
