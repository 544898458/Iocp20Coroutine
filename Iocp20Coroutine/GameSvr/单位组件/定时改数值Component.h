#pragma once
#include "../../CoRoutine/CoTask.h"
#include <map>
class Entity;
class PlayerGateSession_Game;
class Space;
enum ��λ��������;
class ��ʱ����ֵComponent
{
public:
	��ʱ����ֵComponent(Entity& ref);
	static ��ʱ����ֵComponent& AddComponent(Entity& refEntity);
	void ������(��λ�������� ��������, int16_t �仯, std::chrono::system_clock::duration duraɾ��);
	int16_t ����(��λ�������� ��������)const;
	void ��ʱ��Ѫ();
	void OnDestroy();
private:
	CoTaskBool Co��ʱ����ֵ(std::chrono::system_clock::duration dura���, int16_t i16�仯);
	Entity& m_refEntity;
	FunCancel m_funCancel;
	struct ������ֵ
	{
		int16_t �仯;
		FunCancel funCancel;
	};
	std::map<��λ��������, std::map<uint32_t, ������ֵ>> m_map������ֵ;
	uint32_t m_sn������ֵ = 0;
};

