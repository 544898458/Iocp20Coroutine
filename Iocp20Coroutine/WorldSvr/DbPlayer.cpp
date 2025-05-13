#include "pch.h"
#include "DbPlayer.h"
#include "../CoRoutine/CoDb.h"
#include "../CoRoutine/CoDbTemplate.h"
#include "../AliyunGreen/AliyunGreen.h"

std::map<std::string, DbPlayer> g_mapDbPlayer;
std::map<std::string, bool> g_mapAliyunGreenCheck;
extern CoDb<DbPlayer> g_CoDbPlayer;

template CoAwaiter<DbPlayer>& CoDb<DbPlayer>::CoSave(const DbPlayer&, const std::string& strNickName, FunCancel&);

CoTask<DbPlayer*> DbPlayer::CoGet¾ø²»·µ»Ø¿Õ(const std::string& refStrNickName)
{
	_ASSERT(!refStrNickName.empty());
	static FunCancel fun;
	//LOG(INFO) << "GameSvrÇëÇó¿ÛÇ®" << msg.changeMoney;
	if (g_mapDbPlayer.find(refStrNickName) == g_mapDbPlayer.end())
	{
		DbPlayer loadDb = co_await g_CoDbPlayer.Load(refStrNickName, fun);
		g_mapDbPlayer.insert({ refStrNickName, loadDb });
	}
	co_return &g_mapDbPlayer[refStrNickName];
}