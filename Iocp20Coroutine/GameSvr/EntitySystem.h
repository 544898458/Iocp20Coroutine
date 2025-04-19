#pragma once

class Entity;

namespace EntitySystem
{
	class �ָ����ж��� final
	{
	public:
		�ָ����ж���(Entity& refEntity, const std::string& str����);
		~�ָ����ж���();
	private:
		Entity& m_refEntity;
	};
	void BroadcastEntity����(Entity& refEntity, const std::string& refStrGbk);
	void BroadcastChangeSkeleAnimIdle(Entity& refEntity);
	void BroadcastChangeSkeleAnim�ɼ�(Entity& refEntity);
	void Broadcast��������(Entity& refEntity, const std::string& refStr����, const std::string& str�ı� = "");
	bool Is�ӿ�(const Entity& refEntity);
	bool Is���(const Entity& refEntity);
	bool Is���λ������(const Entity& refEntity);
	bool �����ѷ���λ̫��(Entity& refEntity);
	const std::string GetNickName(const Entity& refEntity);
	bool Is����(const ��λ���� ����);
	bool Is����(const Entity& refEntity);
	bool Is�ɽ��ر�(const ��λ���� ����);
	bool Is��Դ(const ��λ���� ����);
	bool Is���λ(const ��λ���� ����);
	bool Is��λ����(const WpEntity& wp, const ��λ���� ����);
	bool Is�յ��ܴ�(const ��λ���� ��, const ��λ���� ��);
	bool Is��(const ��λ���� ����);
	bool Is�ӿ�(const ��λ���� ����);
	bool Is���(const ��λ���� ����);
	bool Is���λ������(const ��λ���� ����);
	template<typename T_��Ա>
	T_��Ա ����������(const Entity& refEntity, const �������� ����, T_��Ա ��λ::ս������::* p��Ա);
	uint16_t �����󹥻�(Entity& refEntity);
	uint16_t ���������(Entity& refEntity);
	uint16_t �����󹥻�ǰҡ_�˺���ʱ(Entity& refEntity);
	float �������ٶ�ÿ֡�ƶ�����(Entity& refEntity);
	float �������ٶ�ÿ���ƶ�����(Entity& refEntity);
	void ֹͣ����������(Entity& refEntity);
	void BroadcastEntity̦���뾶(Entity& refEntity);
};

