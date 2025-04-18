#include "pch.h"
#include "../recastnavigation-main/DetourCrowd/Include/DetourCrowd.h"
#include "../recastnavigation-main/Detour/Include/DetourCommon.h"
#include "../recastnavigation-main/Detour/Include/DetourStatus.h"
#include "../recastnavigation-main/DetourTileCache/Include/DetourTileCache.h"
#include "../recastnavigation-main/RecastDemo/Include/PerfTimer.h"
#include "CrowdTool.h"
#include "../recastnavigation-main/RecastDemo/Include/Sample_TempObstacles.h"

#include <unordered_map>
#include "Space.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "RecastNavigationCrowd.h"
#include "��λ���/��Component.h"
#include "MyMsgQueue.h"

std::shared_ptr<CrowdToolState> CreateCrowdToolState(const std::string& stfѰ·�ļ�)
{
	auto sp = std::make_shared<CrowdToolState>();
	auto pSample = new Sample_TempObstacles();
	pSample->m_agentRadius = 1.5f;
	pSample->loadAll(stfѰ·�ļ�.c_str());
	pSample->m_navQuery->init(pSample->m_navMesh, 1024 * 32);
	sp->init(pSample);
	return sp;
}
//
//CrowdToolState& GetCrowdTool()
//{
//	static CrowdToolState* p;
//	if (nullptr == p)
//	{
//		auto pSample = new Sample_TempObstacles();
//		pSample->loadAll("all_tiles_tilecache.bin");
//		pSample->m_navQuery->init(pSample->m_navMesh, 2048);
//		p = new CrowdToolState();
//		p->init(pSample);
//	}
//	return *p;
//}

int CrowToolAddAgent(CrowdToolState& ref, float arrF[], float fSpeed)
{
	return ref.addAgent(arrF, fSpeed);
}

void CrowToolRemoveAgent(CrowdToolState& ref, int idx)
{
	ref.removeAgent(idx);
}


uint32_t CrowToolAdd�����赲(CrowdToolState& ref, float arrF[], float f��߳�)
{
	auto* pSample_TempObstacles = dynamic_cast<Sample_TempObstacles*>(ref.m_sample);
	float arrMin[] = { arrF[0] - f��߳� , arrF[1] - f��߳� , arrF[2] - f��߳� };
	float arrMax[] = { arrF[0] + f��߳� , arrF[1] + f��߳� , arrF[2] + f��߳� };
	uint32_t u32Ret(0);
	const auto result = pSample_TempObstacles->m_tileCache->addBoxObstacle(arrMin, arrMax, &u32Ret);

	if (DT_SUCCESS != result)
	{
		LOG(ERROR) << "" << result;
		_ASSERT(false);
	}
	return u32Ret;
}

bool CrowToolRemove�赲(CrowdToolState& ref, const uint32_t u32DtObstacleRef)
{
	auto* pSample_TempObstacles = dynamic_cast<Sample_TempObstacles*>(ref.m_sample);
	const auto result = pSample_TempObstacles->m_tileCache->removeObstacle(u32DtObstacleRef);
	const auto ret = DT_SUCCESS == result;
	_ASSERT(ret);
	return ret;
}

void CrowToolUpdate(Space& ref, CrowdToolState& refCrowdToolState)
{
	const float SIM_RATE = 10;
	const float DELTA_TIME = 1.0f / SIM_RATE;
	refCrowdToolState.handleUpdate(DELTA_TIME);
	auto* m_sample = refCrowdToolState.m_sample;
	m_sample->handleUpdate(DELTA_TIME);
	duDebugDraw& dd = m_sample->getDebugDraw();
	const float rad = m_sample->getAgentRadius();

	dtNavMesh* nav = m_sample->getNavMesh();
	dtCrowd* crowd = m_sample->getCrowd();
	if (!nav || !crowd)
		return;

	// Agent cylinders.
	for (int i = 0; i < crowd->getAgentCount(); ++i)
	{
		const dtCrowdAgent* ag = crowd->getAgent(i);
		if (!ag->active) continue;

		const float radius = ag->params.radius;
		const float* pos = ag->npos;
		const float* vel = ag->vel;
		unsigned int col = duRGBA(0, 0, 0, 32);
		//if (m_agentDebug.idx == i)
		//	col = duRGBA(255,0,0,128);

		//duDebugDrawCircle(&dd, pos[0], pos[1], pos[2], radius, col, 2.0f);
		const auto idEntity = refCrowdToolState.m_mapEntityId[i];
		auto spEntity = ref.GetEntity(idEntity);
		if (spEntity.expired())
		{
			LOG(WARNING) << "Ҫɾ����";
			//_ASSERT(false);
			continue;
		}
		auto sp = spEntity.lock();
		sp->SetPos({ pos[0] ,pos[2] });
		sp->m_eulerAnglesY = CalculateAngle(vel[0], vel[2]);
		//sp->Broadcast(MsgNotifyPos(*sp));
	}
}

