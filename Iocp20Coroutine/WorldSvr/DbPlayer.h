#pragma once
#include <string>
#include <msgpack.hpp>
#include "../CoRoutine/CoTask.h"
struct DbPlayer
{
	std::string nickName;
	std::string pwd="Hello, world!pwd";
	int32_t money = 0;
	MSGPACK_DEFINE(nickName, pwd, money);

	static CoTask<DbPlayer*> CoGet¾ø²»·µ»Ø¿Õ(const std::string& refStrNickName);
};

extern std::map<std::string, DbPlayer> g_mapDbPlayer;