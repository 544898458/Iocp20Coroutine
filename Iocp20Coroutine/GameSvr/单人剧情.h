#pragma once
#include "../CoRoutine/CoTask.h"
class Space;
class PlayerGateSession_Game;
class PlayerComponent;
class Entity;

namespace 单人剧情
{
	CoTask<int> Co新手训练_单位介绍_人(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);
	CoTask<int> Co新手训练_单位介绍_虫(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);
	CoTask<int> Co新手训练_战斗_人(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);
	CoTask<int> Co新手训练_战斗_虫(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);
	CoTask<int> Co防守战(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);
	CoTask<int> Co防守战_虫(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);
	CoTask<int> Co攻坚战(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);
	CoTask<int> Co攻坚战_虫(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);
	CoTask<int> Co新手训练_空降战_虫(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);
	CoTask<int> Co新手训练_反空降战_人(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);
	CoTask<int> Co中央防守_人(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);
};

