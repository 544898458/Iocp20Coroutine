#pragma once
#include <msgpack.hpp>
#include <mutex>
enum MsgId
{
	Login,
	Move,
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
	MSGPACK_DEFINE(id,x, y,z);
};

struct MsgLoginRet
{
	int id;
	std::string nickName;
	MSGPACK_DEFINE(id, nickName);
};

struct MsgNotifyPos
{
	long entityId;
	float x;
	MSGPACK_DEFINE(entityId,x);
};

class MySession;
class MsgQueue
{
public:
	/// <summary>
	/// 工作线程中（单线程）调用
	/// </summary>
	void Process();

	/// <summary>
	/// 网络线程中（多线程）调用
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
};