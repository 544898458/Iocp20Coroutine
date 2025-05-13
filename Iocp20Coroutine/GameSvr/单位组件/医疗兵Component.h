#pragma once
#include "../../CoRoutine/CoTask.h"
#include "../MyMsgQueue.h"
#include "../单位.h"
#include "../SpEntity.h"

class 找目标走过去Component;

class 医疗兵Component
{
public:
	static void AddComponent(Entity& refEntity);
	医疗兵Component(Entity& refEntity);
	void OnEntityDestroy(const bool bDestroy);
private:
	bool 可以治疗();
	CoTask<std::tuple<bool, bool>> Co治疗(const Entity& refTarget, WpEntity wpEntity, 找目标走过去Component& ref找目标走过去);
	WpEntity Get最近的可治疗友方单位();


	CoTaskBool Co治疗目标(WpEntity wpDefencer, FunCancel& cancel);

	Entity& m_refEntity;
	FunCancel安全 m_cancel治疗;
};

