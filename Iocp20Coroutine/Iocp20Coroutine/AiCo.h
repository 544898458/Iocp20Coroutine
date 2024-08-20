#pragma once
#include "../CoRoutine/CoTask.h"
#include "SpEntity.h"
class GameSvr;
struct Position;
class Space;
namespace AiCo
{
	//Э�̲���ͬ��ִ�н���������Entityȫ��Ҫ��SpEntity�����������ã�ʵ��һ��ȥ���̼����ü���
	//������ͬ������������ֱ�Ӵ� Entity&
	//Э���е�ȫ�ֶ���Ҳ����ֱ�Ӵ����ã���������ָ��
	CoTask<int> Attack(SpEntity spEntity, SpEntity spDefencer, FunCancel& cancel);
	CoTask<int> Idle(SpEntity spEntity, FunCancel& funCancel);
	CoTask<int> WalkToPos(SpEntity spThis, const Position &posTarget, FunCancel& funCancel);
	CoTask<int> WalkToTarget(SpEntity spThis, SpEntity spEntity, GameSvr* pServer, FunCancel& funCancel);
	CoTask<int> WaitDelete(SpEntity spThis, FunCancel& funCancel);
	CoTask<int> SpawnMonster(Space& refSpace, FunCancel& funCancel);
	CoTask<int> AddMoney(SpEntity spThis, FunCancel& funCancel);
};

