#include "pch.h"
#include "PlayerGateSession_Game.h"
#include "Space.h"
#include "Entity.h"
#include "../IocpNetwork/MsgPack.h"
#include "../IocpNetwork/StrConv.h"
#include "../CoRoutine/CoRpc.h"
#include "../CoRoutine/CoEvent.h"
#include "AiCo.h"
#include "AttackComponent.h"
#include "DefenceComponent.h"
#include "�ɼ�Component.h"
#include "GameSvr.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"
#include "BuildingComponent.h"
#include <unordered_map>
#include "��λ.h"
#include "���˾���.h"
#include <sstream>
/// <summary>
/// GameSvrͨ��GateSvr͸������Ϸ�ͻ���
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="ref"></param>
template<class T>
void PlayerGateSession_Game::Send(const T& ref)
{
	++m_snSend;
	ref.msg.sn = (m_snSend);

	MsgPack::SendMsgpack(ref, [this](const void* buf, int len)
		{
			MsgGateת�� msg(buf, len, m_idPlayerGateSession, m_snSend);
			MsgPack::SendMsgpack(msg, [this](const void* bufת��, int lenת��)
				{
					this->m_refSession.SendToGate(bufת��, lenת��);
				});
		}, false);
}


void PlayerGateSession_Game::OnDestroy()
{
	for (auto sp : m_setSpEntity)
	{
		sp->m_refSpace.m_mapEntity.erase((int64_t)sp.get());
		LOG(INFO) << "m_mapEntity.size=" << sp->m_refSpace.m_mapEntity.size();
		sp->OnDestroy();
	}

	m_setSpEntity.clear();

	for (auto& sp : m_vecFunCancel)
	{
		auto& fun = *sp;
		if (fun)
			fun();
	}
	m_vecFunCancel.clear();
	/*m_pServer->m_Sessions.DeleteSession(this->m_pWsSession->m_pSession, [this]()
		{
		});*/

	if (m_funCancel���˾���)
		m_funCancel���˾���();
}

void PlayerGateSession_Game::Erase(SpEntity& spEntity)
{
	if (!m_setSpEntity.contains(spEntity))
	{
		LOG(WARNING) << "ERR";
		return;
	}

	m_setSpEntity.erase(spEntity);
}

void PlayerGateSession_Game::Say(const std::string& str)
{
	Send(MsgSay(StrConv::GbkToUtf8(str)));
}

void PlayerGateSession_Game::OnRecv(const MsgAddRole& msg)
{
	ForEachSelected([this](Entity& ref)
		{
			if (ref.m_spBuilding)
				ref.m_spBuilding->���(*this, ref);
		});
}

void PlayerGateSession_Game::OnRecv(const Msg�ɼ�& msg)
{
	ForEachSelected([this, &msg](Entity& ref)
		{
			if (!ref.m_sp�ɼ�)
				return;

			CHECK_NOTNULL_VOID(m_pCurSpace);
			auto wpEntity = m_pCurSpace->GetEntity((int64_t)msg.idĿ����Դ);
			if (!wpEntity.expired())
				ref.m_sp�ɼ�->�ɼ�(*this, ref, wpEntity);
		});
}


void PlayerGateSession_Game::OnRecv(const MsgMove& msg)
{
	LOG(INFO) << "�յ��������:" << msg.x << "," << msg.z;
	const auto targetX = msg.x;
	const auto targetZ = msg.z;
	CHECK_NOTNULL_VOID(m_pCurSpace);
	ForEachSelected([this, targetX, targetZ](Entity& ref)
		{
			if (ref.m_spAttack)
			{
				if (ref.m_sp�ɼ�)
					ref.m_sp�ɼ�->m_TaskCancel.TryCancel();

				ref.m_spAttack->TryCancel();
				ref.m_spAttack->WalkToPos�ֶ�����(Position(targetX, targetZ));
			}
		});
}

void PlayerGateSession_Game::ForEachSelected(std::function<void(Entity& ref)> fun)
{
	CHECK_NOTNULL_VOID(m_pCurSpace);
	for (const auto id : m_vecSelectedEntity)
	{
		auto itFind = m_pCurSpace->m_mapEntity.find(id);
		if (itFind == m_pCurSpace->m_mapEntity.end())
		{
			LOG(INFO) << "ѡ�е�ʵ�岻����:" << id;
			//assert(false);
			continue;
		}
		auto& spEntity = itFind->second;
		if (m_setSpEntity.end() == std::find_if(m_setSpEntity.begin(), m_setSpEntity.end(), [&spEntity](const auto& sp) {return sp == spEntity; }))
		{
			LOG(ERROR) << id << "�����Լ��ĵ�λ�����ܲ���";
			continue;
		}

		fun(*spEntity);

	}

	//CoAddRole().RunNew();
}

template<class T> void SendToWorldSvr(const T& msg, const uint64_t idGateSession);

