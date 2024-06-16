#include <glog/logging.h>
#include <cstdlib>
#include "MyMsgQueue.h"
#include "MySession.h"
#include "Space.h"
#include "../CoRoutine/CoTimer.h"
#include "MyServer.h"
#include "../IocpNetwork/MsgQueueTemplate.h"
#include "../IocpNetwork/StrConv.h"

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
	//printf("准备广播%s",utf8Name.c_str());
	/*for (auto p : g_set)
	{
		const auto strBroadcast = "[" + utf8Name + "]进来了";
		MsgLoginRet ret = { 223,GbkToUtf8(strBroadcast.c_str()) };
		p->Send(ret);
	}*/
	//const auto strBroadcast = "[" + utf8Name + "]进来了";
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
	LOG(INFO) << "收到点击坐标:" << msg.x << "," << msg.z;
	const auto targetX = msg.x;
	const auto targetZ = msg.z;
	auto pServer = refThis.m_pSession->m_pServer;
	refThis.m_pSession->m_entity.ReplaceCo(	//替换协程
		[targetX, targetZ, pServer](Entity* pEntity, float& x, float& z, std::function<void()>& funCancel)->CoTask<int>
		{
			KeepCancel kc(funCancel);
			const auto localTargetX = targetX;
			const auto localTargetZ = targetZ;
			auto pLocalServer = pServer;
			pLocalServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(pEntity, "run"));

			while (true)
			{
				if (co_await CoTimer::WaitNextUpdate(funCancel))//服务器主工作线程大循环，每次循环触发一次
				{
					LOG(INFO) << "走向" << localTargetX << "," << localTargetZ << "的协程取消了";
					co_return 0;
				}

				const auto step = 0.5f;
				if (std::abs(localTargetX - x) < step && std::abs(localTargetZ - z) < step) {
					LOG(INFO) << "已走到" << localTargetX << "," << localTargetZ << "附近，协程正常退出";
					pLocalServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(pEntity, "idle"));
					co_return 0;
				}

				x += localTargetX < x ? -step : step;
				z += localTargetZ < z ? -step : step;

				pLocalServer->m_Sessions.Broadcast(MsgNotifyPos(pEntity, x, z));
			}
		});
	refThis.m_pSession->m_entity.m_coWalk.Run();//协程离开开始运行（运行到第一个co_await
}

void MyMsgQueue::OnRecv(MyMsgQueue& refThis, const MsgSay& msg)
{
	auto utf8Content = StrConv::Utf8ToGbk(msg.content);
	LOG(INFO) << "收到聊天:" << utf8Content;
	void SendToWorldSvr(const MsgSay & msg);
	SendToWorldSvr(msg);
}