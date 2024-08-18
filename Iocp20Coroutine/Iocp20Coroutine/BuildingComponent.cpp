#include "stdafx.h"
#include "BuildingComponent.h"
#include "Entity.h"
#include "../CoRoutine/CoRpc.h"

void Entity::AddComponentBuilding()
{
	m_spBuilding = std::make_shared<BuildingComponent, Entity&>(*this);
}
void SendToWorldSvr(const MsgChangeMoney& msg);
BuildingComponent::BuildingComponent(Entity& refEntity)
{
	m_coAddMoney = [&refEntity]()->CoTask<int>
		{

			MsgChangeMoneyResponce responce = co_await CoRpc<MsgChangeMoneyResponce>::Send<MsgChangeMoney>({ .addMoney = true,.changeMoney = 10 }, SendToWorldSvr);//以同步编程的方式，向另一个服务器发送请求并等待返回
			LOG(INFO) << "协程RPC返回,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
			auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string& >({ 30,30 }, refEntity.m_space, "altman-blue");
			if (0 != responce.error)
			{
				LOG(WARNING) << "扣钱失败";
				co_return 0;
			}
		}();
		m_coAddMoney.Run();
}