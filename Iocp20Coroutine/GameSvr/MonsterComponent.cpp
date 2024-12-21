#include "pch.h"
#include "MonsterComponent.h"
#include "Entity.h"
#include "AiCo.h"
#include "AttackComponent.h"
#include "DefenceComponent.h"
#include "Space.h"
#include "◊ﬂComponent.h"
#include "µ•Œª.h"

void MonsterComponent::AddComponent(Entity& refEntity)
{
	refEntity.m_spMonster = std::make_shared<MonsterComponent, Entity&>(refEntity);
}

MonsterComponent::MonsterComponent(Entity& refThis)
{
	if (!refThis.m_spAttack)
	{
		LOG(ERROR) << "m_spAttack";
		return;
	}
	//m_coIdle = AiCo::Idle(refThis.shared_from_this(), refThis.m_spAttack->m_cancel);
	//m_coIdle.Run();
}


std::vector<SpEntity> MonsterComponent::AddMonster(Space& refSpace, const ªÓ∂Øµ•Œª¿‡–Õ ¿‡–Õ, const Position& refPos, const int count)
{
	std::vector<SpEntity> vecRet;
	µ•Œª::π÷≈‰÷√ ≈‰÷√;
	µ•Œª::Findπ÷≈‰÷√(¿‡–Õ, ≈‰÷√);
	for (int i = 0; i < count; ++i)
	{
		SpEntity spEntityMonster = std::make_shared<Entity, const Position&, Space&, const std::string&, const std::string& >(
			refPos, refSpace, ≈‰÷√.≈‰÷√.strPrefabName, ≈‰÷√.≈‰÷√.strName);
		AttackComponent::AddComponent(*spEntityMonster, ¿‡–Õ, ≈‰÷√.’Ω∂∑.fπ•ª˜æ‡¿Î, ≈‰÷√.’Ω∂∑.f…À∫¶, ≈‰÷√.’Ω∂∑.fæØΩ‰æ‡¿Î, ≈‰÷√.’Ω∂∑.dura∆’π•«∞“°);
		DefenceComponent::AddComponent(*spEntityMonster, ≈‰÷√.u16≥ı ºHp);
		◊ﬂComponent::AddComponent(*spEntityMonster);
		AddComponent(*spEntityMonster);
		//spEntityMonster->m_fæØΩ‰æ‡¿Î = ≈‰÷√.’Ω∂∑.fæØΩ‰æ‡¿Î;
		spEntityMonster->m_ÀŸ∂»√ø÷°“∆∂Øæ‡¿Î = ≈‰÷√.’Ω∂∑.f√ø÷°“∆∂Øæ‡¿Î;
		//refSpace.m_mapEntity.insert({ (int64_t)spEntityMonster.get() ,spEntityMonster });
		refSpace.AddEntity(spEntityMonster);
		//LOG(INFO) << "SpawnMonster:" << refSpace.m_mapEntity.size();
		spEntityMonster->BroadcastEnter();
		vecRet.emplace_back(spEntityMonster);
	}
	return vecRet;
}