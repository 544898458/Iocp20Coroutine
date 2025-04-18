#include "pch.h"
#include "BuffComponent.h"
#include "DefenceComponent.h"
#include "走Component.h"
#include "Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../RecastNavigationCrowd.h"

BuffComponent::BuffComponent(Entity& ref) :m_refEntity(ref)
{
}

CoTaskBool BuffComponent::Co定时改数值(std::chrono::system_clock::duration dura间隔, int16_t i16变化)
{
	while (!co_await CoTimer::Wait(dura间隔, m_funCancel))
	{
		CHECK_CO_RET_FALSE(m_refEntity.m_spDefence);
		if (m_refEntity.IsDead())
			co_return false;

		if (m_refEntity.m_spDefence->已满血())
			continue;

		m_refEntity.m_spDefence->m_hp += i16变化;
		m_refEntity.BroadcastNotifyPos();
	}
	co_return false;
}

BuffComponent& BuffComponent::AddComponent(Entity& refEntity)
{
	refEntity.m_upBuff.reset(new BuffComponent(refEntity));
	return *refEntity.m_upBuff;
}

void BuffComponent::加属性(uint32_t u32Buff表Id, 单位属性类型 属性类型, float f变化, std::chrono::system_clock::duration dura删除)
{
	auto& refMap属性 = m_map属性数值[属性类型];
	auto iterOld属性 = refMap属性.find(u32Buff表Id);
	float fOld变化 = 0;
	if (iterOld属性 != refMap属性.end())
	{
		fOld变化 = iterOld属性->second.变化;
		refMap属性.erase(iterOld属性);
	}
	auto [pair, ok] = refMap属性.insert({ u32Buff表Id, {f变化} });
	CHECK_RET_VOID(ok);
	[属性类型, u32Buff表Id, &refMap属性, this, dura删除](FunCancel& fun)->CoTaskBool
		{
			const auto local属性类型 = 属性类型;
			const auto u32Buff表IdLocal = u32Buff表Id;
			auto& refEntity = *this;
			auto& refMap属性Local = refMap属性;
			if (co_await CoTimer::Wait(dura删除, fun))
				co_return true;

			const auto sizeErase = refMap属性Local.erase(u32Buff表IdLocal);//销毁整个协程，下面代码只能调用局部变量
			CHECK_CO_RET_FALSE(sizeErase == 1);
			refEntity.On属性变化(local属性类型);
			co_return true;
		}(pair->second.funCancel).RunNew();

		if (f变化 != fOld变化)
		{
			On属性变化(属性类型);
		}
}

void BuffComponent::On属性变化(单位属性类型 属性类型)
{
	if (属性类型 == 单位属性类型::移动速度)
	{
		CHECK_RET_VOID(m_refEntity.m_sp走);
		if (!m_refEntity.m_sp走->m_wpRecastNavigationCrowd.expired())
			m_refEntity.m_sp走->m_wpRecastNavigationCrowd.lock()->SetSpeed();
	}
}
float BuffComponent::属性(单位属性类型 属性) const
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

void BuffComponent::定时回血()
{
	using namespace std;
	Co定时改数值(2s, 1).RunNew();
}

void BuffComponent::OnDestroy()
{
	if (m_funCancel)
	{
		m_funCancel();
		m_funCancel = nullptr;
	}
}

