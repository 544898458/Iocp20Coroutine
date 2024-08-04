#pragma once
#include "../CoRoutine/CoTask.h"
#include "SpEntity.h"
class MyServer;
struct Position;
class Space;
namespace AiCo
{
	//Э�̲���ͬ��ִ�н���������Entityȫ��Ҫ��SpEntity�����������ã�ʵ��һ��ȥ���̼����ü���
	//������ͬ������������ֱ�Ӵ� Entity&
	//Э���е�ȫ�ֶ���Ҳ����ֱ�Ӵ����ã���������ָ��
	CoTask<int> Attack(SpEntity spEntity, SpEntity spDefencer, std::function<void()>& cancel);
	CoTask<int> Idle(SpEntity spEntity, std::function<void()>& funCancel);
	CoTask<int> WalkToPos(SpEntity spThis, const Position &posTarget, MyServer *pServer, std::function<void()>& funCancel);
	CoTask<int> WalkToTarget(SpEntity spThis, SpEntity spEntity, MyServer* pServer, std::function<void()>& funCancel);
	CoTask<int> WaitDelete(SpEntity spThis, std::function<void()>& funCancel);
	CoTask<int> SpawnMonster(Space& refSpace, FunCancel& funCancel);
};

