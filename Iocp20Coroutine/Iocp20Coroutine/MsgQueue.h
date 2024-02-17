#pragma once
#include <msgpack.hpp>
#include <mutex>
enum MsgId
{
	Login,
	Move,
	LoginRet,
	NotifyPos,
};
struct MsgLogin
{
	int id;
	std::string name;
	std::string pwd;
	MSGPACK_DEFINE(id, name, pwd);
};

struct MsgMove
{
	int id;
	float x;
	float y;
	float z;
	MSGPACK_DEFINE(id, x, y, z);
};

struct MsgLoginRet
{
	int id = (int)LoginRet;
	std::string nickName;
	MSGPACK_DEFINE(id, nickName);
};

struct MsgNotifyPos
{
	int msgId = (int)NotifyPos;
	uint64_t entityId;
	float x;
	MSGPACK_DEFINE(msgId, entityId, x);
};

class MySession;
class MsgQueue
{
public:
	MsgQueue(MySession* p) :pSession(p)
	{

	}
	/// <summary>
	/// �����߳��У����̣߳�����
	/// </summary>
	void Process();

	/// <summary>
	/// �����߳��У����̣߳�����
	/// </summary>
	/// <param name="msg"></param>
	void Push(const MsgLogin& msg);
	void Push(const MsgMove& msg);
	void OnRecv(const MsgLogin& msg);
	void OnRecv(const MsgMove& msg);
	std::deque<MsgLogin> queueLogin;
	std::deque<MsgMove> queueMove;
	std::deque<MsgId> queueMsgId;
	std::mutex mutex;
private:
	MySession* const pSession;
};