void PlayerGateSession_Game::OnRecv(const MsgAddBuilding& msg)
{
	//if (!m_coRpc.Finished())
	//{
	//	LOG(WARNING) << "m_coRpcǰһ��Э�̻�û����";
	//	return;
	//}
	/*auto iterNew = m_vecCoRpc.insert(m_vecCoRpc.end(), CoAddBuilding(msg.����));
	if (iterNew == m_vecCoRpc.end())
	{
		LOG(ERROR) << "err";
		return;
	}
	iterNew->Run();*/
	CoAddBuilding(msg.����).RunNew();
}

CoTask<int> PlayerGateSession_Game::CoAddBuilding(const ������λ���� ����)
{
	��λ::������λ���� ����;
	if (!��λ::Find������λ����(����, ����))
	{
		co_return 0;
	}
	if (����.����.u32����ȼ���� > m_u32ȼ����)
	{
		std::ostringstream oss;
		oss << "ȼ������" << ����.����.u32����ȼ����;
		Say(oss.str());
		co_return 0;
	}
	m_u32ȼ���� -= ����.����.u32����ȼ����;
	auto iterNew = m_vecFunCancel.insert(m_vecFunCancel.end(), std::make_shared<FunCancel>());//���ܴ�������ݿ��ܵ������ú�ָ��ʧЧ
	auto [stop, responce] = co_await CoRpc<MsgChangeMoneyResponce>::Send<MsgChangeMoney>({ .changeMoney = ����.����.u32���ľ���� },
		[this](const MsgChangeMoney& ref) {SendToWorldSvr<MsgChangeMoney>(ref, m_idPlayerGateSession); }, **iterNew);//��ͬ����̵ķ�ʽ������һ���������������󲢵ȴ�����
	LOG(INFO) << "Э��RPC����,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
	if (stop)
	{
		m_u32ȼ���� += ����.����.u32����ȼ����;//����ȼ����
		co_return 0;
	}
	if (0 != responce.error)
	{
		//LOG(WARNING) << "��Ǯʧ��,error=" << responce.error;
		m_u32ȼ���� += ����.����.u32����ȼ����;//����ȼ����
		Say("��������" + ����.����.u32���ľ����);
		co_return 0;
	}

	//�ӽ���
	CHECK_NOTNULL_CO_RET_0(m_pCurSpace);
	auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string&, const std::string& >(
		{ 35,float(std::rand() % 20) }, *m_pCurSpace, ����.����.strPrefabName, ����.����.strName);
	//spNewEntity->AddComponentAttack();
	spNewEntity->AddComponentPlayer(*this);
	BuildingComponent::AddComponent(*spNewEntity, *this, ����, ����.f��߳�);
	DefenceComponent::AddComponent(*spNewEntity);
	spNewEntity->m_spBuilding->m_fun����λ = ����.fun���;
	m_setSpEntity.insert(spNewEntity);//�Լ����Ƶĵ�λ
	m_pCurSpace->m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//ȫ��ͼ��λ

	spNewEntity->BroadcastEnter();
	co_return 0;
}

void PlayerGateSession_Game::EnterSpace(Space& refSpace, const std::string& strNickName)
{
	m_pCurSpace = &refSpace;
	m_strNickName = strNickName;
	for (const auto& [id, spEntity] : refSpace.m_mapEntity)//���е�ͼ�ϵ�ʵ�巢���Լ�
	{
		Send(MsgAddRoleRet(*spEntity));
		Send(MsgNotifyPos(*spEntity));
	}

	CoEvent<PlayerGateSession_Game*>::OnRecvEvent(false, this);
	���˾���::Co(m_Space���˾���, m_funCancel���˾���, *this).RunNew();
}

void PlayerGateSession_Game::OnRecv(const MsgSay& msg)
{
	auto utf8Content = StrConv::Utf8ToGbk(msg.content);
	LOG(INFO) << "�յ�����:" << utf8Content;
	SendToWorldSvr<MsgSay>(msg, m_idPlayerGateSession);
}

void PlayerGateSession_Game::OnRecv(const MsgSelectRoles& msg)
{
	LOG(INFO) << "�յ�ѡ��:" << msg.ids.size();
	m_vecSelectedEntity.clear();
	std::transform(msg.ids.begin(), msg.ids.end(), std::back_inserter(m_vecSelectedEntity), [](const double& id) {return uint64_t(id); });
}

template void PlayerGateSession_Game::Send(const MsgAddRoleRet&);
template void PlayerGateSession_Game::Send(const MsgNotifyPos&);
template void PlayerGateSession_Game::Send(const MsgChangeSkeleAnim&);
template void PlayerGateSession_Game::Send(const MsgSay&);
template void PlayerGateSession_Game::Send(const MsgDelRoleRet&);
template void PlayerGateSession_Game::Send(const MsgNotifyMoney&);


