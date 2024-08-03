#include "StdAfx.h"
#include "Space.h"
#include "Entity.h"

void Space::Update()
{
	for (auto pEntity : setEntity)
	{
		pEntity->Update();
	}
}
