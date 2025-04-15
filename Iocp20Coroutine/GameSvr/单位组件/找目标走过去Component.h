#pragma once
#include "../../CoRoutine/CoTask.h"
#include "../MyMsgQueue.h"
#include "../��λ.h"

class Entity;

class ��Ŀ���߹�ȥComponent
{
public:
	static void AddComponent(Entity& refEntity);
	static Position �����й�(const Position& refOld);
	static void ����ǰҡ����(Entity& refEntity);
	static void ���Ź�������(Entity& refEntity);
	static void ���Ź�����Ч(Entity& refEntity);

	��Ŀ���߹�ȥComponent(Entity& refEntity);
	float ��������(const Entity& refTarget)const;
	float ��������(const float fĿ�꽨����߳�) const;

	void �����ѭ��(const std::function<bool()> fun���Բ���, const std::function<WpEntity()> fun�������Ŀ��, const std::function<CoTask<std::tuple<bool, bool>>(const Entity& refTarget, WpEntity wpEntity, ��Ŀ���߹�ȥComponent& ref��Ŀ���߹�ȥ)> fun���������Ŀ��, const std::function<void(WpEntity& wpEntity, bool& ref���Ŀ��)> fun������Ŀ��);

	
	��λ::ս������ m_ս������;
	bool m_b�����µ�Ŀ�� = true;
	bool m_bԭ�ؼ��� = false;//Hold Position
	using Fun��������Ŀ�� = std::function<Position(const Position&)>;
	Fun��������Ŀ�� m_fun��������˴�;
	void TryCancel(const bool bDestroy = false);
	bool ��鴩ǽ(const Entity& refEntity);

private:
	CoTaskBool Co����(
		const std::function<bool()> fun���Բ���,
		const std::function<WpEntity()> fun�������Ŀ��,
		const std::function<CoTask<std::tuple<bool, bool>>(const Entity& refTarget, WpEntity wpEntity, ��Ŀ���߹�ȥComponent& ref��Ŀ���߹�ȥ)> fun���������Ŀ��,
		const std::function<void(WpEntity& wpEntity, bool& ref���Ŀ��)> fun������Ŀ��);

	CoTaskBool Co���򾯽䷶Χ�ڵ�Ŀ��Ȼ�����(
		const std::function<bool()> fun���Բ���,
		const std::function<WpEntity()> fun�������Ŀ��,
		const std::function<CoTask<std::tuple<bool, bool>>(const Entity& refTarget, WpEntity wpEntity, ��Ŀ���߹�ȥComponent& ref��Ŀ���߹�ȥ)> fun���������Ŀ��,
		const std::function<void(WpEntity& wpEntity, bool& ref���Ŀ��)> fun������Ŀ��);

	Entity& m_refEntity;
	CoTaskCancel m_TaskCancel����;
	CoTaskCancel m_TaskCancel;

};

