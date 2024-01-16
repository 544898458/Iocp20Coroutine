#include "MsgQueue.h"
#include "MySession.h"
void MsgQueue::Process()
{
	MsgId msgId = MsgId::Login;
	{
		std::lock_guard lock(this->mutex);
		if (this->queueMsgId.empty())
			return;
		const auto msgId = this->queueMsgId.front();
		this->queueMsgId.pop_front();
	}
	switch (msgId)
	{
	case Login:
	{
		std::lock_guard lock(this->mutex);
		const auto msg = this->queueLogin.front();
		this->queueLogin.pop_front();
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
	std::lock_guard lock(this->mutex);
	queueLogin.push_back(msg);
	queueMsgId.push_back(Login);
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
	MsgLoginRet ret = { 223,GbkToUtf8(strBroadcast.c_str()) };
	Broadcast(ret);
}
