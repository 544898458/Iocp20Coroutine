#pragma once
#include "SpEntity.h"
#include "../CoRoutine/CoTask.h"

class PlayerGateSession_Game;
enum ��Դ����;
class �ɼ�Component
{
public:
	�ɼ�Component();
	void �ɼ�(PlayerGateSession_Game&, Entity& refThis, WpEntity wp);
	CoTaskBool Co�ɼ�(PlayerGateSession_Game&, Entity& refThis, WpEntity wp);
	static void AddComponent(Entity& spEntity);
	CoTaskCancel m_TaskCancel;
private:
	static constexpr uint32_t MaxЯ����() { return 5; }
	uint32_t m_u32Я���� = 0;
	��Դ���� m_Я��������;
};

