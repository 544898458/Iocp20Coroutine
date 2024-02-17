#include "Space.h"
#include "Entity.h"

Space space;

void Space::Update()
{
	for (auto& kv : mapEntity)
	{
		kv.second->Update();
	}
}
