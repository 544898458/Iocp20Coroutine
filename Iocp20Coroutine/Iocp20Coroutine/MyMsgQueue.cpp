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
#include "../CoRoutine/CoRpc.h"

//���̣߳����߳�
void MyMsgQueue::Process()
{
	const MsgId msgId = this->m_MsgQueue.PopMsg();
	switch (msgId)
	{
	case MsgId::Invalid_0://û����Ϣ�ɴ���
		return;
	case MsgId::Login:this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, OnRecv); break;
	case MsgId::Move:this->m_MsgQueue.OnRecv(this->m_queueMove, *this, OnRecv); break;
	case MsgId::Say:this->m_MsgQueue.OnRecv(this->m_queueSay, *this, OnRecv); break;
	case MsgId::SelectRoles:this->m_MsgQueue.OnRecv(this->m_queueSelectRoles, *this, OnRecv); break;
	case MsgId::AddRole:this->m_MsgQueue.OnRecv(this->m_queueAddRole, *this, OnRecv); break;
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
template<> std::deque<MsgAddRole>& MyMsgQueue::GetQueue() { return m_queueAddRole; }
template<class T> void MyMsgQueue::Push(const T& msg) { m_MsgQueue.Push(msg, GetQueue<T>()); }
template void MyMsgQueue::Push(const MsgLogin& msg);
template void MyMsgQueue::Push(const MsgMove& msg);
template void MyMsgQueue::Push(const MsgSay& msg);
template void MyMsgQueue::Push(const MsgSelectRoles& msg);
template void MyMsgQueue::Push(const MsgLogin& msg);
template void MyMsgQueue::Push(const MsgAddRole& msg);

void MyMsgQueue::OnRecv(MyMsgQueue& refThis, const MsgAddRole& msg)
{
	//ǰһ��û�����ͻ��ڴ�й©
	refThis.m_coRpc = refThis.CoAddRole();//����Э�̣���ȥWorldSvr��Ǯ���һ���µ�λ
	refThis.m_coRpc.Run();
}



CoTask<int> MyMsgQueue::CoAddRole()
{
	void SendToWorldSvr(const MsgConsumeMoney & msg);
	MsgConsumeMoneyResponce responce = co_await CoRpc<MsgConsumeMoneyResponce>::Send<MsgConsumeMoney>({ .consumeMoney = 3 }, SendToWorldSvr);//��ͬ����̵ķ�ʽ������һ���������������󲢵ȴ�����

	auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string& >({ 30,30 }, m_pSession->m_pServer->m_space, "altman-blue");
	spNewEntity->AddComponentPlayer(m_pSession);
	m_pSession->m_vecSpEntity.insert(spNewEntity);//�Լ����Ƶĵ�λ
	m_pSession->m_pServer->m_space.setEntity.insert(spNewEntity);//ȫ��ͼ��λ

	spNewEntity->BroadcastEnter();
	co_return 0;
}

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

	refThis.m_pSession->m_nickName = gbkName;

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


	for (const auto& spEntity : refThis.m_pSession->m_pServer->m_space.setEntity)//���˷����Լ�
	{
		refThis.m_pSession->Send(MsgAddRoleRet((uint64_t)spEntity.get(), StrConv::GbkToUtf8(spEntity->NickName()), spEntity->m_strPrefabName));
		refThis.m_pSession->Send(MsgNotifyPos(*spEntity));
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
		const auto& refVecSpEntity = refThis.m_pSession->m_vecSpEntity;
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