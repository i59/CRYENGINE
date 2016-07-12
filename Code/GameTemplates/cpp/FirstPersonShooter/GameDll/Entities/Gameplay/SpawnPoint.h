#pragma once

#include "Entities/Helpers/NativeEntityBase.h"

////////////////////////////////////////////////////////
// Entity responsible for spawning other entities
////////////////////////////////////////////////////////
class CSpawnPoint : public CGameObjectExtensionHelper<CSpawnPoint, CNativeEntityBase>
{
	// Indices of the Flowgraph node input ports for this entity
	enum EFlowgraphInputPorts
	{
		eInputPorts_Spawn,
		eInputPorts_EntityId
	};

	// Indices of the Flowgraph node output ports for this entity
	enum EFlowgraphOutputPorts
	{
		eOutputPorts_OnSpawned,
	};

public:
	CSpawnPoint();
	virtual ~CSpawnPoint();

	// CNativeEntityBase
	virtual bool Init(IGameObject *pGameObject) override;
	virtual void Release() override;
	// ~CNativeEntityBase

	void SpawnEntity(IEntity &otherEntity);
	
public:
	// Called when one of the input Flowgraph ports are activated in one of the entity instances tied to this class
	static void OnFlowgraphActivation(EntityId entityId, IFlowNode::SActivationInfo* pActInfo, const class CFlowGameEntityNode *pNode);
};