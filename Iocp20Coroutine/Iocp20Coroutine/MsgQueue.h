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
	float z;
	MSGPACK_DEFINE(msgId, entityId, x, z);
};

class MySession;
class MsgQueue
{
public:
	MsgQueue(MySession* p) :m_pSession(p)
	{
	}
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

	/// <summary>
	/// 主逻辑线程（控制台界面线程）调用
	/// </summary>
	/// <param name="msg"></param>
	void OnRecv(const MsgLogin& msg);
	void OnRecv(const MsgMove& msg);
	/// <summary>
	/// 这里保存的都是解析后的消息明文
	/// </summary>
	std::deque<MsgLogin> m_queueLogin;
	std::deque<MsgMove> m_queueMove;
	std::deque<MsgId> m_queueMsgId;

	std::mutex m_mutex;
private:
	/// <summary>
	/// 弱引用，不要销毁
	/// </summary>
	MySession* const m_pSession;
};