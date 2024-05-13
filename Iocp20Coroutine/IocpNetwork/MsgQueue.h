#pragma once
#include <mutex>
#include <deque>

enum MsgId;

class MsgQueue
{
public:

	/// <summary>
	/// ǿ��Ҫ��MsgIdö��0�ǷǷ�ֵ
	/// </summary>
	/// <returns></returns>
	MsgId PopMsg();

	template<class T_Msg>
	void Push(const T_Msg& msg, std::deque<T_Msg>& queue);

	template<class T_Sub, class T_Msg>
	void OnRecv(std::deque<T_Msg>& queue, T_Sub& refSub, void (*funOnRecv)(T_Sub& refSub, const T_Msg&));

	std::deque<MsgId> m_queueMsgId;
	std::mutex m_mutex;
};
