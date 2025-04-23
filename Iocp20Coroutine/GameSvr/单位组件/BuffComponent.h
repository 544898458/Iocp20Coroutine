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
	void OnEntityDestroy(const bool bDestroy);

	void ������(BuffId idBuff��);
	float ����(�������� ��������)const;
	void ��ʱ����ֵ(const BuffId id, const uint64_t idAttacker);
	void ɾBuff(BuffId id);
	bool ����Buff(BuffId id)const;
private:
	void On���Ա仯(�������� ��������);
	CoTaskBool Co��ʱ����ֵ(const �������� ����, std::chrono::system_clock::duration dura���, int16_t i16�仯, FunCancel& funCancel, const uint64_t idAttacker);
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

