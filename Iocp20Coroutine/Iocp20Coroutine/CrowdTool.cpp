#include "pch.h"
#include "../recastnavigation-main/DetourCrowd/Include/DetourCrowd.h"
#include "../recastnavigation-main/Detour/Include/DetourCommon.h"
#include "../recastnavigation-main/RecastDemo/Include/PerfTimer.h"
#include "CrowdTool.h"
#include "Sample_TempObstacles.h"


void CrowdTool::handleUpdate(const float dt)
{
	if (m_run)
		updateTick(dt);
}

void CrowdTool::addAgent(const float* p)
{
	if (!m_sample) return;
	dtCrowd* crowd = m_sample->getCrowd();

	dtCrowdAgentParams ap;
	memset(&ap, 0, sizeof(ap));
	ap.radius = m_sample->getAgentRadius();
	ap.height = m_sample->getAgentHeight();
	ap.maxAcceleration = 8.0f;
	ap.maxSpeed = 3.5f;
	ap.collisionQueryRange = ap.radius * 12.0f;
	ap.pathOptimizationRange = ap.radius * 30.0f;
	ap.updateFlags = 0;
	//if (m_toolParams.m_anticipateTurns)
	ap.updateFlags |= DT_CROWD_ANTICIPATE_TURNS;
	//if (m_toolParams.m_optimizeVis)
	ap.updateFlags |= DT_CROWD_OPTIMIZE_VIS;
	//if (m_toolParams.m_optimizeTopo)
	ap.updateFlags |= DT_CROWD_OPTIMIZE_TOPO;
	//if (m_toolParams.m_obstacleAvoidance)
	ap.updateFlags |= DT_CROWD_OBSTACLE_AVOIDANCE;
	//if (m_toolParams.m_separation)
	ap.updateFlags |= DT_CROWD_SEPARATION;
	ap.obstacleAvoidanceType = 3;// (unsigned char)m_toolParams.m_obstacleAvoidanceType;
	ap.separationWeight = 2.00f;// m_toolParams.m_separationWeight;

	int idx = crowd->addAgent(p, &ap);
	if (idx != -1)
	{
		//if (m_targetRef)
		//	crowd->requestMoveTarget(idx, m_targetRef, m_targetPos);

		//// Init trail
		//AgentTrail* trail = &m_trails[idx];
		//for (int i = 0; i < AGENT_MAX_TRAIL; ++i)
		//	dtVcopy(&trail->trail[i * 3], p);
		//trail->htrail = 0;
	}
}

void CrowdTool::updateTick(const float dt)
{
	if (!m_sample) return;
	dtNavMesh* nav = m_sample->getNavMesh();
	dtCrowd* crowd = m_sample->getCrowd();
	if (!nav || !crowd) return;

	TimeVal startTime = getPerfTime();

	crowd->update(dt, &m_agentDebug);

	TimeVal endTime = getPerfTime();

	// Update agent trails
	//for (int i = 0; i < crowd->getAgentCount(); ++i)
	//{
	//	const dtCrowdAgent* ag = crowd->getAgent(i);
	//	AgentTrail* trail = &m_trails[i];
	//	if (!ag->active)
	//		continue;
	//	// Update agent movement trail.
	//	trail->htrail = (trail->htrail + 1) % AGENT_MAX_TRAIL;
	//	dtVcopy(&trail->trail[trail->htrail * 3], ag->npos);
	//}

	//m_agentDebug.vod->normalizeSamples();

	//m_crowdSampleCount.addSample((float)crowd->getVelocitySampleCount());
	//m_crowdTotalTime.addSample(getPerfTimeUsec(endTime - startTime) / 1000.0f);
}

static void calcVel(float* vel, const float* pos, const float* tgt, const float speed)
{
	dtVsub(vel, tgt, pos);
	vel[1] = 0.0;
	dtVnormalize(vel);
	dtVscale(vel, vel, speed);
}

void CrowdTool::setMoveTarget(const float* p, bool adjust)
{
	if (!m_sample) return;

	// Find nearest point on navmesh and set move request to that location.
	dtNavMeshQuery* navquery = m_sample->getNavMeshQuery();
	dtCrowd* crowd = m_sample->getCrowd();
	const dtQueryFilter* filter = crowd->getFilter(0);
	const float* halfExtents = crowd->getQueryExtents();

	if (adjust)
	{
		float vel[3];
		// Request velocity
		if (m_agentDebug.idx != -1)
		{
			const dtCrowdAgent* ag = crowd->getAgent(m_agentDebug.idx);
			if (ag && ag->active)
			{
				calcVel(vel, ag->npos, p, ag->params.maxSpeed);
				crowd->requestMoveVelocity(m_agentDebug.idx, vel);
			}
		}
		else
		{
			for (int i = 0; i < crowd->getAgentCount(); ++i)
			{
				const dtCrowdAgent* ag = crowd->getAgent(i);
				if (!ag->active) continue;
				calcVel(vel, ag->npos, p, ag->params.maxSpeed);
				crowd->requestMoveVelocity(i, vel);
			}
		}
	}
	else
	{
		navquery->findNearestPoly(p, halfExtents, filter, &m_targetRef, m_targetPos);

		if (m_agentDebug.idx != -1)
		{
			const dtCrowdAgent* ag = crowd->getAgent(m_agentDebug.idx);
			if (ag && ag->active)
				crowd->requestMoveTarget(m_agentDebug.idx, m_targetRef, m_targetPos);
		}
		else
		{
			for (int i = 0; i < crowd->getAgentCount(); ++i)
			{
				const dtCrowdAgent* ag = crowd->getAgent(i);
				if (!ag->active) continue;
				crowd->requestMoveTarget(i, m_targetRef, m_targetPos);
			}
		}
	}
}