void CrowdToolSetMoveTarget(CrowdToolState& ref, const float* p, const int idx)
{
	ref.m_agentDebug.idx = idx;
	ref.setMoveTarget(p, false);
}

const float halfExtents[] = { 10, 5, 10 };//refCrowTool.m_sample->m_crowd->getQueryExtents();

bool CrowdToolFindNerestPos(CrowdToolState& refCrowTool, Position& refPos)
{
	dtNavMeshQuery* navquery = refCrowTool.m_sample->getNavMeshQuery();
	CHECK_NOTNULL_RET_FALSE(navquery);

	dtQueryFilter filter;

	float tgt[3] = { 0 };
	dtPolyRef ref;
	float p[] = { refPos.x,0,refPos.z };
	dtVcopy(tgt, p);
	if (DT_SUCCESS != navquery->findNearestPoly(p, halfExtents, &filter, &ref, tgt))
		return false;

	refPos = { tgt[0],tgt[2] };
	return true;
}

bool CrowdTool��վ��(CrowdToolState& refCrowTool, const Position& refPos)
{
	Position pos(refPos);
	if (!CrowdToolFindNerestPos(refCrowTool, pos))
		return false;

	return refPos.DistanceLessEqual(pos, 0.1f);
}

bool CrowdTool����ֱ��(CrowdToolState& refCrowTool, const Position& pos��ʼ, const Position& posĿ��)
{
	dtNavMeshQuery* navquery = refCrowTool.m_sample->getNavMeshQuery();
	CHECK_NOTNULL_RET_FALSE(navquery);

	float arr��ʼ��[] = { pos��ʼ.x, 0, pos��ʼ.z }, arrĿ���[] = { posĿ��.x, 0, posĿ��.z };
	float arr��ʼ���վ��[3] = { 0 }, arrĿ����վ��[3] = { 0 };
	dtQueryFilter filter;
	filter.setIncludeFlags(0xffff);
	filter.setExcludeFlags(0);
	dtPolyRef polyRef��ʼ, polyRefĿ��;
	bool bOverlay��ʼ�� = false, bOverlayĿ�� = false;
	navquery->findNearestPoly(arr��ʼ��, halfExtents, &filter, &polyRef��ʼ, arr��ʼ���վ��, &bOverlay��ʼ��);
	navquery->findNearestPoly(arrĿ���, halfExtents, &filter, &polyRefĿ��, arrĿ����վ��, &bOverlayĿ��);

	const int MAX_POLYS = 100;
	dtPolyRef polys[MAX_POLYS];
	int npolys;
	unsigned char straightPathFlags[MAX_POLYS];
	dtPolyRef straightPathPolys[MAX_POLYS];
	float straightPath[3 * MAX_POLYS];
	int nstraightPath = 0;

	navquery->findPath(polyRef��ʼ, polyRefĿ��, arr��ʼ���վ��, arrĿ����վ��, &filter, polys, &npolys, MAX_POLYS);

	if (!npolys)
		return false;

	float epos1[3];
	dtVcopy(epos1, arrĿ����վ��);

	if (polys[npolys - 1] != polyRefĿ��)
	{
		navquery->closestPointOnPoly(polys[npolys - 1], arrĿ����վ��, epos1, 0);
	}

	if (DT_SUCCESS != navquery->findStraightPath(arr��ʼ���վ��, arrĿ����վ��, polys, npolys, straightPath, straightPathFlags, straightPathPolys, &nstraightPath, MAX_POLYS, DT_STRAIGHTPATH_ALL_CROSSINGS))
		return false;

	float f��·���� = 0.f;
	for (int i = 0; i < nstraightPath - 1; ++i)
	{
		const Position pos = { straightPath[i * 3 + 0], straightPath[i * 3 + 2] };
		const int i��һ�� = i + 1;
		const Position pos��һ�� = { straightPath[i��һ�� * 3 + 0], straightPath[i��һ�� * 3 + 2] };
		f��·���� += pos.Distance(pos��һ��);
	}

	const float fֱ�߾��� = pos��ʼ.Distance({ arrĿ����վ��[0], arrĿ����վ��[2] });
	return fֱ�߾��� + 0.01f > f��·����;// refPos.DistanceLessEqual(pos, 0.1f);
}

