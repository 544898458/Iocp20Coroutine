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
	case MsgId::Invalid_0://没有消息可处理
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
template<> std::deque<MsgLogin>& MyMsgQueue::GetQueue(){return m_queueLogin;}
template<> std::deque<MsgMove>& MyMsgQueue::GetQueue() { return m_queueMove; }
template<> std::deque<MsgSay>& MyMsgQueue::GetQueue() { return m_queueSay; }
template<> std::deque<MsgSelectRoles>& MyMsgQueue::GetQueue() { return m_queueSelectRoles; }
template<class T>
void MyMsgQueue::Push(const T& msg) { m_MsgQueue.Push(msg, GetQueue<T>()); }
//void MyMsgQueue::Push(const MsgMove& msg) { m_MsgQueue.Push(msg, m_queueMove); }
//void MyMsgQueue::Push(const MsgSay& msg) { m_MsgQueue.Push(msg, m_queueSay); }

void MyMsgQueue::OnRecv(MyMsgQueue& refThis, const MsgLogin& msg)
{
	auto utf8Name = StrConv::Utf8ToGbk(msg.name);
	//printf("准备广播%s",utf8Name.c_str());
	/*for (auto p : g_set)
	{
		const auto strBroadcast = "[" + utf8Name + "]进来了";
		MsgLoginRet ret = { 223,GbkToUtf8(strBroadcast.c_str()) };
		p->Send(ret);
	}*/
	//const auto strBroadcast = "[" + utf8Name + "]进来了";
	if (msg.name.empty())
	{
		refThis.m_pSession->Send(MsgSay(StrConv::GbkToUtf8("请输入名字")));
		return;
	}
	if (!refThis.m_pSession->m_entity.m_nickName.empty())
	{
		refThis.m_pSession->Send(MsgSay(StrConv::GbkToUtf8("不能重复登录")));
		return;
	}
	for (const auto pENtity : refThis.m_pSession->m_pServer->m_space.setEntity)
	{
		if (pENtity == &refThis.m_pSession->m_entity)
			continue;

		if (pENtity->m_nickName == utf8Name)
		{
			LOG(WARNING) << "重复登录" << utf8Name;
			//主动断线还没做
			return;
		}
	}

	{
		MsgLoginRet ret((uint64_t)&refThis.m_pSession->m_entity, StrConv::GbkToUtf8(utf8Name), refThis.m_pSession->m_entity.m_strPrefabName);
		refThis.m_pSession->m_entity.Broadcast(ret);//自己广播给别人
	}

	for (const auto pEntity : refThis.m_pSession->m_pServer->m_space.setEntity)
	{
		if (pEntity == &refThis.m_pSession->m_entity)
			continue;

		refThis.m_pSession->Send(MsgLoginRet((uint64_t)pEntity, StrConv::GbkToUtf8(pEntity->m_nickName), pEntity->m_strPrefabName));//别人发给自己
		refThis.m_pSession->m_entity.Broadcast(MsgNotifyPos(&refThis.m_pSession->m_entity));//别人发给自己
	}
}

void MyMsgQueue::OnRecv(MyMsgQueue& refThis, const MsgMove& msg)
{
	LOG(INFO) << "收到点击坐标:" << msg.x << "," << msg.z;
	const auto targetX = msg.x;
	const auto targetZ = msg.z;
	auto pServer = refThis.m_pSession->m_pServer;
	//refThis.m_pSession->m_entity.WalkToPos(targetX, targetZ, pServer);
	for (const auto id : refThis.m_pSession->m_vecSelectedEntities) 
	{
		Entity* pEntity = (Entity*)id;
		if (refThis.m_pSession->m_entity.m_space->setEntity.end() == refThis.m_pSession->m_entity.m_space->setEntity.find(pEntity))
		{
			LOG(ERROR) << id << "已离开，不能移动";
			continue;
		}

		pEntity->WalkToPos(targetX, targetZ, pServer);

	}
}

void MyMsgQueue::OnRecv(MyMsgQueue& refThis, const MsgSay& msg)
{
	auto utf8Content = StrConv::Utf8ToGbk(msg.content);
	LOG(INFO) << "收到聊天:" << utf8Content;
	void SendToWorldSvr(const MsgSay & msg);
	SendToWorldSvr(msg);
}

void MyMsgQueue::OnRecv(MyMsgQueue& refThis, const MsgSelectRoles& msg)
{
	LOG(INFO) << "收到选择:" << msg.ids.size();
	refThis.m_pSession->m_vecSelectedEntities.clear();
	std::transform(msg.ids.begin(), msg.ids.end(), std::back_inserter(refThis.m_pSession->m_vecSelectedEntities), [](const double& id) {return uint64_t(id); });
}

MsgNotifyPos::MsgNotifyPos(Entity* p) :	entityId((uint64_t)p), x(p->m_Pos.x), z(p->m_Pos.z), eulerAnglesY(p->m_eulerAnglesY), hp(p->m_hp)
{}

template void MyMsgQueue::Push(const MsgLogin& msg);
template void MyMsgQueue::Push(const MsgMove& msg);
template void MyMsgQueue::Push(const MsgSay& msg);
template void MyMsgQueue::Push(const MsgSelectRoles& msg);
template void MyMsgQueue::Push(const MsgLogin& msg);