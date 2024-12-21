#include "pch.h"
#include "AoiComponent.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "Space.h"
#include "AttackComponent.h"
void AoiComponent::Add(Space& refSpace, Entity& refEntity)
{
	refEntity.m_upAoi.reset(new AoiComponent(refSpace, refEntity));// std::make_unique<AoiComponent>();
	refEntity.m_upAoi->����Space();
}
#define CHECK_WP_CONTINUE( wp ) \
{\
	if (wp.expired()) \
	{\
		LOG(ERROR)<< #wp<< ",expired";\
		assert(false);\
		continue;\
	}\
}
#define CHECK_AOI_CONTINUE( ref ) \
{\
	if (!(ref).m_upAoi) \
	{\
		LOG(ERROR)<< #ref << ",m_upAoi";\
		assert(false);\
		continue;\
	}\
}
void AoiComponent::�ܿ�����һ����˶�������()
{
	const auto [id, _, __] = ����(m_refEntity);
	{
		const auto [iter, ok] = m_refEntity.m_refSpace.m_map����һ����[id].insert({ m_refEntity.Id,m_refEntity.weak_from_this() });
		assert(ok);
	}
	auto& refMapWp�ܿ�����һ�� = m_refEntity.m_refSpace.m_map�ܿ�����һ��[id];
	//{
	//	const auto [iter, ok] = refMapWp�ܿ�����һ��.insert({ m_refEntity.Id,m_refEntity.weak_from_this() });
	//	assert(ok);
	//}
	for (auto [k, wp] : refMapWp�ܿ�����һ��)
	{
		CHECK_WP_CONTINUE(wp);
		auto& refEntity = *wp.lock();
		CHECK_AOI_CONTINUE(refEntity);
		refEntity.m_upAoi->m_map���ܿ�����[m_refEntity.Id] = m_refEntity.weak_from_this();
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
		const auto sizeErase = refEntity.m_upAoi->m_map���ܿ�����.erase(m_refEntity.Id);// = m_refEntity.weak_from_this();
		assert(1 == sizeErase);
	}

	{
		const auto sizeErase = m_refEntity.m_refSpace.m_map����һ����[id].erase(m_refEntity.Id);// , m_refEntity.weak_from_this() });
		assert(1 == sizeErase);
	}
	//{
	//	const auto [iter, ok] = refMapWp�ܿ�����һ��.insert({ m_refEntity.Id,m_refEntity.weak_from_this() });
	//	assert(ok);
	//}
}
void AoiComponent::����Space()
{
	������Щ����(�ܿ����ĸ���Vec());
	�ܿ�����һ����˶�������();
}


void AoiComponent::�뿪Space()
{
	��������Щ����(�ܿ����ĸ���Vec());
	const auto [id, _, __] = ����(m_refEntity);
	const auto sizeErase = m_refEntity.m_refSpace.m_map����һ����[id].erase(m_refEntity.Id);
	assert(1 == sizeErase);
}
void AoiComponent::����(Entity& refEntity����)
{
	assert(refEntity����.m_upAoi);
	if (!refEntity����.m_upAoi)//����ģ�ͣ��໥��ס
		return;

	refEntity����.m_upAoi->m_map�ܿ����ҵ�[m_refEntity.Id] = m_refEntity.weak_from_this();
	m_refEntity.m_upAoi->m_map���ܿ�����[refEntity����.Id] = refEntity����.weak_from_this();
}

void AoiComponent::������(Entity& refEntity����)
{
	assert(refEntity����.m_upAoi);
	if (!refEntity����.m_upAoi)//����ģ�ͣ��໥��ס
		return;

	refEntity����.m_upAoi->m_map�ܿ����ҵ�.erase(m_refEntity.Id);// ] = m_refEntity.weak_from_this();
	m_refEntity.m_upAoi->m_map���ܿ�����.erase(refEntity����.Id);// ] = refEntity����.weak_from_this();
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
		assert(ok);
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
		assert(1 == sizeɾ����);
	}
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

	������Щ����(vec���������ĸ���);
	��������Щ����(vecɾ�������ٿ����ĸ���);

	m_refSpace.m_map����һ����[idOld].erase(m_refEntity.Id);
	m_refSpace.m_map����һ����[idNew][m_refEntity.Id] = m_refEntity.weak_from_this();
}
void AoiComponent::OnDestory()
{
	for (auto [k, wp] : m_map���ܿ�����)
	{
		assert(!wp.expired());
		if (wp.expired())
			continue;

		auto sp = wp.lock();
		assert(sp->m_upAoi);
		if (!sp->m_upAoi)
			continue;

		sp->m_upAoi->m_map�ܿ����ҵ�.erase(m_refEntity.Id);
	}
	m_map���ܿ�����.clear();

	for (auto [k, wp] : m_map�ܿ����ҵ�)
	{
		assert(!wp.expired());
		if (wp.expired())
			continue;

		auto sp = wp.lock();
		assert(sp->m_upAoi);
		if (!sp->m_upAoi)
			continue;

		sp->m_upAoi->m_map���ܿ�����.erase(m_refEntity.Id);
	}
	m_map�ܿ����ҵ�.clear();
}

AoiComponent::AoiComponent(Space& refSpace, Entity& refEntity) :m_refSpace(refSpace), m_refEntity(refEntity)
{
}

const uint8_t u8���������α߳� = 10;

int AoiComponent::����Id(const int32_t i32����X, const int32_t i32����Z)
{
	const uint16_t u16X����Ŵ��� = 10000;//z���겻�ܳ���9999
	const int32_t i32����ID = i32����X * u16X����Ŵ��� + i32����Z;
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
			assert(ok);
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
		i32��Ұ��Χ = std::max<int>((int)m_refEntity.m_spAttack->m_f�������, i32��Ұ��Χ);

	i32��Ұ��Χ /= u8���������α߳�;
	++i32��Ұ��Χ;//����ȥ��С���ˣ������һ��

	for (int x = i32����X - i32��Ұ��Χ; x <= i32����X + i32��Ұ��Χ; ++x)
	{
		for (int z = i32����Z - i32��Ұ��Χ; z <= i32����Z + i32��Ұ��Χ; ++z)
		{
			const auto id = ����Id(x, z);
			const auto [iter, ok] = set.insert(id);
			assert(ok); //boolֵ���ʾ�Ƿ�����˸�Ԫ��
		}
	}

	return set;
}
