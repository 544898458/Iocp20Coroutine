#include <glog/logging.h>
#include <cstdlib>
#include "MyMsgQueue.h"
#include "MySession.h"
#include "Space.h"
#include "../CoRoutine/CoTimer.h"
#include "MyServer.h"


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
	case MsgId::Login:
	{
		//std::lock_guard lock(this->m_mutex);
		//const auto msg = this->m_queueLogin.front();
		//this->m_queueLogin.pop_front();
		//OnRecv(msg);
		this->m_MsgQueue.OnRecv(this->m_queueLogin, this, OnRecv);
	}
	break;
	case MsgId::Move:
	{
		/*std::lock_guard lock(this->m_mutex);
		const auto msg = this->m_queueMove.front();
		this->m_queueMove.pop_front();
		OnRecv(msg);*/
		this->m_MsgQueue.OnRecv(this->m_queueMove, this, OnRecv);
	}
	break;
	default:
		break;
	}
}

void MyMsgQueue::Push(const MsgLogin& msg)
{
	//std::lock_guard lock(this->m_mutex);
	//m_queueLogin.push_back(msg);
	//m_queueMsgId.push_back(Login);
	m_MsgQueue.Push(msg, m_queueLogin);
}

template<class T_Msg>
void MsgQueue::Push(const T_Msg& msg, std::deque<T_Msg>& queue)
{
	std::lock_guard lock(this->m_mutex);
	queue.push_back(msg);
	this->m_queueMsgId.push_back(msg.id);
}

template<class T_Sub, class T_Msg>
void MsgQueue::OnRecv(std::deque<T_Msg>& queue, T_Sub* pSub, void (*funOnRecv)(T_Sub* pSub, const T_Msg&))
{
	std::lock_guard lock(this->m_mutex);
	const auto msg = queue.front();
	queue.pop_front();
	funOnRecv(pSub, msg);
}
void MyMsgQueue::Push(const MsgMove& msg)
{
	//std::lock_guard lock(this->m_mutex);
	//m_queueMove.push_back(msg);
	//m_queueMsgId.push_back(Move);
	m_MsgQueue.Push(msg, m_queueMove);
}
std::string GbkToUtf8(const char* src_str)
{
	int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	auto strTemp = str;
	if (wstr) delete[] wstr;
	if (str) delete[] str;
	return strTemp;
}
std::string Utf8ToGbk(const std::string& str)
{
#if defined(_WIN32) || defined(_MSC_VER) || defined(WIN64) 
	// calculate length
	int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	wchar_t* wsGbk = new wchar_t[len + 1ull];
	// set to '\0'
	memset(wsGbk, 0, len + 1ull);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wsGbk, len);
	len = WideCharToMultiByte(CP_ACP, 0, wsGbk, -1, NULL, 0, NULL, NULL);
	char* csGbk = new char[len + 1ull];
	memset(csGbk, 0, len + 1ull);
	WideCharToMultiByte(CP_ACP, 0, wsGbk, -1, csGbk, len, NULL, NULL);
	std::string res(csGbk);

	if (wsGbk)
	{
		delete[] wsGbk;
	}

	if (csGbk)
	{
		delete[] csGbk;
	}

	return res;
#elif defined(__linux__) || defined(__GNUC__)
	size_t len = str.size() * 2 + 1;
	char* temp = new char[len];
	if (EncodingConvert("utf-8", "gb2312", const_cast<char*>(str.c_str()),
		str.size(), temp, len) >= 0)
	{
		std::string res;
		res.append(temp);
		delete[] temp;
		return res;
	}
	else
	{
		delete[] temp;
		return str;
	}

#else
	std::cerr << "Unhandled operating system." << std::endl;
	return str;
#endif
}

void MyMsgQueue::OnRecv(MyMsgQueue* pThis, const MsgLogin& msg)
{
	auto utf8Name = Utf8ToGbk(msg.name);
	//printf("准备广播%s",utf8Name.c_str());
	/*for (auto p : g_set)
	{
		const auto strBroadcast = "[" + utf8Name + "]进来了";
		MsgLoginRet ret = { 223,GbkToUtf8(strBroadcast.c_str()) };
		p->Send(ret);
	}*/
	//const auto strBroadcast = "[" + utf8Name + "]进来了";
	MsgLoginRet ret;
	ret.nickName = GbkToUtf8(utf8Name.c_str());// strBroadcast.c_str());
	ret.entityId = (uint64_t)&pThis->m_pSession->m_entity;
	pThis->m_pSession->m_entity.m_nickName = utf8Name;
	pThis->m_pSession->m_pServer->Broadcast(ret);

	for (const auto pENtity : pThis->m_pSession->m_pServer->g_space.setEntity)
	{
		if (pENtity == &pThis->m_pSession->m_entity)
			continue;

		ret.nickName = GbkToUtf8(pThis->m_pSession->m_entity.m_nickName.c_str());
		ret.entityId = (uint64_t)pENtity;
		pThis->m_pSession->Send(ret);
	}
}

void MyMsgQueue::OnRecv(MyMsgQueue* pThis, const MsgMove& msg)
{
	LOG(INFO) << "收到点击坐标:" << msg.x << "," << msg.z;
	const auto targetX = msg.x;
	const auto targetZ = msg.z;
	auto pServer = pThis->m_pSession->m_pServer;
	pThis->m_pSession->m_entity.ReplaceCo(	//替换协程
		[targetX, targetZ, pServer](Entity* pEntity, float& x, float& z, std::function<void()>& funCancel)->CoTask<int>
		{
			KeepCancel kc(funCancel);
			const auto localTargetX = targetX;
			const auto localTargetZ = targetZ;
			auto pLocalServer = pServer;
			pLocalServer->Broadcast(MsgChangeSkeleAnim(pEntity, "run"));

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
					pLocalServer->Broadcast(MsgChangeSkeleAnim(pEntity, "idle"));
					co_return 0;
				}

				x += localTargetX < x ? -step : step;
				z += localTargetZ < z ? -step : step;

				pLocalServer->Broadcast(MsgNotifyPos(pEntity, x, z));
			}
		});
	pThis->m_pSession->m_entity.m_coWalk.Run();//协程离开开始运行（运行到第一个co_await
}
