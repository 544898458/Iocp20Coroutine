#include "StdAfx.h"
#include <glog/logging.h>
#include <cstdlib>
#include "MyMsgQueue.h"
#include "MySession.h"
#include "Space.h"
#include "../CoRoutine/CoTimer.h"
#include "MyServer.h"
#include "../IocpNetwork/MsgQueueTemplate.h"
#include "../IocpNetwork/StrConv.h"
#include "AiCo.h"
#include "Entity.h"
#include "PlayerComponent.h"

void MyMsgQueue::Process()
{
	MsgId msgId = this->m_MsgQueue.PopMsg();
	//{
	//	std::lock_guard lock(this->m_MsgQueue.m_mutex);
	//	if (this->m_MsgQueue.m_queueMsgId.empty())
	//		return;

	//	msgId = this->m_MsgQueue.m_queueMsgId.front();
	//	this->m_MsgQueue.m_queueMsgId.pop_front();
	//}
	switch (msgId)
	{
	case MsgId::Invalid_0://û����Ϣ�ɴ���
		return;
	case MsgId::Login:this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, OnRecv); break;
	case MsgId::Move:this->m_MsgQueue.OnRecv(this->m_queueMove, *this, OnRecv); break;
	case MsgId::Say:this->m_MsgQueue.OnRecv(this->m_queueSay, *this, OnRecv); break;
	case MsgId::SelectRoles:this->m_MsgQueue.OnRecv(this->m_queueSelectRoles, *this, OnRecv); break;
	default:
		LOG(ERROR) << "msgId:" << msgId;
		assert(false);
		break;
	}
}
template<> std::deque<MsgLogin>& MyMsgQueue::GetQueue() { return m_queueLogin; }
template<> std::deque<MsgMove>& MyMsgQueue::GetQueue() { return m_queueMove; }
template<> std::deque<MsgSay>& MyMsgQueue::GetQueue() { return m_queueSay; }
template<> std::deque<MsgSelectRoles>& MyMsgQueue::GetQueue() { return m_queueSelectRoles; }
template<class T>
void MyMsgQueue::Push(const T& msg) { m_MsgQueue.Push(msg, GetQueue<T>()); }
//void MyMsgQueue::Push(const MsgMove& msg) { m_MsgQueue.Push(msg, m_queueMove); }
//void MyMsgQueue::Push(const MsgSay& msg) { m_MsgQueue.Push(msg, m_queueSay); }

void MyMsgQueue::OnRecv(MyMsgQueue& refThis, const MsgLogin& msg)
{
	auto utf8Name = msg.name;
	auto gbkName = StrConv::Utf8ToGbk(msg.name);
	//printf("׼���㲥%s",utf8Name.c_str());
	/*for (auto p : g_set)
	{
		const auto strBroadcast = "[" + utf8Name + "]������";
		MsgLoginRet ret = { 223,GbkToUtf8(strBroadcast.c_str()) };
		p->Send(ret);
	}*/
	//const auto strBroadcast = "[" + utf8Name + "]������";
	if (msg.name.empty())
	{
		refThis.m_pSession->Send(MsgSay(StrConv::GbkToUtf8("����������")));
		return;
	}
	if (!refThis.m_pSession->m_vecSpEntity.empty())
	{
		refThis.m_pSession->Send(MsgSay(StrConv::GbkToUtf8("�����ظ���¼")));
		return;
	}

	auto spNewEntity = std::make_shared<Entity>();
	refThis.m_pSession->m_vecSpEntity.push_back(spNewEntity);
	spNewEntity->Init(5, refThis.m_pSession->m_pServer->m_space, "altman-blue");
	spNewEntity->AddComponent(refThis.m_pSession);
	spNewEntity->m_nickName = gbkName;
	refThis.m_pSession->m_pServer->m_space.setEntity.insert(spNewEntity);
	//for (const auto pENtity : refThis.m_pSession->m_pServer->m_space.setEntity)
	//{
	//	if (pENtity == &refThis.m_pSession->m_entity)
	//		continue;

	//	if (pENtity->m_nickName == utf8Name)
	//	{
	//		LOG(WARNING) << "�ظ���¼" << utf8Name;
	//		//�������߻�û��
	//		return;
	//	}
	//}

	{
		MsgLoginRet ret((uint64_t)spNewEntity.get(), utf8Name, spNewEntity->m_strPrefabName);
		spNewEntity->Broadcast(ret);//�Լ��㲥������
	}

	for (const auto &spEntity : refThis.m_pSession->m_pServer->m_space.setEntity)
	{
		if (!spEntity->m_spPlayer)
			continue;

		if (refThis.m_pSession == spEntity->m_spPlayer->m_pSession)
			continue;

		refThis.m_pSession->Send(MsgLoginRet((uint64_t)spEntity.get(), StrConv::GbkToUtf8(spEntity->m_nickName), spEntity->m_strPrefabName));//���˷����Լ�
		for (auto spEntity : spEntity->m_spPlayer->m_pSession->m_vecSpEntity)
		{
			spNewEntity->Broadcast(MsgNotifyPos(*spEntity));//���˷����Լ�
		}
	}
}

void MyMsgQueue::OnRecv(MyMsgQueue& refThis, const MsgMove& msg)
{
	LOG(INFO) << "�յ��������:" << msg.x << "," << msg.z;
	const auto targetX = msg.x;
	const auto targetZ = msg.z;
	auto pServer = refThis.m_pSession->m_pServer;
	//refThis.m_pSession->m_entity.WalkToPos(targetX, targetZ, pServer);
	for (const auto id : refThis.m_pSession->m_vecSelectedEntity)
	{
		Entity* pEntity = (Entity*)id;
		const auto &refVecSpEntity = refThis.m_pSession->m_vecSpEntity;
		if (refVecSpEntity.end() == std::find_if(refVecSpEntity.begin(), refVecSpEntity.end(), [pEntity](const auto& sp) {return sp.get() == pEntity; }))
		{
			LOG(ERROR) << id << "�����Լ��ĵ�λ�������ƶ�";
			continue;
		}

		pEntity->WalkToPos(Position(targetX, targetZ), pServer);

	}
}

void MyMsgQueue::OnRecv(MyMsgQueue& refThis, const MsgSay& msg)
{
	auto utf8Content = StrConv::Utf8ToGbk(msg.content);
	LOG(INFO) << "�յ�����:" << utf8Content;
	void SendToWorldSvr(const MsgSay & msg);
	SendToWorldSvr(msg);
}

void MyMsgQueue::OnRecv(MyMsgQueue& refThis, const MsgSelectRoles& msg)
{
	LOG(INFO) << "�յ�ѡ��:" << msg.ids.size();
	refThis.m_pSession->m_vecSelectedEntity.clear();
	std::transform(msg.ids.begin(), msg.ids.end(), std::back_inserter(refThis.m_pSession->m_vecSelectedEntity), [](const double& id) {return uint64_t(id); });
}

MsgNotifyPos::MsgNotifyPos(Entity& ref) : entityId((uint64_t)&ref), x(ref.m_Pos.x), z(ref.m_Pos.z), eulerAnglesY(ref.m_eulerAnglesY), hp(ref.m_hp)
{}

template void MyMsgQueue::Push(const MsgLogin& msg);
template void MyMsgQueue::Push(const MsgMove& msg);
template void MyMsgQueue::Push(const MsgSay& msg);
template void MyMsgQueue::Push(const MsgSelectRoles& msg);
template void MyMsgQueue::Push(const MsgLogin& msg);