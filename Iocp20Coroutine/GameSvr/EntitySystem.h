#pragma once

class Entity;

namespace EntitySystem
{
	void BroadcastEntity����(Entity& refEntity, const std::string& refStrGbk);
	void BroadcastChangeSkeleAnimIdle(Entity& refEntity);
	void Broadcast��������(Entity& refEntity, const std::string& refStr����, const std::string& str�ı� = "");
	bool Is�ӿ�(const Entity& refEntity);
	bool �����ѷ���λ̫��(Entity& refEntity);
};

