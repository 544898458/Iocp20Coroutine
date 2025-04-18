#pragma once
#include "../../CoRoutine/CoTask.h"
#include <map>
class Entity;
class PlayerGateSession_Game;
class Space;
enum ��λ��������;
class BuffComponent
{
public:
	BuffComponent(Entity& ref);
	static BuffComponent& AddComponent(Entity& refEntity);
	void ������(uint32_t u32Buff��Id, ��λ�������� ��������, float f�仯, std::chrono::system_clock::duration duraɾ��);
	float ����(��λ�������� ��������)const;
	void ��ʱ��Ѫ();
	void OnDestroy();
private:
	void On���Ա仯(��λ�������� ��������);
	CoTaskBool Co��ʱ����ֵ(std::chrono::system_clock::duration dura���, int16_t i16�仯);
	Entity& m_refEntity;
	FunCancel m_funCancel;
	struct ������ֵ final
	{
		~������ֵ()
		{
			if (funCancel)
			{
				funCancel();
				funCancel = nullptr;
			}
		}
		float �仯;
		FunCancel funCancel;
	};
	std::map<��λ��������, std::map<uint32_t, ������ֵ>> m_map������ֵ;
};

