#pragma once
#include "../GameSvr/MyMsgQueue.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
#include "SpEntity.h"
#include "Space.h"
#include <set>
class GameSvrSession;


/// <summary>
/// 这个GameSvr里的对象，对应一个GateSvr对游戏客户端的连接
/// </summary>
class PlayerGateSession_Game
{
public:
	PlayerGateSession_Game(GameSvrSession& ref, uint64_t idPlayerGateSession);
	PlayerGateSession_Game(const PlayerGateSession_Game&) = delete;
	PlayerGateSession_Game(PlayerGateSession_Game&& ref) = delete;
	void RecvMsg(const MsgId idMsg, const msgpack::object& obj);
	void Process();
	void OnDestroy();
	void Erase(SpEntity& spEntity);
	template<class T> void Send(const T& ref);
	void Say(const std::string& str, const SayChannel channel);
	void Say系统(const std::string& str);
	void Say语音提示(const std::string& str);
	
	const std::string& NickName()const { return m_strNickName; }
	void EnterSpace(WpSpace wpSpace, const std::string& strNickName);
	void Send资源();
	uint16_t 活动单位上限() const;
	uint16_t 活动单位包括制造队列中的() const;

	bool 可放置建筑(const Position& pos, float f半边长);
	CoTask<SpEntity> CoAddBuilding(const 建筑单位类型 类型, const Position pos);
	void 播放声音(const std::string& refStr声音);

	uint32_t m_snRecv = 0;
	const uint64_t m_idPlayerGateSession;
	/// <summary>
	/// 当前这个玩家在哪个Space里
	/// </summary>
	WpSpace m_wpSpace;
	SpSpace m_spSpace单人剧情副本;
	/// <summary>
	/// 加入Space空间的实体（玩家角色）
	/// </summary>
	std::set<SpEntity> m_setSpEntity;

	uint32_t m_u32燃气矿 = 100;

private:
	template<class T_Msg> void RecvMsg(const msgpack::object& obj);
	/// <summary>
	/// 主逻辑线程（控制台界面线程）调用
	/// </summary>
	/// <param name="msg"></param>
	void OnRecv(const Msg进Space& msg);
	void OnRecv(const Msg离开Space& msg);
	void OnRecv(const Msg进单人剧情副本& msg);
	void OnRecv(const MsgMove& msg);
	void OnRecv(const MsgSay& msg);
	void OnRecv(const MsgSelectRoles& msg);
	void OnRecv(const MsgAddRole& msg);
	void OnRecv(const MsgAddBuilding& msg);
	void OnRecv(const Msg采集& msg);
	void OnRecv(const Msg进地堡& msg);
	void OnRecv(const Msg出地堡& msg);

	void ForEachSelected(std::function<void(Entity& ref)> fun);
	//CoTask<int> CoAddRole();
	
	//std::vector<CoTask<int>>	m_vecCoRpc;
	std::vector<std::shared_ptr<FunCancel>>	m_vecFunCancel;
	bool m_bLoginOk = false;

	GameSvrSession& m_refSession;

	std::list<uint64_t> m_listSelectedEntity;
	std::string m_strNickName;
	uint32_t m_snSend = 0;
	FunCancel m_funCancel单人剧情;
};

