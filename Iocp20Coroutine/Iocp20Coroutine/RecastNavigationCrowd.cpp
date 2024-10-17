#include "pch.h"
#include "../recastnavigation-main/DetourCrowd/Include/DetourCrowd.h"
#include "../recastnavigation-main/Detour/Include/DetourCommon.h"
#include "../recastnavigation-main/DetourTileCache/Include/DetourTileCache.h"
#include "../recastnavigation-main/RecastDemo/Include/PerfTimer.h"
#include "CrowdTool.h"
#include "../recastnavigation-main/RecastDemo/Include/Sample_TempObstacles.h"

#include <unordered_map>
#include "Space.h"
#include "Entity.h"
#include "RecastNavigationCrowd.h"
#include "AttackComponent.h"

CrowdToolState& GetCrowdTool()
{
	static CrowdToolState* p;
	if (nullptr == p)
	{
		auto pSample = new Sample_TempObstacles();
		pSample->loadAll("all_tiles_tilecache.bin");
		pSample->m_navQuery->init(pSample->m_navMesh, 2048);
		p = new CrowdToolState();
		p->init(pSample);
	}
	return *p;
}

int CrowToolAddAgent(float arrF[], float fSpeed)
{
	return GetCrowdTool().addAgent(arrF, fSpeed);
}

void CrowToolRemoveAgent(int idx)
{
	GetCrowdTool().removeAgent(idx);
}


uint32_t CrowToolAdd方块阻挡(float arrF[], float f半边长)
{
	auto* pSample_TempObstacles = dynamic_cast<Sample_TempObstacles*>(GetCrowdTool().m_sample);
	float arrMin[] = { arrF[0] - f半边长 , arrF[1] - f半边长 , arrF[2] - f半边长 };
	float arrMax[] = { arrF[0] + f半边长 , arrF[1] + f半边长 , arrF[2] + f半边长 };
	uint32_t u32Ret(0);
	const auto result = pSample_TempObstacles->m_tileCache->addBoxObstacle(arrMin, arrMax, &u32Ret);
	assert(DT_SUCCESS == result);
	return u32Ret;
}


std::unordered_map<int, uint64_t> m_mapEntityId;
extern Space g_Space无限刷怪;
void CrowToolUpdate()
{
	const float SIM_RATE = 10;
	const float DELTA_TIME = 1.0f / SIM_RATE;
	GetCrowdTool().handleUpdate(DELTA_TIME);
	auto m_sample = GetCrowdTool().m_sample;

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
		const auto idEntity = m_mapEntityId[i];
		auto spEntity = g_Space无限刷怪.GetEntity(idEntity);
		if (spEntity.expired())
		{
			LOG(WARNING) << "要删才行";
			assert(false);
			continue;
		}
		auto sp = spEntity.lock();
		sp->m_Pos = { pos[0] ,pos[2] };

		//sp->Broadcast(MsgNotifyPos(*sp));
	}
}

void CrowdToolSetMoveTarget(const float* p, const int idx)
{
	GetCrowdTool().m_agentDebug.idx = idx;
	GetCrowdTool().setMoveTarget(p, false);
}

RecastNavigationCrowd::RecastNavigationCrowd(Entity& refEntity, const Position& posTarget) :m_refEntity(refEntity)
{
	float arrF[] = { refEntity.m_Pos.x,0,refEntity.m_Pos.z };
	assert(AttackComponent::INVALID_AGENT_IDX == refEntity.m_spAttack->m_idxCrowdAgent);
	refEntity.m_spAttack->m_idxCrowdAgent = CrowToolAddAgent(arrF, refEntity.m_速度每帧移动距离 * 10);
	assert(AttackComponent::INVALID_AGENT_IDX != m_refEntity.m_spAttack->m_idxCrowdAgent);
	m_mapEntityId[refEntity.m_spAttack->m_idxCrowdAgent] = refEntity.Id;

	SetMoveTarget(posTarget);
}

RecastNavigationCrowd::~RecastNavigationCrowd()
{
	CrowToolRemoveAgent(m_refEntity.m_spAttack->m_idxCrowdAgent);
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
	CrowdToolSetMoveTarget(arrF, m_refEntity.m_spAttack->m_idxCrowdAgent);
}
