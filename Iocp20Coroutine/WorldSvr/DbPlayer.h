#pragma once
#include <string>
#include <msgpack.hpp>
#include "../CoRoutine/CoTask.h"
struct DbPlayer
{
	//std::string nickName;
	std::string pwd="Hello, world!pwd";
	int32_t money = 3;
	MSGPACK_DEFINE(pwd, money);

	static CoTask<DbPlayer*> CoGet�������ؿ�(const std::string& refStrNickName);
	CoTaskBool AliyunGreenCheck(const std::string strGbk);
};

extern std::map<std::string, DbPlayer> g_mapDbPlayer;