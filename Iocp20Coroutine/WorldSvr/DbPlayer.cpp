#include "pch.h"
#include "DbPlayer.h"
#include "../CoRoutine/CoDb.h"
#include "../CoRoutine/CoDbTemplate.h"
#include "../AliyunGreen/AliyunGreen.h"

std::map<std::string, DbPlayer> g_mapDbPlayer;
std::map<std::string, bool> g_mapAliyunGreenCheck;
extern CoDb<DbPlayer> g_CoDbPlayer;
extern CoDb<bool> g_CoAliyunGreenCheck;
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

CoTaskBool DbPlayer::AliyunGreenCheck(const std::string strGbk)
{
	_ASSERT(!strGbk.empty());
	static FunCancel fun;
	//LOG(INFO) << "GameSvr请求扣钱" << msg.changeMoney;
	const bool ok = co_await g_CoAliyunGreenCheck.DoDb([this, strGbk](auto& sp) 
		{
			const auto ok线程 = AliyunGreen::Check(strGbk);
			g_mapAliyunGreenCheck[strGbk] = ok线程
		}, fun);
	co_return g_mapAliyunGreenCheck[strGbk];
}