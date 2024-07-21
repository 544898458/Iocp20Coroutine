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
	case MsgId::Login:this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, OnRecv);break;
	case MsgId::Move:this->m_MsgQueue.OnRecv(this->m_queueMove, *this, OnRecv);break;
	case MsgId::Say:this->m_MsgQueue.OnRecv(this->m_queueSay, *this, OnRecv); break;
	default:
		LOG(ERROR) << "msgId:" <<  msgId;
		assert(false);
		break;
	}
}

void MyMsgQueue::Push(const MsgLogin& msg){	m_MsgQueue.Push(msg, m_queueLogin);}
void MyMsgQueue::Push(const MsgMove& msg) { m_MsgQueue.Push(msg, m_queueMove); }
void MyMsgQueue::Push(const MsgSay& msg) { m_MsgQueue.Push(msg, m_queueSay); }

void MyMsgQueue::OnRecv(MyMsgQueue& refThis, const MsgLogin& msg)
{
	auto utf8Name = StrConv::Utf8ToGbk(msg.name);
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
		MsgSay msg;
		msg.content = StrConv::GbkToUtf8("����������");
		//auto gbk = StrConv::Utf8ToGbk(msg.content);
		refThis.m_pSession->Send(msg);
		return;
	}
	if (!refThis.m_pSession->m_entity.m_nickName.empty())
	{
		MsgSay msg;
		msg.content = StrConv::GbkToUtf8("�����ظ���¼");
		//auto gbk = StrConv::Utf8ToGbk(msg.content);
		refThis.m_pSession->Send(msg);
		return;
	}

	MsgLoginRet ret;
	ret.nickName = StrConv::GbkToUtf8(utf8Name.c_str());// strBroadcast.c_str());
	ret.entityId = (uint64_t)&refThis.m_pSession->m_entity;
	refThis.m_pSession->m_entity.m_nickName = utf8Name;
	refThis.m_pSession->m_pServer->m_Sessions.Broadcast(ret);

	for (const auto pENtity : refThis.m_pSession->m_pServer->m_space.setEntity)
	{
		if (pENtity == & refThis.m_pSession->m_entity)
			continue;

		ret.nickName = StrConv::GbkToUtf8(refThis.m_pSession->m_entity.m_nickName.c_str());
		ret.entityId = (uint64_t)pENtity;
		refThis.m_pSession->Send(ret);
	}
}

void MyMsgQueue::OnRecv(MyMsgQueue& refThis, const MsgMove& msg)
{
	LOG(INFO) << "�յ��������:" << msg.x << "," << msg.z;
	const auto targetX = msg.x;
	const auto targetZ = msg.z;
	auto pServer = refThis.m_pSession->m_pServer;
	refThis.m_pSession->m_entity.WalkToPos(targetX, targetZ, pServer);
}

void MyMsgQueue::OnRecv(MyMsgQueue& refThis, const MsgSay& msg)
{
	auto utf8Content = StrConv::Utf8ToGbk(msg.content);
	LOG(INFO) << "�յ�����:" << utf8Content;
	void SendToWorldSvr(const MsgSay & msg);
	SendToWorldSvr(msg);
}