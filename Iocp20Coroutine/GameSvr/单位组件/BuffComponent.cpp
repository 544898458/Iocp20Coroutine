#include "pch.h"
#include "../枚举/BuffId.h"
#include "../枚举/属性类型.h"
#include "BuffComponent.h"
#include "DefenceComponent.h"
#include "走Component.h"
#include "Entity.h"
#include "数值Component.h"
#include "../../CoRoutine/CoTimer.h"
#include "../RecastNavigationCrowd.h"
#include "../Space.h"
#include <algorithm>

BuffComponent::BuffComponent(Entity& ref) :m_refEntity(ref), m_funCancel定时清理已结束的Buff("m_funCancel定时清理已结束的Buff")
{
	Co定时清理已结束的Buff().RunNew();
}

CoTaskBool BuffComponent::Co定时清理已结束的Buff()
{
	using namespace std::chrono_literals;
	while (!co_await CoTimer::Wait(1s, m_funCancel定时清理已结束的Buff))
	{
		bool bChanged = false;
		for (auto& [id, listFunCancel] : m_mapFunCancel)
		{
			for (auto iterList = listFunCancel.begin(); iterList != listFunCancel.end(); )
			{
				if (!*iterList)
				{
					iterList = listFunCancel.erase(iterList);
					bChanged = true;
					continue;
				}
				++iterList;
			}
		}

		if (bChanged)
		{
			广播Buff列表();
		}
	}

	co_return false;
}

CoTaskBool BuffComponent::Co定时改数值(const 属性类型 属性, std::chrono::system_clock::duration dura间隔, int16_t i16变化, FunCancel& funCancel, WpEntity wpAttacker, std::chrono::system_clock::duration dura持续)
{
	const auto 开始时刻 = std::chrono::system_clock::now();
	do
	{
		CHECK_CO_RET_FALSE(m_refEntity.m_upDefence);
		if (m_refEntity.IsDead())
			co_return false;

		if (属性 == 属性类型::生命 && 0 > i16变化)
			m_refEntity.m_upDefence->受伤(-i16变化, wpAttacker);
		else
			数值Component::改变(m_refEntity, 属性, i16变化);
	}
	while (开始时刻 + dura持续 > std::chrono::system_clock::now() && !co_await CoTimer::Wait(dura间隔, funCancel));

	co_return false;
}

BuffComponent& BuffComponent::AddComponent(Entity& refEntity)
{
	refEntity.AddComponentOnDestroy(&Entity::m_upBuff, refEntity);
	return *refEntity.m_upBuff;
}

void BuffComponent::加属性(BuffId idBuff表)
{
	单位::Buff配置 buff配置;
	CHECK_RET_VOID(单位::FindBuff配置(idBuff表, 1, buff配置));
	auto& refMap属性 = m_map属性数值[buff配置.属性];
	auto iterOld属性 = refMap属性.find(idBuff表);
	float fOld变化 = 0;
	if (iterOld属性 != refMap属性.end())
	{
		fOld变化 = iterOld属性->second.变化;
		refMap属性.erase(iterOld属性);
	}
	auto [pair, ok] = refMap属性.insert({ idBuff表, 属性数值(buff配置.f变化值) });
	CHECK_RET_VOID(ok);
	[buff配置, idBuff表, &refMap属性, this](FunCancel& fun)->CoTaskBool
		{
			const auto local属性类型 = buff配置.属性;
			const auto idBuff表Local = idBuff表;
			auto& refEntity = *this;
			auto& refMap属性Local = refMap属性;
			if (co_await CoTimer::Wait(buff配置.dura间隔, fun))
				co_return true;

			const auto sizeErase = refMap属性Local.erase(idBuff表Local);//销毁整个协程，下面代码只能调用局部变量
			CHECK_CO_RET_FALSE(sizeErase == 1);
			refEntity.On属性变化(local属性类型);
			co_return true;
		}(pair->second.m_funCancel).RunNew();

	if (buff配置.f变化值 != fOld变化)
	{
		On属性变化(buff配置.属性);
	}
}

void BuffComponent::On属性变化(属性类型 属性类型)
{
	if (属性类型 == 属性类型::移动速度 && m_refEntity.m_up走)
	{
		if (!m_refEntity.m_up走->m_wpRecastNavigationCrowd.expired())
			m_refEntity.m_up走->m_wpRecastNavigationCrowd.lock()->SetSpeed();
	}
}
float BuffComponent::属性(属性类型 属性) const
{
	const auto iterFind = m_map属性数值.find(属性);
	if (iterFind == m_map属性数值.end())
		return 0;

	float 属性值 = 0;
	for (const auto& [_, 属性数值] : iterFind->second)
	{
		属性值 += 属性数值.变化;
	}
	return 属性值;
}

