#pragma once
#include "SpEntity.h"

class Space;
class PlayerGateSession_Game;
namespace MyEvent
{
	struct AddEntity
	{
		WpEntity wpEntity;
	};
	struct MoveEntity
	{
		WpEntity wpEntity;
	};
	struct ��ʼ�ɼ������
	{
	};
	struct ��������˻ػ���
	{
	};
	struct ��λ����
	{
		WpEntity wpEntity;
	};
	struct ��ҽ���Space
	{
		std::weak_ptr<PlayerGateSession_Game> wpPlayerGateSession;
		std::weak_ptr<Entity> wp�ӿ�;
		std::weak_ptr<Space> wpSpace;
	};
	
	struct ���Ķ�����Ի�
	{
		std::weak_ptr<PlayerGateSession_Game> wpPlayerGateSession;
	};

	struct �����λ
	{
		WpEntity wpOwner;
	};

	struct ����
	{
		WpEntity wpҽ�Ʊ�;
	};

	bool SameSpace(const WpEntity& refWpEntity, const Space& refSpace, const std::string& stdNickName);
}

