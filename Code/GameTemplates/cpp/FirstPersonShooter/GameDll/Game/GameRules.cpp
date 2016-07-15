// CryEngine Source File.
// Copyright (C), Crytek, 1999-2016.


#include "StdAfx.h"
#include "GameRules.h"
#include <GameObjects/GameObject.h>

#include "Game/GameFactory.h"

#include "Player/Player.h"
#include "Entities/Gameplay/SpawnPoint.h"

#include "FlowNodes/Helpers/FlowGameEntityNode.h"

#include <IActorSystem.h>

class CRulesRegistrator
	: public IEntityRegistrator
{
	virtual void Register() override
	{
		CGameFactory::RegisterGameObject<CGameRules>("GameRules", "", 0);

		IGameFramework* pGameFramework = gEnv->pGame->GetIGameFramework();

		pGameFramework->GetIGameRulesSystem()->RegisterGameRules(GAMERULES_NAME, "GameRules");
		pGameFramework->GetIGameRulesSystem()->AddGameRulesAlias(GAMERULES_NAME, GAMERULES_NAME_SHORT);
	}
};

CRulesRegistrator g_gameRulesRegistrator;

CGameRules::CGameRules()
{
}

CGameRules::~CGameRules()
{
	gEnv->pGame->GetIGameFramework()->GetIGameRulesSystem()->SetCurrentGameRules(nullptr);
}

bool CGameRules::Init(IGameObject *pGameObject)
{
	SetGameObject(pGameObject);

	if (!pGameObject->BindToNetwork())
		return false;

	gEnv->pGame->GetIGameFramework()->GetIGameRulesSystem()->SetCurrentGameRules(this);

	return true;
}

bool CGameRules::OnClientConnect(int channelId, bool isReset)
{
	auto *pActorSystem = gEnv->pGame->GetIGameFramework()->GetIActorSystem();
	
	return pActorSystem->CreateActor(channelId, "DefaultPlayer", "Player", ZERO, IDENTITY, Vec3(1, 1, 1)) != nullptr;
}

void CGameRules::OnClientDisconnect(int channelId, EDisconnectionCause cause, const char *desc, bool keepClient)
{
	auto *pActorSystem = gEnv->pGame->GetIGameFramework()->GetIActorSystem();
	if(IActor *pActor = pActorSystem->GetActorByChannelId(channelId))
	{
		pActorSystem->RemoveActor(pActor->GetEntityId());
	}
}

bool CGameRules::OnClientEnteredGame(int channelId, bool isReset)
{
	// We only handle default spawning below for the Launcher
	// Editor has special logic in CEditorGame
	if (gEnv->IsEditor())
		return true;

	auto *pActorSystem = gEnv->pGame->GetIGameFramework()->GetIActorSystem();

	auto *pActor = pActorSystem->GetActorByChannelId(channelId);
	if(pActor != nullptr)
	{
		// Spawn at first default spawner
		auto *pEntityIterator = gEnv->pEntitySystem->GetEntityIterator();
		pEntityIterator->MoveFirst();

		auto *pSpawnerClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("SpawnPoint");
		auto extensionId = gEnv->pGame->GetIGameFramework()->GetIGameObjectSystem()->GetID("SpawnPoint");

		while (!pEntityIterator->IsEnd())
		{
			IEntity *pEntity = pEntityIterator->Next();
			if(pEntity->GetClass() != pSpawnerClass)
				continue;

			auto *pGameObject = gEnv->pGame->GetIGameFramework()->GetGameObject(pEntity->GetId());
			if(pGameObject == nullptr)
				continue;

			auto *pSpawner = static_cast<CSpawnPoint *>(pGameObject->QueryExtension(extensionId));
			if(pSpawner == nullptr)
				continue;

			pSpawner->SpawnEntity(*pActor->GetEntity());

			break;
		}
	}

	return true;
}

void CGameRules::GetMemoryUsage(ICrySizer* s) const
{
	s->Add(*this);
}