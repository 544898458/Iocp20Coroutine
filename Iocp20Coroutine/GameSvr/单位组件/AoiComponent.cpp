#include "pch.h"
#include "AoiComponent.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "Space.h"
#include "AttackComponent.h"
#include "地堡Component.h"

void AoiComponent::Add(Space& refSpace, Entity& refEntity, const int32_t i32视野范围)
{
	refEntity.m_upAoi.reset(new AoiComponent(refSpace, refEntity));// std::make_unique<AoiComponent>();
	refEntity.m_upAoi->m_i32视野范围 = i32视野范围;
	refEntity.m_upAoi->进入Space();
}

void AoiComponent::能看到这一格的人都看到我()
{
	const auto [id, _, __] = 格子(m_refEntity);
	能看到这一格的人都看到我(id);
}

void AoiComponent::能看到这一格的人都看到我(const int id)
{
	{
		const auto [iter, ok] = m_refEntity.m_refSpace.m_map在这一格里[id].insert({ m_refEntity.Id,m_refEntity.weak_from_this() });
		_ASSERT(ok);
	}
	auto& refMapWp能看到这一格 = m_refEntity.m_refSpace.m_map能看到这一格[id];
	//{
	//	const auto [iter, ok] = refMapWp能看到这一格.insert({ m_refEntity.Id,m_refEntity.weak_from_this() });
	//	_ASSERT(ok);
	//}
	for (auto [k, wp] : refMapWp能看到这一格)
	{
		CHECK_WP_CONTINUE(wp);
		auto& refEntity = *wp.lock();
		CHECK_AOI_CONTINUE(refEntity);
		refEntity.m_upAoi->看到(m_refEntity);
	}
}
void AoiComponent::能看到这一格的人都看不到我()
{
	const auto [id, _, __] = 格子(m_refEntity);
	auto& refMapWp能看到这一格 = m_refEntity.m_refSpace.m_map能看到这一格[id];
	for (auto [k, wp] : refMapWp能看到这一格)
	{
		CHECK_WP_CONTINUE(wp);
		auto& refEntity = *wp.lock();
		CHECK_AOI_CONTINUE(refEntity);
		refEntity.m_upAoi->看不到(m_refEntity);
	}

	{
		const auto sizeErase = m_refEntity.m_refSpace.m_map在这一格里[id].erase(m_refEntity.Id);// , m_refEntity.weak_from_this() });
		_ASSERT(1 == sizeErase);
	}
	//{
	//	const auto [iter, ok] = refMapWp能看到这一格.insert({ m_refEntity.Id,m_refEntity.weak_from_this() });
	//	_ASSERT(ok);
	//}
}
void AoiComponent::进入Space()
{
	能看到这一格的人都看到我();
	看到这些格子(能看到的格子Vec());
}


void AoiComponent::离开Space()
{
	if (m_refSpace.GetEntity(m_refEntity.Id).expired())
	{
		LOG(INFO) << "可能在地堡中";
		return;
	}
	看不到这些格子(能看到的格子Vec());
	能看到这一格的人都看不到我();
}
void AoiComponent::看到(Entity& refEntity被看)
{
	_ASSERT(refEntity被看.m_upAoi);
	if (!refEntity被看.m_upAoi)//灯塔模型，相互记住
		return;

	{
		const auto [iter, ok] = refEntity被看.m_upAoi->m_map能看到我的.insert({ m_refEntity.Id, m_refEntity.weak_from_this() });
		_ASSERT(ok);
	}
	{
		const auto [iter, ok] = m_refEntity.m_upAoi->m_map我能看到的.insert({ refEntity被看.Id, refEntity被看.weak_from_this() });
		_ASSERT(ok);

		if (m_refEntity.m_spAttack)
		{
			m_refEntity.m_spAttack->m_b搜索新的目标 = true;
		}
		else if (m_refEntity.m_sp地堡) {
			for (auto sp : m_refEntity.m_sp地堡->m_listSpEntity)
			{
				if (sp->m_spAttack)
				{
					sp->m_spAttack->m_b搜索新的目标 = true;
				}
			}
		}
	}
}

void AoiComponent::看不到(Entity& refEntity被看)
{
	_ASSERT(refEntity被看.m_upAoi);
	if (!refEntity被看.m_upAoi)//灯塔模型，相互记住
		return;

	{
		const auto sizeErase = refEntity被看.m_upAoi->m_map能看到我的.erase(m_refEntity.Id);// ] = m_refEntity.weak_from_this();
		_ASSERT(sizeErase);
	}
	{
		const auto sizeErase = m_refEntity.m_upAoi->m_map我能看到的.erase(refEntity被看.Id);// ] = refEntity被看.weak_from_this();
		_ASSERT(sizeErase);
	}
}

