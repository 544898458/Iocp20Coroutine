#pragma once
#include <msgpack.hpp>

enum MsgId
{
	Login,
};
struct MsgLogin 
{
	std::string name;
	std::string pwd;
	MSGPACK_DEFINE(name, pwd);
};

struct MsgLoginRet
{
	int id;
	std::string nickName;
	MSGPACK_DEFINE(id, nickName);
};
class MsgQueue
{
public:
	void Process();
	void Push(const MsgLogin& msg) 
	{
		queueLogin.push_back(msg);
		queueMsgId.push_back(Login);
	}
	void OnRecv(const MsgLogin& msg) {}
	std::deque<MsgLogin> queueLogin;
	std::deque<MsgId> queueMsgId;
};

extern MsgQueue g_MsgQueue;