#include "Space.h"
#include "Entity.h"
void Space::Update()
{
	for (auto& kv : mapEntity)
	{
		kv.second->Update();
	}
}
