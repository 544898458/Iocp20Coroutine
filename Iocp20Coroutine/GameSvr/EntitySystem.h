#pragma once

class Entity;

namespace EntitySystem
{
	void BroadcastEntity����(Entity& refEntity, const std::string& refStrGbk);
	void BroadcastChangeSkeleAnimIdle(Entity& refEntity);
	void BroadcastChangeSkeleAnim�ɼ�(Entity& refEntity);
	void Broadcast��������(Entity& refEntity, const std::string& refStr����, const std::string& str�ı� = "");
	bool Is�ӿ�(const Entity& refEntity);
	bool �����ѷ���λ̫��(Entity& refEntity);
	const std::string GetNickName(Entity& refEntity);
	bool Is����(const ��λ���� ����);
	bool Is�ɽ��ر�(const ��λ���� ����);
	bool Is��Դ(const ��λ���� ����);
	bool Is���λ(const ��λ���� ����);
	bool Is��λ����(const WpEntity& wp, const ��λ���� ����);
	bool Is�յ��ܴ�(const ��λ���� ��, const ��λ���� ��);
};

