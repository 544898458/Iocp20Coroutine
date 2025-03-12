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
class PlayerGateSession_Game : public std::enable_shared_from_this<PlayerGateSession_Game>//必须公有继承，否则无效
{
public:
	PlayerGateSession_Game(GameSvrSession& ref, uint64_t idPlayerGateSession, const std::string& strNickName);
	PlayerGateSession_Game(const PlayerGateSession_Game&) = delete;
	PlayerGateSession_Game(PlayerGateSession_Game&& ref) = delete;
	void RecvMsg(const MsgId idMsg, const msgpack::object& obj);
	void Process();
	void OnDestroy();
	void 离开Space(const bool b主动退);

	template<class T> void Send(const T& ref);
	void Say(const std::string& str, const SayChannel channel);
	void Say系统(const std::string& str);
	static void Say系统(const std::string& refStrNickName, const std::string& str);
	static void Say任务提示(const std::string& refStrNickName, const std::string& str);
	//void Say语音提示(const std::string& str);

	const std::string& NickName()const { return m_strNickName; }
	WpEntity EnterSpace(WpSpace wpSpace);
	void Send资源();
	uint16_t 活动单位上限() const;
	uint16_t 活动单位包括制造队列中的() const;

	void 播放声音Buzz(const std::string& str文本);
	void 播放声音(const std::string& refStr声音, const std::string& str文本);
	static void 播放声音(const std::string& refStrNickName, const std::string& refStr声音, const std::string& str文本);

	void Send设置视口(const Entity& refEntity);
	void 删除选中(const uint64_t id);

	void 剧情对话(
		const std::string& str头像左, const std::string& str名字左,
		const std::string& str头像右, const std::string& str名字右,
		const std::string& str内容, const bool b显示退出场景按钮);
	void 剧情对话已看完();

	uint32_t m_snRecv = 0;
	const uint64_t m_idPlayerGateSession;
	/// <summary>
	/// 当前这个玩家在哪个Space里
	/// </summary>
	WpSpace m_wpSpace;
	//WpSpace m_wpSpace单人剧情副本;
	//SpSpace m_spSpace多人战局;
	WpEntity m_wp视口;
private:
	template<class T_Msg> void RecvMsg(const msgpack::object& obj);
	/// <summary>
	/// 主逻辑线程（控制台界面线程）调用
	/// </summary>
	/// <param name="msg"></param>
	void OnRecv(const Msg进Space& msg);
	void OnRecv(const Msg离开Space& msg);
	void OnRecv(const Msg进单人剧情副本& msg);
	void OnRecv(const Msg创建多人战局& msg);
	void OnRecv(const MsgMove& msg);
	void OnRecv(const MsgSay& msg);
	void OnRecv(const MsgSelectRoles& msg);
	void OnRecv(const MsgAddRole& msg);
	void OnRecv(const MsgAddBuilding& msg);
	void OnRecv(const Msg采集& msg);
	void OnRecv(const Msg进地堡& msg);
	void OnRecv(const Msg出地堡& msg);
	void OnRecv(const Msg框选& msg);
	void OnRecv(const Msg玩家个人战局列表& msg);
	void OnRecv(const Msg玩家多人战局列表& msg);
	void OnRecv(const Msg进其他玩家个人战局& msg);
	void OnRecv(const Msg进其他玩家多人战局& msg);
	void OnRecv(const Msg切换空闲工程车& msg);
	void OnRecv(const Msg剧情对话已看完& msg);
	void OnRecv(const Msg建筑产出活动单位的集结点& msg);

	void 选中单位(std::vector<uint64_t> vecId, const bool b追加 = false);
	std::vector<WpEntity> Get空闲工程车(单位类型 造活动单位类型, bool b包括采集中的工程车);

	void Send选中音效(const Entity& refEntity);

	void Send选中单位Responce();


	void ForEachSelected(std::function<void(Entity& ref)> fun);
	CoTaskBool Co进多人联机地图(WpEntity wp视口);

	//std::vector<CoTask<int>>	m_vecCoRpc;
	std::vector<std::shared_ptr<FunCancel>>	m_vecFunCancel;
	bool m_bLoginOk = false;

	GameSvrSession& m_refGameSvrSession;

	std::vector<uint64_t> m_vecSelectedEntity;
	uint64_t m_id上次单选选中 = 0;
	std::string m_strNickName;
	uint32_t m_snSend = 0;
	FunCancel m_funCancel进地图;
	uint16_t m_idx切换工程车 = 0;
	Position m_pos上次点击走路目标;
};

