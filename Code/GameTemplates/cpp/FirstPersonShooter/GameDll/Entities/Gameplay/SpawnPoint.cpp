#include "StdAfx.h"
#include "SpawnPoint.h"

#include "Game/GameFactory.h"

#include "Player/Player.h"

class CSpawnPointRegistrator
	: public IEntityRegistrator
{
	virtual void Register() override
	{
		CGameFactory::RegisterNativeEntity<CSpawnPoint>("SpawnPoint", "Gameplay", "");
	}
};

CSpawnPointRegistrator g_spawnerRegistrator;

bool CSpawnPoint::Init(IGameObject* pGameObject)
{
	SetGameObject(pGameObject);
	return pGameObject->BindToNetwork();
}

void CSpawnPoint::SpawnEntity(IEntity &otherEntity)
{
	otherEntity.SetWorldTM(GetEntity()->GetWorldTM());

	// Special behavior for players, notify respawn
	if(!strcmp(otherEntity.GetClass()->GetName(), "Player") && !gEnv->IsEditing())
	{
		if(IGameObject *pGameObject = gEnv->pGame->GetIGameFramework()->GetGameObject(otherEntity.GetId()))
		{
			if(auto *pPlayer = static_cast<CPlayer *>(pGameObject->QueryExtension("Player")))
			{
				// Revive the player
				pPlayer->SetHealth(100.f);
			}
		}
	}
}