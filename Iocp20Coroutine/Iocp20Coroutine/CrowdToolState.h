#pragma once

class Sample_TempObstacles;

class CrowdToolState
{
public:
	void addAgent(const float* pos);

	Sample_TempObstacles* m_sample;
};

