#pragma once
#include "SpEntity.h"
#include "../CoRoutine/CoTask.h"

class PlayerGateSession_Game;
enum ��λ����;
class �ɼ�Component
{
public:
	�ɼ�Component(Entity&);
	void �ɼ�(PlayerGateSession_Game&, WpEntity wp);
	CoTaskBool Co�ɼ�(WpEntity wp);
	static void AddComponent(Entity& spEntity);
	CoTaskCancel m_TaskCancel;
private:
	static constexpr uint32_t MaxЯ����() { return 5; }
	uint32_t m_u32Я���� = 0;
	��λ���� m_Я��������;
	Entity& m_refEntity;
};

