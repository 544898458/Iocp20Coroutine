#pragma once
#include "../MyMsgQueue.h"
class Entity;
class ����Ŀ��Component
{
public:
	static void AddComponet(Entity& refEntity, const Position& posĿ��);
private:
	����Ŀ��Component(Entity& ref,const Position& posĿ��);
	CoTaskBool Co����Ŀ�����б�ը();
	Entity& m_refEntity;
	const Position m_posĿ��;
	CoTaskCancel m_TaskCancel;
};