bool CrowdTool�жϵ�λ�ص�(const Position& refPosOld, const Position& refPosNew, const float fNew��߳�)
{
	const float f��ͨ��λ��߳� = 0.5;
	const float arrMinNew[] = { refPosNew.x - fNew��߳�,refPosNew.z - fNew��߳� };
	const float arrMaxNew[] = { refPosNew.x + fNew��߳�,refPosNew.z + fNew��߳� };
	const float arrMinOld[] = { refPosOld.x - f��ͨ��λ��߳�,refPosOld.z - f��ͨ��λ��߳� };
	const float arrMaxOld[] = { refPosOld.x + f��ͨ��λ��߳�,refPosOld.z + f��ͨ��λ��߳� };
	return dtOverlapBounds(arrMinOld, arrMaxOld, arrMinNew, arrMaxNew);
}

RecastNavigationCrowd::RecastNavigationCrowd(Entity& refEntity, const Position& posTarget) :m_refEntity(refEntity)
{
	CHECK_RET_VOID(refEntity.m_sp��);
	float arrF[] = { refEntity.Pos().x,0,refEntity.Pos().z };
	_ASSERT(��Component::INVALID_AGENT_IDX == refEntity.m_sp��->m_idxCrowdAgent);
	auto& refCrowdToolState = refEntity.m_refSpace.GetCrowdToolState(refEntity.m_����);
	refEntity.m_sp��->m_idxCrowdAgent = CrowToolAddAgent(refCrowdToolState, arrF, EntitySystem::�������ٶ�ÿ���ƶ�����(refEntity));
	_ASSERT(��Component::INVALID_AGENT_IDX != m_refEntity.m_sp��->m_idxCrowdAgent);
	refCrowdToolState.m_mapEntityId[refEntity.m_sp��->m_idxCrowdAgent] = refEntity.Id;

	SetMoveTarget(posTarget);
}

RecastNavigationCrowd::~RecastNavigationCrowd()
{
	CrowToolRemoveAgent(m_refEntity.m_refSpace.GetCrowdToolState(m_refEntity.m_����), m_refEntity.m_sp��->m_idxCrowdAgent);
	m_refEntity.m_sp��->m_idxCrowdAgent = ��Component::INVALID_AGENT_IDX;
}

void RecastNavigationCrowd::SetMoveTarget(const Position& posTarget)
{
	float arrF[] = { posTarget.x,0,posTarget.z };
	_ASSERT(��Component::INVALID_AGENT_IDX != m_refEntity.m_sp��->m_idxCrowdAgent);
	if (��Component::INVALID_AGENT_IDX == m_refEntity.m_sp��->m_idxCrowdAgent)
	{
		LOG(ERROR) << "���ܳ�������";
	}
	CrowdToolSetMoveTarget(m_refEntity.m_refSpace.GetCrowdToolState(m_refEntity.m_����), arrF, m_refEntity.m_sp��->m_idxCrowdAgent);
}

void RecastNavigationCrowd::SetSpeed()
{
	m_refEntity.m_refSpace.GetCrowdToolState(m_refEntity.m_����).SetSpeed(m_refEntity.m_sp��->m_idxCrowdAgent, EntitySystem::�������ٶ�ÿ֡�ƶ�����(m_refEntity));
}

