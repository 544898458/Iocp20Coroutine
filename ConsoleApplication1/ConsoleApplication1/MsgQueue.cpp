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
void MsgQueue::OnRecv(const MsgLogin& msg)
{
	for (auto p : g_set)
	{
		MsgLoginRet ret = { 223,GbkToUtf8("´óÏÀ") };
		p->Send(ret);
	}

}
