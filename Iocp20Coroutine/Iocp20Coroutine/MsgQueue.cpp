#include <glog/logging.h>
#include "MsgQueue.h"
#include "MySession.h"

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
		const auto msg = this->queueLogin.front();
		this->queueLogin.pop_front();
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
	queueLogin.push_back(msg);
	m_queueMsgId.push_back(Login);
}
void MsgQueue::Push(const MsgMove& msg)
{
	//printf("Push,");
	std::lock_guard lock(this->m_mutex);
	m_queueMove.push_back(msg);
	m_queueMsgId.push_back(Move);
}
string GbkToUtf8(const char* src_str)
{
	int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	string strTemp = str;
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
	const auto strBroadcast = "[" + utf8Name + "]进来了";
	MsgLoginRet ret = { (int)LoginRet,GbkToUtf8(strBroadcast.c_str()) };
	Broadcast(ret);
}

void MsgQueue::OnRecv(const MsgMove& msg)
{
	LOG(INFO) << "收到点击坐标:" << msg.x << "," << msg.z;
	const auto targetX = msg.x;
	const auto targetZ = msg.z;
	m_pSession->m_entity.ReplaceCo(
		[targetX, targetZ](Entity* pEntity, float& x, float& z, bool& stop)->CoTask<int>
		{
			const auto localTargetX = targetX;
			const auto localTargetZ = targetZ;
			while (true)
			{
				co_yield 0;
				if (stop)
				{
					LOG(INFO) << "走向" << localTargetX << "的协程正常退出";
					co_return 0;
				}

				x += localTargetX < x ? -0.5f : 0.5f;
				z += localTargetZ < z ? -0.5f : 0.5f;


				MsgNotifyPos msg = { (int)NotifyPos , (uint64_t)pEntity, x,z };
				Broadcast(msg);
			}
		});
}
