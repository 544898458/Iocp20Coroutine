#pragma once
#include "SpEntity.h"
#include "../CoRoutine/CoTask.h"

class PlayerGateSession_Game;
class ��ԴComponent;

enum ��λ����;
class �ɼ�Component
{
public:
	�ɼ�Component(Entity&);
	void �ɼ�(WpEntity wp);
	WpEntity GetĿ����Դ(WpEntity& refWpĿ����Դ);
	CoTaskBool Co�ɼ�(WpEntity wp);
	static void AddComponent(Entity& spEntity);
	static bool ���ڲɼ�(Entity& refEntity);
	void OnEntityDestroy(const bool bDestroy);
	CoTaskCancel m_TaskCancel;
private:
	static constexpr uint32_t MaxЯ����() { return 5; }
	void ������Դ�ݽ�();
	uint32_t m_u32Я���� = 0;
	��λ���� m_Я��������;
	��λ���� m_Ŀ����Դ����;
	Entity& m_refEntity;
};

