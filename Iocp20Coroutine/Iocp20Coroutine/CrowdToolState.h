#pragma once

class Sample_TempObstacles;

class CrowdToolState
{
public:
	void addAgent(const float* pos);
	void handleUpdate(const float dt);
	void updateTick(const float dt);

	Sample_TempObstacles* m_sample;
	bool m_run = true;

	dtCrowdAgentDebugInfo m_agentDebug;
	dtObstacleAvoidanceDebugData* m_vod;

};

