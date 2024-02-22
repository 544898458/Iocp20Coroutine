#include "Space.h"
#include "Entity.h"

Space g_space;

void Space::Update()
{
	for (auto pEntity : setEntity)
	{
		pEntity->Update();
	}
}
