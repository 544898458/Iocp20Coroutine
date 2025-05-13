#pragma once
#include "../../CoRoutine/CoTask.h"
#include "../MyMsgQueue.h"
#include "../��λ.h"
#include "../SpEntity.h"

class ��Ŀ���߹�ȥComponent;

class ҽ�Ʊ�Component
{
public:
	static void AddComponent(Entity& refEntity);
	ҽ�Ʊ�Component(Entity& refEntity);
	void OnEntityDestroy(const bool bDestroy);
private:
	bool ��������();
	CoTask<std::tuple<bool, bool>> Co����(const Entity& refTarget, WpEntity wpEntity, ��Ŀ���߹�ȥComponent& ref��Ŀ���߹�ȥ);
	WpEntity Get����Ŀ������ѷ���λ();


	CoTaskBool Co����Ŀ��(WpEntity wpDefencer, FunCancel& cancel);

	Entity& m_refEntity;
	FunCancel��ȫ m_cancel����;
};

