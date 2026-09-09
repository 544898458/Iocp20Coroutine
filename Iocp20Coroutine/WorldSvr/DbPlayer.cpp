#include "pch.h"
#include "DbPlayer.h"
#include "../CoRoutine/CoDb.h"
#include "../CoRoutine/CoDbTemplate.h"
#include "../AliyunGreen/AliyunGreen.h"
#include "../读配置文件/文件存取对象.h"

std::map<std::string, DbPlayer> g_mapDbPlayer;
std::map<std::string, bool> g_mapAliyunGreenCheck;
extern CoDb<DbPlayer> g_CoDbPlayer;

namespace
{
	struct Db账号索引
	{
		uint32_t u32下一个账号Id = 1;
		std::map<std::string, uint32_t> map账号Id;
		MSGPACK_DEFINE(u32下一个账号Id, map账号Id);
	};
}

uint32_t GetOrCreate账号Id(const std::string& refStr账号)
{
	SAFE_ASSERT(!refStr账号.empty());
	static Db账号索引 s_账号索引 = 从文件里读出对象<Db账号索引>("全局账号索引");
	if (0 == s_账号索引.u32下一个账号Id)
		s_账号索引.u32下一个账号Id = 1;
	const auto iterFind = s_账号索引.map账号Id.find(refStr账号);
	if (iterFind != s_账号索引.map账号Id.end())
		return iterFind->second;
	const auto u32账号Id = s_账号索引.u32下一个账号Id++;
	const auto [_, b插入] = s_账号索引.map账号Id.insert({ refStr账号, u32账号Id });
	SAFE_ASSERT(b插入);
	写对象进文件(s_账号索引, "全局账号索引");
	return u32账号Id;
}

template CoAwaiter<DbPlayer>& CoDb<DbPlayer>::CoSave(const DbPlayer&, const std::string& strNickName, FunCancel&);

CoTask<DbPlayer*> DbPlayer::CoGet绝不返回空(const std::string& refStrNickName)
{
	_ASSERT(!refStrNickName.empty());
	static FunCancel fun;
	//LOG(INFO) << "GameSvr请求扣钱" << msg.changeMoney;
	if (g_mapDbPlayer.find(refStrNickName) == g_mapDbPlayer.end())
	{
		DbPlayer loadDb = co_await g_CoDbPlayer.Load(refStrNickName, fun);
		g_mapDbPlayer.insert({ refStrNickName, loadDb });
	}
	co_return &g_mapDbPlayer[refStrNickName];
}