#pragma once
#include "../MyMsgQueue.h"
class Entity;
class ����Ŀ��Component
{
public:
	static void AddComponent(Entity& refEntity, const Position& pos��ʼ��, const Position& pos����, const float f��Զ����, const uint64_t idAttacker);
	����Ŀ��Component(Entity& ref, const Position& pos��ʼ��, const Position& pos����, const float f��Զ����, const uint64_t idAttacker);
	void OnEntityDestroy(const bool bDestroy);
private:
	CoTaskBool Co����Ŀ�����б�ը();
	Entity& m_refEntity;
	const Position m_vec����;
	const Position m_pos��ʼ��;
	const float m_f��Զ����;
	FunCancel��ȫ m_funCancel;
	const uint64_t m_idAttacker;
};

