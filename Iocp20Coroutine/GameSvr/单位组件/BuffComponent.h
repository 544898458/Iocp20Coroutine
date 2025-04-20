#pragma once
#include "../../CoRoutine/CoTask.h"
#include <map>
class Entity;
class PlayerGateSession_Game;
class Space;
enum ��������;
enum BuffId;
class BuffComponent
{
public:
	BuffComponent(Entity& ref);
	static BuffComponent& AddComponent(Entity& refEntity);
	void OnDestroy();

	void ������(BuffId idBuff��);
	float ����(�������� ��������)const;
	void ��ʱ����ֵ(const BuffId id);
	void ɾBuff(BuffId id);
private:
	void On���Ա仯(�������� ��������);
	CoTaskBool Co��ʱ����ֵ(std::chrono::system_clock::duration dura���, int16_t i16�仯, FunCancel& funCancel);
	Entity& m_refEntity;
	std::map<BuffId, FunCancel> m_mapFunCancel;
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
	std::map<��������, std::map<BuffId, ������ֵ>> m_map������ֵ;
};

