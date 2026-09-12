#pragma once
#include <string>
#include <msgpack.hpp>
#include "../CoRoutine/CoTask.h"

struct Db’À∫≈
{
	std::string str’À∫≈;
	std::string strNickName;
	std::string strPwd = "Hello, world!pwd";
	uint32_t u32’À∫≈Id = 0;
	MSGPACK_DEFINE(str’À∫≈, strNickName, strPwd, u32’À∫≈Id);
};

struct DbPlayer
{
	Db’À∫≈ ’À∫≈;
	int32_t money = 3;
	MSGPACK_DEFINE(’À∫≈, money);

	static CoTask<DbPlayer*> CoGetæ¯≤ª∑µªÿø’(const std::string& refStr’À∫≈);
	CoTaskBool AliyunGreenCheck(const std::string strGbk);
};

extern std::map<std::string, DbPlayer> g_mapDbPlayer;