void BuffComponent::定时改数值(const BuffId id, const uint64_t idAttacker)
{
	单位::Buff配置 buff配置;
	CHECK_RET_VOID(单位::FindBuff配置(id, buff配置));
	删Buff(id);
	auto& refList = m_mapFunCancel[id];
	refList.push_back(FunCancel());
	Co定时改数值(buff配置.属性, buff配置.dura间隔, (int16_t)buff配置.f变化值, refList.back(), idAttacker).RunNew();
}

CoTaskBool BuffComponent::Co定时改数值(const 属性类型 属性, std::chrono::system_clock::duration dura间隔, int16_t i16变化, FunCancel& refFunCancel, const uint64_t idAttacker)
{
	while (!co_await CoTimer::Wait(dura间隔, refFunCancel))
	{
		CHECK_CO_RET_FALSE(m_refEntity.m_upDefence);
		if (m_refEntity.IsDead())
			co_return false;

		if (属性 == 生命 && 0 > i16变化)
			m_refEntity.m_upDefence->受伤(-i16变化, m_refEntity.m_refSpace.GetEntity(idAttacker));
		else
			数值Component::改变(m_refEntity, 属性, i16变化);
	}
	co_return false;
}

void BuffComponent::定时改数值(const BuffId id, WpEntity wpAttacker, const uint16_t u16等级)
{
	using namespace std;
	单位::Buff配置 buff配置;
	CHECK_RET_VOID(单位::FindBuff配置(id, u16等级, buff配置));
	
	if (! buff配置.b可叠加)
	{
		删Buff(id);
	}
	
	auto& listFunCancel = m_mapFunCancel[id];
	// 先查找是否有空闲位置（已结束的协程）
	auto iter空闲 = std::find_if(listFunCancel.begin(), listFunCancel.end(), 
		[](const FunCancel& funCancel) { return !funCancel; });
	
	FunCancel* pFunCancel = nullptr;
	if (iter空闲 != listFunCancel.end())
	{
		// 复用空闲位置，不需要创建新的
		pFunCancel = &(*iter空闲);
	}
	else
	{
		// 没有空闲位置，才创建新的空FunCancel
		listFunCancel.push_back(FunCancel());
		pFunCancel = &listFunCancel.back();
	}
	
	CHECK_RET_VOID(pFunCancel);
	
	// 现在运行协程，协程会设置已经在list中的funCancel
	Co定时改数值(buff配置.属性, buff配置.dura间隔, (int16_t)buff配置.f变化值, *pFunCancel, wpAttacker, buff配置.dura持续).RunNew();

	广播Buff列表();
}

void BuffComponent::广播Buff列表()
{
	MsgBuff列表 msgBuff列表;
	for (const auto& [id, listFunCancel] : m_mapFunCancel)
	{
		auto iter = std::find_if(listFunCancel.begin(), listFunCancel.end(), [](const FunCancel& funCancel)->bool { return (bool)funCancel; });
		if (iter == listFunCancel.end())
			continue;
		msgBuff列表.setBuff.insert(id);
	}
	msgBuff列表.idEntity = m_refEntity.Id;
	m_refEntity.Broadcast(msgBuff列表);
}

void BuffComponent::删Buff(BuffId id)
{
	auto iterFind = m_mapFunCancel.find(id);
	if (iterFind != m_mapFunCancel.end())
	{
		// 取消所有该BuffId的协程
		for (auto& funCancel : iterFind->second)
		{
			if (funCancel)
			{
				funCancel();
				funCancel = nullptr;
			}
		}
		// 清理空闲位置
		清理空闲Buff(id);
		// 如果全部清空，删除整个条目
		if (iterFind->second.empty())
		{
			m_mapFunCancel.erase(iterFind);
		}
	}
}

bool BuffComponent::已有Buff(BuffId id) const
{
	auto iterFind = m_mapFunCancel.find(id);
	if (iterFind == m_mapFunCancel.end())
		return false;
	// 检查是否有活跃的Buff（非空的FunCancel）
	for (const auto& funCancel : iterFind->second)
	{
		if (funCancel)
			return true;
	}
	return false;
}

void BuffComponent::清理空闲Buff(BuffId id)
{
	auto iterFind = m_mapFunCancel.find(id);
	if (iterFind != m_mapFunCancel.end())
	{
		// 移除所有已结束的FunCancel（为nullptr的）
		auto& listFunCancel = iterFind->second;
		listFunCancel.remove_if([](const FunCancel& funCancel) { return !funCancel; });
	}
}

void BuffComponent::OnEntityDestroy(const bool /*bDestroy*/)
{
	for (auto& [id, listFunCancel] : m_mapFunCancel)
	{
		// 取消所有该BuffId的协程
		for (auto& funCancel : listFunCancel)
		{
			if (funCancel)
			{
				funCancel();
				funCancel = nullptr;
			}
		}
	}
	m_mapFunCancel.clear();

	m_map属性数值.clear();

	m_funCancel定时清理已结束的Buff.TryCancel();
}
