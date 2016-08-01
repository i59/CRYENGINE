// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "GameRules.h"

CGameRules::CGameRules()
{
}

CGameRules::~CGameRules()
{
	gEnv->pGame->GetIGameFramework()->GetIGameRulesSystem()->SetCurrentGameRules(nullptr);
}

void CGameRules::PostInitialize()
{
	gEnv->pGame->GetIGameFramework()->GetIGameRulesSystem()->SetCurrentGameRules(this);
}

bool CGameRules::OnClientConnect(int channelId, bool isReset)
{
	const float fTerrainSize = static_cast<float>(gEnv->p3DEngine->GetTerrainSize());
	const float fTerrainElevation = gEnv->p3DEngine->GetTerrainElevation(fTerrainSize * 0.5f, fTerrainSize * 0.5f);
	const Vec3 vSpawnLocation(fTerrainSize * 0.5f, fTerrainSize * 0.5f, fTerrainElevation + 15.0f);

	IEntitySystem* pEntitySystem = gEnv->pEntitySystem;
	IEntityClass* pPlayerClass = pEntitySystem->GetClassRegistry()->FindClass("Player");

	if (!pPlayerClass)
		return false;

	SEntitySpawnParams params;
	params.sName = "Player";
	params.vPosition = vSpawnLocation;
	params.pClass = pPlayerClass;

	if (channelId)
	{
		params.nFlags |= ENTITY_FLAG_NEVER_NETWORK_STATIC;
		if (INetChannel* pNetChannel = gEnv->pGame->GetIGameFramework()->GetNetChannel(channelId))
		{
			if (pNetChannel->IsLocal())
			{
				params.id = LOCAL_PLAYER_ENTITY_ID;
			}
		}
	}

	if (IEntity *pPlayerEntity = pEntitySystem->SpawnEntity(params, false))
	{
		auto &playerGameObject = pPlayerEntity->AcquireExternalComponent<IGameObject>();

		// always set the channel id before initializing the entity
		playerGameObject.SetChannelId(channelId);

		return pEntitySystem->InitEntity(pPlayerEntity, params);
	}

	return false;
}

void CGameRules::GetMemoryUsage(ICrySizer* s) const
{
	s->Add(*this);
}
