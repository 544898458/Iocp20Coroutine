#pragma once
#include "../MyMsgQueue.h"
class Entity;
class ����Ŀ��Component
{
public:
	static void AddComponet(Entity& refEntity, const Position& posĿ��);
	����Ŀ��Component(Entity& ref, const Position& posĿ��);
	void TryCancel();
private:
	CoTaskBool Co����Ŀ�����б�ը();
	Entity& m_refEntity;
	const Position m_posĿ��;
	FunCancel m_funCancel;
};

