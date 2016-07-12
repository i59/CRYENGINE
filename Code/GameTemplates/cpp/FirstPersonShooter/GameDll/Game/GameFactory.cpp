// CryEngine Source File.
// Copyright (C), Crytek, 1999-2016.


#include "StdAfx.h"
#include "Game/GameRules.h"
#include "Game/GameFactory.h"

#include "Entities/Helpers/NativeEntityPropertyHandling.h"

#include "FlowNodes/Helpers/FlowGameEntityNode.h"

std::map<string, CGameEntityNodeFactory*> CGameFactory::s_flowNodeFactories;

IEntityRegistrator *IEntityRegistrator::g_pFirst = nullptr;
IEntityRegistrator *IEntityRegistrator::g_pLast = nullptr;

void CGameFactory::Init()
{
	if(IEntityRegistrator::g_pFirst != nullptr)
	{
		do
		{
			IEntityRegistrator::g_pFirst->Register();

			IEntityRegistrator::g_pFirst = IEntityRegistrator::g_pFirst->m_pNext;
		}
		while(IEntityRegistrator::g_pFirst != nullptr);
	}
}

void CGameFactory::RegisterEntityFlowNodes()
{
	IFlowSystem* pFlowSystem = gEnv->pGame->GetIGameFramework()->GetIFlowSystem();
	std::map<string, CGameEntityNodeFactory*>::iterator it = s_flowNodeFactories.begin(), end = s_flowNodeFactories.end();
	for(; it != end; ++it)
	{
		pFlowSystem->RegisterType(it->first.c_str(), it->second);
	}

	stl::free_container(s_flowNodeFactories);
}

CGameEntityNodeFactory &CGameFactory::RegisterEntityFlowNode(const char *className)
{
	CGameEntityNodeFactory* pNodeFactory = new CGameEntityNodeFactory();
	
	string nodeName = "entity:";
	nodeName += className;

	s_flowNodeFactories[nodeName] = pNodeFactory;

	return *pNodeFactory;
}