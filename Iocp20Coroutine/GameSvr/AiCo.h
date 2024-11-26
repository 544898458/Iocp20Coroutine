#pragma once
#include "../CoRoutine/CoTask.h"
#include "SpEntity.h"
struct Position;
class Space;
class GameSvrSession;
struct MsgChangeMoneyResponce;
class PlayerGateSession_Game;
namespace AiCo
{
	//������ͬ������������ֱ�Ӵ� Entity&
	//Э���е�ȫ�ֶ���Ҳ����ֱ�Ӵ����ã���������ָ��
	//�����Entity�Լ���Э�̣�����������Entity&��ǰ����ȷ��Entity����ǰֹͣ���Э��
	CoTask<int> Attack(SpEntity spEntity, WpEntity spDefencer, FunCancel& cancel);
	CoTask<int> Idle(SpEntity spEntity, FunCancel& funCancel);
	CoTaskBool WalkToPos(Entity& refThis, const Position &posTarget, FunCancel& funCancel, const float f����Ŀ��С�ڴ˾���ͣ�� = 0);
	CoTaskBool WalkToTarget(Entity &refThis, SpEntity spEntity, FunCancel& funCancel, const bool b��龯����� = true);
	CoTask<int> SpawnMonster(Space& refSpace, FunCancel& funCancel);
	CoTask<std::tuple<bool, MsgChangeMoneyResponce>> ChangeMoney(PlayerGateSession_Game& refSession, int32_t changeMoney, bool addMoney, FunCancel& funCancel);
	CoTask<int> AddMoney(PlayerGateSession_Game& refSession, FunCancel& funCancel);
};

