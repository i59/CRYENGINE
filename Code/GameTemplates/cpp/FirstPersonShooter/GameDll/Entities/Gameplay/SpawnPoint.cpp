#include "StdAfx.h"
#include "SpawnPoint.h"

#include "Game/GameFactory.h"
#include "FlowNodes/Helpers/FlowGameEntityNode.h"

#include "Player/Player.h"

class CSpawnPointRegistrator
	: public IEntityRegistrator
{
	virtual void Register() override
	{
		CGameFactory::RegisterNativeEntity<CSpawnPoint>("SpawnPoint", "Gameplay", "");

		// Create spawner flownode
		CGameEntityNodeFactory &nodeFactory = CGameFactory::RegisterEntityFlowNode("SpawnPoint");

		// Define input ports, and the callback function for when they are triggered
		std::vector<SInputPortConfig> inputs;
		inputs.push_back(InputPortConfig_Void("Spawn", "Spawns the entity"));
		inputs.push_back(InputPortConfig<EntityId>("SpawnEntityId", _HELP("Identifier of the entity to be spawned"))),
		nodeFactory.AddInputs(inputs, CSpawnPoint::OnFlowgraphActivation);

		// Define output ports
		std::vector<SOutputPortConfig> outputs;
		outputs.push_back(OutputPortConfig_Void("OnSpawned"));
		nodeFactory.AddOutputs(outputs);

		// Mark the factory as complete, indicating that there will be no additional ports
		nodeFactory.Close();
	}
};

CSpawnPointRegistrator g_spawnerRegistrator;

bool CSpawnPoint::Init(IGameObject* pGameObject)
{
	SetGameObject(pGameObject);
	return pGameObject->BindToNetwork();
}

void CSpawnPoint::Release()
{
	ISimpleExtension::Release();
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

void CSpawnPoint::OnFlowgraphActivation(EntityId entityId, IFlowNode::SActivationInfo *pActInfo, const class CFlowGameEntityNode *pNode)
{
	if (auto *pSpawner = static_cast<CSpawnPoint *>(QueryExtension(entityId)))
	{
		if (IsPortActive(pActInfo, eInputPorts_Spawn))
		{
			CRY_ASSERT(gEnv->bServer);

			if(IEntity *pOtherEntity = gEnv->pEntitySystem->GetEntity(GetPortEntityId(pActInfo, eInputPorts_EntityId)))
			{
				pSpawner->SpawnEntity(*pOtherEntity);

				ActivateOutputPort(entityId, eOutputPorts_OnSpawned, TFlowInputData());
			}
		}
	}
}