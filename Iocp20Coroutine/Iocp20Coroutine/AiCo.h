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
	//Э�̲���ͬ��ִ�н���������Entityȫ��Ҫ��SpEntity�����������ã�ʵ��һ��ȥ���̼����ü���
	//������ͬ������������ֱ�Ӵ� Entity&
	//Э���е�ȫ�ֶ���Ҳ����ֱ�Ӵ����ã���������ָ��
	//�����Entity�Լ���Э�̣�����������Entity&��ǰ����ȷ��Entity����ǰֹͣ���Э��
	CoTask<int> Attack(SpEntity spEntity, SpEntity spDefencer, FunCancel& cancel);
	CoTask<int> Idle(SpEntity spEntity, FunCancel& funCancel);
	CoTaskBool WalkToPos(SpEntity spThis, const Position &posTarget, FunCancel& funCancel);
	CoTaskBool WalkToTarget(SpEntity spThis, SpEntity spEntity, FunCancel& funCancel);
	CoTask<int> WaitDelete(SpEntity spThis, FunCancel& funCancel);
	CoTask<int> SpawnMonster(Space& refSpace, FunCancel& funCancel);
	CoTask<std::tuple<bool, MsgChangeMoneyResponce>> ChangeMoney(PlayerGateSession_Game& refSession, uint32_t changeMoney, bool addMoney, FunCancel& funCancel);
	CoTask<int> AddMoney(PlayerGateSession_Game& refSession, FunCancel& funCancel);
};

