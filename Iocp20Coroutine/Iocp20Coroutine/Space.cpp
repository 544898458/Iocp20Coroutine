#include "Space.h"
#include "Entity.h"

Space g_space;

void Space::Update()
{
	for (auto& kv : mapEntity)
	{
		kv.second->Update();
	}
}
