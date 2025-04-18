#include "pch.h"
#include "定时改数值Component.h"
#include "DefenceComponent.h"
#include "Entity.h"
#include "../../CoRoutine/CoTimer.h"

定时改数值Component::定时改数值Component(Entity& ref) :m_refEntity(ref)
{
}

CoTaskBool 定时改数值Component::Co定时改数值(std::chrono::system_clock::duration dura间隔, int16_t i16变化)
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

定时改数值Component& 定时改数值Component::AddComponent(Entity& refEntity)
{
	refEntity.m_up定时改数值.reset(new 定时改数值Component(refEntity));
	return *refEntity.m_up定时改数值;
}

void 定时改数值Component::加属性(单位属性类型 属性类型, int16_t 变化, std::chrono::system_clock::duration dura删除)
{
	auto& refMap属性 = m_map属性数值[属性类型];
	auto [pair, ok] = refMap属性.insert({ ++m_sn属性数值, {变化} });
	CHECK_RET_VOID(ok);
	[&refMap属性, this, dura删除](FunCancel& fun)->CoTaskBool
		{
			auto sn = m_sn属性数值;
			auto& refMap属性Local = refMap属性;
			if (co_await CoTimer::Wait(dura删除, fun))
				co_return false;

			const auto sizeErase = refMap属性Local.erase(sn);
			CHECK_CO_RET_FALSE(sizeErase == 1);
		}(pair->second.funCancel).RunNew();
}

int16_t 定时改数值Component::属性(单位属性类型 属性) const
{
	const auto iterFind = m_map属性数值.find(属性);
    if (iterFind == m_map属性数值.end())
		return 0;

	int16_t 属性值 = 0;
	for (const auto& [_, 属性数值] : iterFind->second)
	{
		属性值 += 属性数值.变化;
	}
	return 属性值;
}

void 定时改数值Component::定时回血()
{
	using namespace std;
	Co定时改数值(2s, 1).RunNew();
}

void 定时改数值Component::OnDestroy()
{
	if (m_funCancel)
	{
		m_funCancel();
		m_funCancel = nullptr;
	}
}

