#include "pch.h"
#include "../枚举/BuffId.h"
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

		m_refEntity.m_spDefence->加血(i16变化);
		m_refEntity.BroadcastNotifyPos();
	}
	co_return false;
}

BuffComponent& BuffComponent::AddComponent(Entity& refEntity)
{
	refEntity.m_upBuff.reset(new BuffComponent(refEntity));
	return *refEntity.m_upBuff;
}

void BuffComponent::加属性(BuffId idBuff表, 属性类型 属性类型, float f变化, std::chrono::system_clock::duration dura删除)
{
	auto& refMap属性 = m_map属性数值[属性类型];
	auto iterOld属性 = refMap属性.find(idBuff表);
	float fOld变化 = 0;
	if (iterOld属性 != refMap属性.end())
	{
		fOld变化 = iterOld属性->second.变化;
		refMap属性.erase(iterOld属性);
	}
	auto [pair, ok] = refMap属性.insert({ idBuff表, {f变化} });
	CHECK_RET_VOID(ok);
	[属性类型, idBuff表, &refMap属性, this, dura删除](FunCancel& fun)->CoTaskBool
		{
			const auto local属性类型 = 属性类型;
			const auto idBuff表Local = idBuff表;
			auto& refEntity = *this;
			auto& refMap属性Local = refMap属性;
			if (co_await CoTimer::Wait(dura删除, fun))
				co_return true;

			const auto sizeErase = refMap属性Local.erase(idBuff表Local);//销毁整个协程，下面代码只能调用局部变量
			CHECK_CO_RET_FALSE(sizeErase == 1);
			refEntity.On属性变化(local属性类型);
			co_return true;
		}(pair->second.funCancel).RunNew();

		if (f变化 != fOld变化)
		{
			On属性变化(属性类型);
		}
}

void BuffComponent::On属性变化(属性类型 属性类型)
{
	if (属性类型 == 属性类型::移动速度 && m_refEntity.m_sp走)
	{
		if (!m_refEntity.m_sp走->m_wpRecastNavigationCrowd.expired())
			m_refEntity.m_sp走->m_wpRecastNavigationCrowd.lock()->SetSpeed();
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

void BuffComponent::定时回血()
{
	using namespace std;
	Co定时改数值(2s, 1).RunNew();
}

void BuffComponent::定时改数值(const BuffId id)
{
	using namespace std;
    单位::Buff配置 buff配置;
	CHECK_RET_VOID(单位::FindBuff配置(id, buff配置));
	Co定时改数值(2s, 1).RunNew();
}

void BuffComponent::OnDestroy()
{
	if (m_funCancel)
	{
		m_funCancel();
		m_funCancel = nullptr;
	}

	m_map属性数值.clear();
}

