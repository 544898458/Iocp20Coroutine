#include <glog/logging.h>
#include <cstdlib>
#include "MsgQueue.h"
#include "MySession.h"
#include "Space.h"
#include "CoTimer.h"
#include "MyServer.h"
void MsgQueue::Process()
{
	MsgId msgId = MsgId::Login;
	{
		std::lock_guard lock(this->m_mutex);
		if (this->m_queueMsgId.empty())
			return;

		msgId = this->m_queueMsgId.front();
		this->m_queueMsgId.pop_front();
	}
	switch (msgId)
	{
	case Login:
	{
		std::lock_guard lock(this->m_mutex);
		const auto msg = this->m_queueLogin.front();
		this->m_queueLogin.pop_front();
		OnRecv(msg);
	}
	break;
	case Move:
	{
		std::lock_guard lock(this->m_mutex);
		const auto msg = this->m_queueMove.front();
		this->m_queueMove.pop_front();
		OnRecv(msg);
	}
	break;
	default:
		break;
	}
}

void MsgQueue::Push(const MsgLogin& msg)
{
	//printf("Push,");
	std::lock_guard lock(this->m_mutex);
	m_queueLogin.push_back(msg);
	m_queueMsgId.push_back(Login);
}
void MsgQueue::Push(const MsgMove& msg)
{
	//printf("Push,");
	std::lock_guard lock(this->m_mutex);
	m_queueMove.push_back(msg);
	m_queueMsgId.push_back(Move);
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

void MsgQueue::OnRecv(const MsgLogin& msg)
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
	ret.entityId = (uint64_t)&m_pSession->m_entity;
	m_pSession->m_entity.m_nickName = utf8Name;
	m_pSession->m_pServer->Broadcast(ret);

	for (const auto pENtity : m_pSession->m_pServer->g_space.setEntity)
	{
		if (pENtity == &m_pSession->m_entity)
			continue;

		ret.nickName = GbkToUtf8(m_pSession->m_entity.m_nickName.c_str());
		ret.entityId = (uint64_t)pENtity;
		m_pSession->Send(ret);
	}
}

void MsgQueue::OnRecv(const MsgMove& msg)
{
	LOG(INFO) << "收到点击坐标:" << msg.x << "," << msg.z;
	const auto targetX = msg.x;
	const auto targetZ = msg.z;
	m_pSession->m_entity.ReplaceCo(	//替换协程
		[targetX, targetZ, this](Entity* pEntity, float& x, float& z, std::function<void()>& funCancel)->CoTask<int>
		{
			KeepCancel kc(funCancel);
			const auto localTargetX = targetX;
			const auto localTargetZ = targetZ;

			m_pSession->m_pServer->Broadcast(MsgChangeSkeleAnim(pEntity, "run"));
			
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
					m_pSession->m_pServer->Broadcast(MsgChangeSkeleAnim(pEntity, "idle"));
					co_return 0;
				}

				x += localTargetX < x ? -step : step;
				z += localTargetZ < z ? -step : step;

				m_pSession->m_pServer->Broadcast(MsgNotifyPos(pEntity, x, z));
			}
		});
	m_pSession->m_entity.m_coWalk.Run();//协程离开开始运行（运行到第一个co_await
}
