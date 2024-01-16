#pragma once
#include <msgpack.hpp>
#include <mutex>
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

struct MsgNotifyPos
{
	float x;
	MSGPACK_DEFINE(x);
};

class MySession;
class MsgQueue
{
public:
	/// <summary>
	/// �����߳��У����̣߳�����
	/// </summary>
	void Process();

	/// <summary>
	/// �����߳��У����̣߳�����
	/// </summary>
	/// <param name="msg"></param>
	void Push(const MsgLogin& msg);
	void OnRecv(const MsgLogin& msg);
	std::deque<MsgLogin> queueLogin;
	std::deque<MsgId> queueMsgId;
	std::mutex mutex;
};