// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "game/GameRules.h"
#include "game/GameFactory.h"
#include "player/Player.h"
#include "player/extensions/ViewExtension.h"
#include "player/extensions/InputExtension.h"
#include "player/extensions/MovementExtension.h"
#include "entities/GeomEntity.h"
#include "flownodes/FlowGameEntityNode.h"

std::map<string, CGameEntityNodeFactory*> CGameFactory::s_flowNodeFactories;

void CGameFactory::Init()
{
	RegisterEntityWithComponent<CPlayer>("Player", ECLF_INVISIBLE);
	RegisterNoScriptGameObject<CGeomEntity>("GameGeomEntity", "Game/GeomEntity");

	RegisterExternalComponent<CGameRules>();

	IGameFramework* pGameFramework = gEnv->pGame->GetIGameFramework();
	pGameFramework->GetIGameRulesSystem()->RegisterGameRules("SinglePlayer", CGameRules::IID());
	pGameFramework->GetIGameRulesSystem()->AddGameRulesAlias("SinglePlayer", "sp");
}

void CGameFactory::RegisterEntityFlowNodes()
{
	IFlowSystem* pFlowSystem = gEnv->pGame->GetIGameFramework()->GetIFlowSystem();
	std::map<string, CGameEntityNodeFactory*>::iterator it = s_flowNodeFactories.begin(), end = s_flowNodeFactories.end();
	for (; it != end; ++it)
	{
		pFlowSystem->RegisterType(it->first.c_str(), it->second);
	}

	stl::free_container(s_flowNodeFactories);
}

void CGameFactory::CreateScriptTables(SEntityScriptProperties& out)
{
	out.pEntityTable = gEnv->pScriptSystem->CreateTable();
	out.pEntityTable->AddRef();

	out.pEditorTable = gEnv->pScriptSystem->CreateTable();
	out.pEditorTable->AddRef();

	out.pPropertiesTable = gEnv->pScriptSystem->CreateTable();
	out.pPropertiesTable->AddRef();

	/*if (flags & eGORF_InstanceProperties)
	{
		out.pInstancePropertiesTable = gEnv->pScriptSystem->CreateTable();
		out.pInstancePropertiesTable->AddRef();
	}*/
}

template <typename T>
static void CreateComponent(IEntity& entity, SEntitySpawnParams& params, void* pUserData)
{
	// Acquire a new instance of T
	entity.AcquireComponent<T>();
}

template<class T>
void CGameFactory::RegisterNoScriptGameObject(const string& name, const string& editorPath)
{
	SEntityScriptProperties props;
	CGameFactory::CreateScriptTables(props);
	gEnv->pScriptSystem->SetGlobalValue(name.c_str(), props.pEntityTable);

	CGameEntityNodeFactory* pNodeFactory = new CGameEntityNodeFactory();
	T::RegisterProperties(props, pNodeFactory);

	if (!editorPath.empty())
	{
		props.pEditorTable->SetValue("EditorPath", editorPath.c_str());
	}

	props.pEntityTable->SetValue("Editor", props.pEditorTable);
	props.pEntityTable->SetValue("Properties", props.pPropertiesTable);

	/*if (flags & eGORF_InstanceProperties)
	{
		props.pEntityTable->SetValue("PropertiesInstance", props.pInstancePropertiesTable);
	}*/

	string nodeName = "entity:";
	nodeName += name.c_str();
	pNodeFactory->Close();
	s_flowNodeFactories[nodeName] = pNodeFactory;

	IEntityClassRegistry::SEntityClassDesc clsDesc;
	clsDesc.sName = name.c_str();
	clsDesc.pScriptTable = props.pEntityTable;

	clsDesc.pEntitySpawnCallback = CreateComponent<T>;
	gEnv->pEntitySystem->GetClassRegistry()->RegisterStdClass(clsDesc);
}