template<class T_Msg>
void PlayerGateSession_Game::RecvMsg(const msgpack::object& obj)
{
	try
	{
		const auto msg = obj.as<T_Msg>();
		OnRecv(msg);
	}
	catch (const msgpack::type_error& error)
	{
		LOG(ERROR) << typeid(T_Msg).name() << ",�����л�ʧ��," << error.what();
		assert(false);
		return;
	}
}
extern Space g_Space����ˢ��;
PlayerGateSession_Game::PlayerGateSession_Game(GameSvrSession& ref, uint64_t idPlayerGateSession) :
	m_refSession(ref), m_idPlayerGateSession(idPlayerGateSession), m_Space���˾���(g_Space����ˢ��)
{

}

void PlayerGateSession_Game::RecvMsg(const MsgId idMsg, const msgpack::object& obj)
{
	switch (idMsg)
	{
	case MsgId::Move:RecvMsg<MsgMove>(obj); break;
	case MsgId::Say:RecvMsg<MsgSay >(obj); break;
	case MsgId::SelectRoles:RecvMsg<MsgSelectRoles>(obj); break;
	case MsgId::AddRole:RecvMsg<MsgAddRole>(obj); break;
	case MsgId::AddBuilding:RecvMsg<MsgAddBuilding>(obj); break;
	case MsgId::�ɼ�:RecvMsg<Msg�ɼ�>(obj); break;
	case MsgId::Gateת��:
		LOG(ERROR) << "������ת��";
		assert(false);
		break;
	default:
		LOG(ERROR) << "û����msgId:" << idMsg;
		assert(false);
		break;
	}
}

void PlayerGateSession_Game::Process()
{
	{
		const auto oldSize = m_vecFunCancel.size();
		std::erase_if(m_vecFunCancel, [](std::shared_ptr<FunCancel>& sp)->bool
			{
				return !(*sp).operator bool();
			});
		const auto newSize = m_vecFunCancel.size();
		if (oldSize != newSize)
		{
			LOG(INFO) << "oldSize:" << oldSize << ",newSize:" << newSize;
		}
	}
	//{
	//	const auto oldSize = m_vecCoRpc.size();
	//	std::erase_if(m_vecCoRpc, [](CoTask<int>& refCo)->bool {return refCo.Finished(); });
	//	const auto newSize = m_vecCoRpc.size();
	//	if (oldSize != newSize)
	//	{
	//		LOG(INFO) << "oldSize:" << oldSize << ",newSize:" << newSize;
	//	}
	//}

	//while (true)
	//{
	//	const MsgId msgId = this->m_MsgQueue.PopMsg();
	//	if (MsgId::Invalid_0 == msgId)//û����Ϣ�ɴ���
	//		break;

	//	switch (msgId)
	//	{
	//	case MsgId::Login:this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, &PlayerGateSession::OnRecv); break;
	//	case MsgId::Move:this->m_MsgQueue.OnRecv(this->m_queueMove, *this, &PlayerGateSession::OnRecv); break;
	//	case MsgId::Say:this->m_MsgQueue.OnRecv(this->m_queueSay, *this, &PlayerGateSession::OnRecv); break;
	//	case MsgId::SelectRoles:this->m_MsgQueue.OnRecv(this->m_queueSelectRoles, *this, &PlayerGateSession::OnRecv); break;
	//	case MsgId::AddRole:this->m_MsgQueue.OnRecv(this->m_queueAddRole, *this, &PlayerGateSession::OnRecv); break;
	//	case MsgId::AddBuilding:this->m_MsgQueue.OnRecv(this->m_queueAddBuilding, *this, &PlayerGateSession::OnRecv); break;
	//		//case MsgId::Gateת��:this->m_MsgQueue.OnRecv(this->m_queueGateת��, *this, &OnRecv); break;
	//	default:
	//		LOG(ERROR) << "msgId:" << msgId;
	//		assert(false);
	//		break;
	//	}
	//}
	m_Space���˾���.Update();
}


void PlayerGateSession_Game::Send��Դ()
{
	Send<Msg��Դ>({ .ȼ���� = m_u32ȼ����,.���λ = ���λ������������е�(),.���λ���� = ���λ����() });
}

uint16_t PlayerGateSession_Game::���λ����() const
{
	uint16_t result = 0;
	for (const auto& refEntity : m_setSpEntity)
	{
		if (!refEntity->m_spBuilding)
			continue;

		switch (refEntity->m_spBuilding->m_����)
		{
		case ��:result += 5; break;
		case ����:result += 2; break;
		default:break;
		}
	}
	return result;
}

uint16_t PlayerGateSession_Game::���λ������������е�() const
{
	uint16_t ��������еĵ�λ = 0;
	for (const auto& refEntity : m_setSpEntity)
	{
		if (refEntity->m_spBuilding)
		{
			��������еĵ�λ += refEntity->m_spBuilding->m_i�ȴ������;
			
		}
		else
		{
			++��������еĵ�λ;
		}
		
	}

	return ��������еĵ�λ;

}