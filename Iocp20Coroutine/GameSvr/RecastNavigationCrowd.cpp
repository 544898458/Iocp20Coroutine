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
#include "RecastNavigationCrowd.h"
#include "AttackComponent.h"
#include "MyMsgQueue.h"

std::shared_ptr<CrowdToolState> CreateCrowdToolState()
{
	auto sp = std::make_shared<CrowdToolState>();
	auto pSample = new Sample_TempObstacles();
	pSample->loadAll("all_tiles_tilecache.bin");
	pSample->m_navQuery->init(pSample->m_navMesh, 2048);
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


uint32_t CrowToolAdd方块阻挡(CrowdToolState& ref, float arrF[], float f半边长)
{
	auto* pSample_TempObstacles = dynamic_cast<Sample_TempObstacles*>(ref.m_sample);
	float arrMin[] = { arrF[0] - f半边长 , arrF[1] - f半边长 , arrF[2] - f半边长 };
	float arrMax[] = { arrF[0] + f半边长 , arrF[1] + f半边长 , arrF[2] + f半边长 };
	uint32_t u32Ret(0);
	const auto result = pSample_TempObstacles->m_tileCache->addBoxObstacle(arrMin, arrMax, &u32Ret);
	assert(DT_SUCCESS == result);
	return u32Ret;
}

bool CrowToolRemove阻挡(CrowdToolState& ref, const uint32_t u32DtObstacleRef)
{
	auto* pSample_TempObstacles = dynamic_cast<Sample_TempObstacles*>(ref.m_sample);
	const auto result = pSample_TempObstacles->m_tileCache->removeObstacle(u32DtObstacleRef);
	const auto ret = DT_SUCCESS == result;
	assert(ret);
	return ret;
}

void CrowToolUpdate(Space& ref)
{
	const float SIM_RATE = 10;
	const float DELTA_TIME = 1.0f / SIM_RATE;
	ref.m_spCrowdToolState->handleUpdate(DELTA_TIME);
	auto m_sample = ref.m_spCrowdToolState->m_sample;
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

		unsigned int col = duRGBA(0, 0, 0, 32);
		//if (m_agentDebug.idx == i)
		//	col = duRGBA(255,0,0,128);

		//duDebugDrawCircle(&dd, pos[0], pos[1], pos[2], radius, col, 2.0f);
		const auto idEntity = ref.m_mapEntityId[i];
		auto spEntity = ref.GetEntity(idEntity);
		if (spEntity.expired())
		{
			LOG(WARNING) << "要删才行";
			//assert(false);
			continue;
		}
		auto sp = spEntity.lock();
		sp->m_Pos = { pos[0] ,pos[2] };

		//sp->Broadcast(MsgNotifyPos(*sp));
	}
}

void CrowdToolSetMoveTarget(CrowdToolState& ref, const float* p, const int idx)
{
	ref.m_agentDebug.idx = idx;
	ref.setMoveTarget(p, false);
}

bool CrowdTool可站立(CrowdToolState& refCrowTool, const Position& refPos)
{
	dtNavMeshQuery* navquery = refCrowTool.m_sample->getNavMeshQuery();
	CHECK_NOTNULL_RET_FALSE(navquery);

	dtQueryFilter filter;
	const float* halfExtents = refCrowTool.m_sample->m_crowd->getQueryExtents();
	float tgt[3];
	dtPolyRef ref;
	float p[] = { refPos.x,0,refPos.z };
	dtVcopy(tgt, p);
	if (DT_SUCCESS != navquery->findNearestPoly(p, halfExtents, &filter, &ref, tgt))
		return false;
	Position posNew = { tgt[0],tgt[2] };
	return refPos.DistanceLessEqual(posNew, 0.1f);
	//return true;
}

bool CrowdTool判断单位重叠(const Position& refPosOld, const Position& refPosNew, const float f半边长)
{
	const float arrMinNew[] = { refPosNew.x - f半边长,0,refPosNew.z - f半边长 };
	const float arrMaxNew[] = { refPosNew.x + f半边长,0,refPosNew.z + f半边长 };
	const float arrMinOld[] = { refPosOld.x - f半边长,0,refPosOld.z - f半边长 };
	const float arrMaxOld[] = { refPosOld.x + f半边长,0,refPosOld.z + f半边长 };
	return dtOverlapBounds(arrMinOld, arrMaxOld, arrMinNew, arrMaxNew);
}

RecastNavigationCrowd::RecastNavigationCrowd(Entity& refEntity, const Position& posTarget) :m_refEntity(refEntity)
{
	float arrF[] = { refEntity.m_Pos.x,0,refEntity.m_Pos.z };
	assert(AttackComponent::INVALID_AGENT_IDX == refEntity.m_spAttack->m_idxCrowdAgent);
	refEntity.m_spAttack->m_idxCrowdAgent = CrowToolAddAgent(*refEntity.m_refSpace.m_spCrowdToolState, arrF, refEntity.m_速度每帧移动距离 * 10);
	assert(AttackComponent::INVALID_AGENT_IDX != m_refEntity.m_spAttack->m_idxCrowdAgent);
	refEntity.m_refSpace.m_mapEntityId[refEntity.m_spAttack->m_idxCrowdAgent] = refEntity.Id;

	SetMoveTarget(posTarget);
}

RecastNavigationCrowd::~RecastNavigationCrowd()
{
	CrowToolRemoveAgent(*m_refEntity.m_refSpace.m_spCrowdToolState, m_refEntity.m_spAttack->m_idxCrowdAgent);
	m_refEntity.m_spAttack->m_idxCrowdAgent = AttackComponent::INVALID_AGENT_IDX;
}

void RecastNavigationCrowd::SetMoveTarget(const Position& posTarget)
{
	float arrF[] = { posTarget.x,0,posTarget.z };
	assert(AttackComponent::INVALID_AGENT_IDX != m_refEntity.m_spAttack->m_idxCrowdAgent);
	if (AttackComponent::INVALID_AGENT_IDX == m_refEntity.m_spAttack->m_idxCrowdAgent)
	{
		LOG(ERROR) << "可能超过容量";
	}
	CrowdToolSetMoveTarget(*m_refEntity.m_refSpace.m_spCrowdToolState, arrF, m_refEntity.m_spAttack->m_idxCrowdAgent);
}

