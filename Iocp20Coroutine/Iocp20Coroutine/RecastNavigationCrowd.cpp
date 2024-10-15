#include "pch.h"
#include "../recastnavigation-main/DetourCrowd/Include/DetourCrowd.h"
#include "../recastnavigation-main/Detour/Include/DetourCommon.h"
#include "../recastnavigation-main/RecastDemo/Include/PerfTimer.h"
#include "CrowdTool.h"
#include "../recastnavigation-main/RecastDemo/Include/Sample_TempObstacles.h"
#include <unordered_map>
#include "Space.h"
#include "Entity.h"

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
int CrowToolAddAgent(float arrF[])
{
	return GetCrowdTool().addAgent(arrF);
}

std::unordered_map<int, uint64_t> m_mapEntityId;
extern Space g_Space无限刷怪;
void CrowToolUpdate()
{
	const float SIM_RATE = 20;
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

		unsigned int col = duRGBA(0,0,0,32);
		//if (m_agentDebug.idx == i)
		//	col = duRGBA(255,0,0,128);

		//duDebugDrawCircle(&dd, pos[0], pos[1], pos[2], radius, col, 2.0f);
		const auto idEntity = m_mapEntityId[i];
		auto spEntity = g_Space无限刷怪.GetEntity(idEntity);
		if (spEntity.expired())
		{
			LOG(WARNING) << "要删才行";
			continue;
		}
		auto sp = spEntity.lock();
		sp->m_Pos = { pos[0] ,pos[2] };
		
		sp->Broadcast(MsgNotifyPos(*sp));
	}
}

void CrowdToolSetMoveTarget(const float* p, bool adjust)
{
	GetCrowdTool().setMoveTarget(p, adjust);
}