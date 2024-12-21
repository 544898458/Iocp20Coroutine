#include "pch.h"
#include "AoiComponent.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "Space.h"
#include "AttackComponent.h"
void AoiComponent::Add(Space& refSpace, Entity& refEntity)
{
	refEntity.m_upAoi.reset(new AoiComponent(refSpace, refEntity));// std::make_unique<AoiComponent>();
	refEntity.m_upAoi->进入Space();
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
void AoiComponent::进入Space()
{
	const auto [i32格子Id, i32格子X, i32格子Z] = AoiComponent::格子(m_refEntity);
	auto& refMapWp能看到这一格 = m_refSpace.m_map能看到这一格[i32格子Id];
	for (auto [k, wp] : refMapWp能看到这一格)
	{
		CHECK_WP_CONTINUE(wp);
		auto& refEntity = *wp.lock();
		CHECK_AOI_CONTINUE(refEntity)
		refEntity.m_upAoi->看到(m_refEntity);
	}

	const auto vecMap = 能看到的格子里的Entity();
	for (auto [k, wp] : vecMap)
	{
		CHECK_WP_CONTINUE(wp);
		auto& refEntity = *wp.lock();
		CHECK_AOI_CONTINUE(refEntity);
		看到(refEntity);
	}
}

void AoiComponent::看到(Entity& refEntity被看)
{
	assert(refEntity被看.m_upAoi);
	if (!refEntity被看.m_upAoi)//灯塔模型，相互记住
		return;

	refEntity被看.m_upAoi->m_map能看到我的[m_refEntity.Id] = m_refEntity.weak_from_this();
	m_refEntity.m_upAoi->m_map我能看到的[refEntity被看.Id] = refEntity被看.weak_from_this();
}

void AoiComponent::看不到(Entity& refEntity被看)
{
	assert(refEntity被看.m_upAoi);
	if (!refEntity被看.m_upAoi)//灯塔模型，相互记住
		return;

	refEntity被看.m_upAoi->m_map能看到我的.erase(m_refEntity.Id);// ] = m_refEntity.weak_from_this();
	m_refEntity.m_upAoi->m_map我能看到的.erase(refEntity被看.Id);// ] = refEntity被看.weak_from_this();
}


void AoiComponent::离开Space()
{
	const auto [i32格子Id, i32格子X, i32格子Z] = AoiComponent::格子(m_refEntity);
	auto& refMapWp能看到这一格 = m_refSpace.m_map能看到这一格[i32格子Id];
	for (auto [k, wp] : refMapWp能看到这一格)
	{
		CHECK_WP_CONTINUE(wp);
		auto& refEntity = *wp.lock();
		if (!refEntity.m_upAoi)
		{
			LOG(ERROR) << "";
			assert(false);
			continue;
		}

		refEntity.m_upAoi->看不到(m_refEntity);
	}

	const auto vecMap = 能看到的格子里的Entity();
	for (auto [k, wp] : vecMap)
	{
		CHECK_WP_CONTINUE(wp);
		auto& refEntity = *wp.lock();
		if (!refEntity.m_upAoi)
			continue;//资源或中立动物之类

		看不到(refEntity);
	}
}

void AoiComponent::OnBeforeChangePos(const Position& posNew)
{
	const auto [idOld, _, ____] = 格子(m_refEntity);
	const auto [idNew, ___, __] = 格子(posNew);
	if (idOld == idNew)
		return;

	const auto set能看到Old = 能看到的格子();
	const auto set能看到New = 能看到的格子(posNew);
	std::vector<int32_t> vec新增看到的格子;
	decltype(vec新增看到的格子) vec删除不会再看到的格子;
	std::set_difference(set能看到New.begin(), set能看到New.end(), set能看到Old.begin(), set能看到Old.end(), std::back_inserter(vec新增看到的格子));
	std::set_difference(set能看到Old.begin(), set能看到Old.end(), set能看到New.begin(), set能看到New.end(), std::back_inserter(vec删除不会再看到的格子));

	for (const auto id : vec新增看到的格子)
	{
		auto& refMapWp = m_refSpace.m_map在这一格里[id];
		for (auto [k, wp] : refMapWp)
		{
			CHECK_WP_CONTINUE(wp);
			auto &refEntity = *wp.lock();
			CHECK_AOI_CONTINUE(refEntity);
			
			m_refEntity.m_upAoi->看到(refEntity);
		}

		m_refSpace.m_map能看到这一格[id][m_refEntity.Id] = m_refEntity.weak_from_this();
	}
	for (const auto id : vec删除不会再看到的格子)
	{
		auto& refMapWp = m_refSpace.m_map在这一格里[id];
		for (auto [k, wp] : refMapWp)
		{
			CHECK_WP_CONTINUE(wp);
			auto& refEntity = *wp.lock();
			CHECK_AOI_CONTINUE(refEntity);

			m_refEntity.m_upAoi->看不到(refEntity);
		}

		const auto size删除数 = m_refSpace.m_map能看到这一格[id].erase(m_refEntity.Id);// = m_refEntity.weak_from_this();
		assert(1 == size删除数);
	}

	m_refSpace.m_map在这一格里[idOld].erase(m_refEntity.Id);
	m_refSpace.m_map在这一格里[idNew][m_refEntity.Id]=m_refEntity.weak_from_this();
}
void AoiComponent::OnDestory()
{
	for (auto [k, wp] : m_map我能看到的)
	{
		assert(!wp.expired());
		if (wp.expired())
			continue;

		auto sp = wp.lock();
		assert(sp->m_upAoi);
		if (!sp->m_upAoi)
			continue;

		sp->m_upAoi->m_map能看到我的.erase(m_refEntity.Id);
	}
	m_map我能看到的.clear();

	for (auto [k, wp] : m_map能看到我的)
	{
		assert(!wp.expired());
		if (wp.expired())
			continue;

		auto sp = wp.lock();
		assert(sp->m_upAoi);
		if (!sp->m_upAoi)
			continue;

		sp->m_upAoi->m_map我能看到的.erase(m_refEntity.Id);
	}
	m_map能看到我的.clear();
}

AoiComponent::AoiComponent(Space& refSpace, Entity& refEntity) :m_refSpace(refSpace), m_refEntity(refEntity)
{
}

const uint8_t u8格子正方形边长 = 10;

std::tuple<int, int, int> AoiComponent::格子(const Position& refPos)
{
	const uint16_t u16X坐标放大倍数 = 10000;//z坐标不能超过9999
	const int32_t i32格子X = ((int)refPos.x) / u8格子正方形边长;
	const int32_t i32格子Z = ((int)refPos.z) / u8格子正方形边长;
	const int32_t i32格子ID = i32格子X * u16X坐标放大倍数 + i32格子Z;
	return { i32格子ID ,i32格子X, i32格子Z };
}


std::tuple<int, int, int> AoiComponent::格子(const Entity& refEntity)
{
	return 格子(refEntity.Pos());
}


std::unordered_map<uint64_t, WpEntity> AoiComponent::能看到的格子里的Entity() const
{
	std::unordered_map<uint64_t, WpEntity> map;
	const auto set = 能看到的格子();
	for (const auto id : set)
	{
		auto mapWp = m_refSpace.m_map在这一格里[id];
		for (auto [k, wp] : mapWp)
		{
			auto [iter, ok] = map.insert({ k,wp });
			assert(ok);
		}
	}
	return map;
}
std::unordered_set<int32_t> AoiComponent::能看到的格子() const
{
	return 能看到的格子(m_refEntity.Pos());
}
std::unordered_set<int32_t> AoiComponent::能看到的格子(const Position& pos) const
{
	std::unordered_set<int32_t> set;
	const auto [i32格子Id, i32格子X, i32格子Z] = AoiComponent::格子(pos);

	int i32视野范围 = m_i32视野范围;
	if (m_refEntity.m_spAttack)
		i32视野范围 = std::max<int>((int)m_refEntity.m_spAttack->m_f警戒距离, i32视野范围);

	i32视野范围 /= u8格子正方形边长;
	++i32视野范围;//上面去掉小数了，这里加一点

	for (int x = i32格子X - i32视野范围; x < i32格子X + i32视野范围; ++x)
	{
		for (int z = i32格子Z - i32视野范围; z < i32格子Z + i32视野范围; ++z)
		{
			const auto [id, _, __] = 格子({ (float)x,(float)z });
			const auto [iter, ok] = set.insert(id);
			assert(ok);
		}
	}

	return set;
}