void AoiComponent::看到这些格子(const std::vector<int32_t>& vec新增看到的格子Id)
{
	for (const auto id : vec新增看到的格子Id)
	{
		auto& refMapWp = m_refSpace.m_map在这一格里[id];
		for (auto [k, wp] : refMapWp)
		{
			CHECK_WP_CONTINUE(wp);
			auto& refEntity = *wp.lock();
			CHECK_AOI_CONTINUE(refEntity);

			m_refEntity.m_upAoi->看到(refEntity);
		}

		auto [iter, ok] = m_refSpace.m_map能看到这一格[id].insert({ m_refEntity.Id, m_refEntity.weak_from_this() });
		_ASSERT(ok);
	}
}

void AoiComponent::看不到这些格子(const std::vector<int32_t>& vec删除不再看到的老格子Id)
{
	for (const auto id : vec删除不再看到的老格子Id)
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
		_ASSERT(1 == size删除数);
	}
}

void AoiComponent::OnAfterChangePos()
{

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

	看不到这些格子(vec删除不会再看到的格子);
	能看到这一格的人都看不到我();

	看到这些格子(vec新增看到的格子);
	能看到这一格的人都看到我(idNew);
}
void AoiComponent::OnDestory()
{
	离开Space();
	//for (auto [k, wp] : m_map我能看到的)
	//{
	//	_ASSERT(!wp.expired());
	//	if (wp.expired())
	//		continue;

	//	auto sp = wp.lock();
	//	_ASSERT(sp->m_upAoi);
	//	if (!sp->m_upAoi)
	//		continue;

	//	sp->m_upAoi->m_map能看到我的.erase(m_refEntity.Id);
	//}
	//m_map我能看到的.clear();

	//for (auto [k, wp] : m_map能看到我的)
	//{
	//	_ASSERT(!wp.expired());
	//	if (wp.expired())
	//		continue;

	//	auto sp = wp.lock();
	//	_ASSERT(sp->m_upAoi);
	//	if (!sp->m_upAoi)
	//		continue;

	//	sp->m_upAoi->m_map我能看到的.erase(m_refEntity.Id);
	//}
	//m_map能看到我的.clear();
}

AoiComponent::AoiComponent(Space& refSpace, Entity& refEntity) :m_refSpace(refSpace), m_refEntity(refEntity)
{
}

const uint8_t u8格子正方形边长 = 10;

int AoiComponent::格子Id(const int32_t i32格子X, const int32_t i32格子Z)
{
	const uint16_t u16X坐标放大倍数 = 10000;//z坐标不能超过9999
	const int32_t i32格子ID = (u16X坐标放大倍数 + i32格子X) * u16X坐标放大倍数 + (u16X坐标放大倍数 + i32格子Z);
	return i32格子ID;
}
std::tuple<int, int, int> AoiComponent::格子(const Position& refPos)
{
	const uint16_t u16X坐标放大倍数 = 10000;//z坐标不能超过9999
	const int32_t i32格子X = ((int)refPos.x) / u8格子正方形边长;
	const int32_t i32格子Z = ((int)refPos.z) / u8格子正方形边长;
	const int32_t i32格子ID = 格子Id(i32格子X, i32格子Z);
	return { i32格子ID ,i32格子X, i32格子Z };
}


std::tuple<int, int, int> AoiComponent::格子(const Entity& refEntity)
{
	return 格子(refEntity.Pos());
}


std::unordered_map<int, std::unordered_map<uint64_t, WpEntity>> AoiComponent::能看到的格子里的Entity() const
{
	std::unordered_map<int, std::unordered_map<uint64_t, WpEntity>> map;
	const auto set = 能看到的格子();
	for (const auto id : set)
	{
		auto mapWp = m_refSpace.m_map在这一格里[id];
		for (auto [k, wp] : mapWp)
		{
			auto [iter, ok] = map[id].insert({ k,wp });
			_ASSERT(ok);
		}
	}
	return map;
}

std::vector<int32_t> AoiComponent::能看到的格子Vec() const
{
	std::vector<int32_t> vec;
	const auto set = 能看到的格子();
	std::copy(set.begin(), set.end(), std::back_inserter(vec));
	return vec;
}

std::set<int32_t> AoiComponent::能看到的格子() const
{
	return 能看到的格子(m_refEntity.Pos());
}
std::set<int32_t> AoiComponent::能看到的格子(const Position& pos) const
{
	std::set<int32_t> set;
	const auto [i32格子Id, i32格子X, i32格子Z] = AoiComponent::格子(pos);

	int i32视野范围 = m_i32视野范围;
	if (m_refEntity.m_spAttack)
		i32视野范围 = std::max<int>((int)m_refEntity.m_spAttack->m_战斗配置.f警戒距离, i32视野范围);

	i32视野范围 /= u8格子正方形边长;
	++i32视野范围;//上面去掉小数了，这里加一点

	for (int x = i32格子X - i32视野范围; x <= i32格子X + i32视野范围; ++x)
	{
		for (int z = i32格子Z - i32视野范围; z <= i32格子Z + i32视野范围; ++z)
		{
			const auto id = 格子Id(x, z);
			const auto [iter, ok] = set.insert(id);
			_ASSERT(ok); //bool值则表示是否插入了该元素
		}
	}

	return set;
}
