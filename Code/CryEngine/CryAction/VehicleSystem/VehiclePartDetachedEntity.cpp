// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Description: Implements an entity class for detached parts

   -------------------------------------------------------------------------
   History:
   - 26:10:2005: Created by Mathieu Pinard

*************************************************************************/
#include "StdAfx.h"

#include <IViewSystem.h>
#include <IItemSystem.h>
#include <IVehicleSystem.h>
#include <CryPhysics/IPhysics.h>
#include <CryAnimation/ICryAnimation.h>
#include <IActorSystem.h>
#include <CryNetwork/ISerialize.h>
#include <CryAISystem/IAgent.h>

#include "CryAction.h"
#include "Vehicle.h"
#include "VehiclePartDetachedEntity.h"
#include "VehiclePartBase.h"

//------------------------------------------------------------------------
void CVehiclePartDetachedEntity::PostInitialize()
{
	InitVehiclePart();

	EnableEvent(ENTITY_EVENT_RESET, 0, true);
	EnableEvent(ENTITY_EVENT_COLLISION, 0, true);
}

//------------------------------------------------------------------------
void CVehiclePartDetachedEntity::InitVehiclePart()
{
	GetEntity()->SetUpdatePolicy(EEntityUpdatePolicy_Visible | EEntityUpdatePolicy_InRange);

	if (IPhysicalEntity* pPhysics = GetEntity()->GetPhysics())
	{
		pe_params_flags physFlags;

		if (pPhysics->GetParams(&physFlags))
		{
			physFlags.flags |= pef_log_collisions;
			pPhysics->SetParams(&physFlags);
		}
	}

	m_timeUntilStartIsOver = 10.0f;
}

//------------------------------------------------------------------------
void CVehiclePartDetachedEntity::Update(SEntityUpdateContext& ctx)
{
	if (m_timeUntilStartIsOver > 0.0f)
		m_timeUntilStartIsOver -= ctx.fFrameTime;
}

//------------------------------------------------------------------------
void CVehiclePartDetachedEntity::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
		case ENTITY_EVENT_RESET:
		{
			gEnv->pEntitySystem->RemoveEntity(GetEntity()->GetId());
		}
		break;
		case ENTITY_EVENT_COLLISION:
		{
			if (m_timeUntilStartIsOver <= 0.0f)
			{
				const EventPhysCollision* pCollision = (EventPhysCollision *)event.nParam[0];
				IEntitySystem* pES = gEnv->pEntitySystem;
				IEntity* pE1 = pES->GetEntityFromPhysics(pCollision->pEntity[0]);
				IEntity* pE2 = pES->GetEntityFromPhysics(pCollision->pEntity[1]);
				IEntity* pCollider = pE1 == GetEntity() ? pE2 : pE1;

				if (pCollider)
				{
					//OnCollision( pCollider->GetId(), pCollision->pt, pCollision->n );
					IEntity* pEntity = GetEntity();
					CRY_ASSERT(pEntity);

					int slotCount = pEntity->GetSlotCount();
					if (slotCount > 1)
					{
						for (int i = 1; i < slotCount; i++)
						{
							if (pEntity->IsSlotValid(i))
								pEntity->FreeSlot(i);
						}
					}
				}
			}
		}
		break;
	}
}

void CVehiclePartDetachedEntity::Serialize(TSerialize ser)
{
	bool hasGeom = false;
	if (ser.IsWriting())
	{
		if (GetEntity()->GetStatObj(0))
			hasGeom = true;
	}
	ser.Value("hasGeometry", hasGeom);
	if (hasGeom)
	{
		if (ser.IsWriting())
		{
			gEnv->p3DEngine->SaveStatObj(GetEntity()->GetStatObj(0), ser);
			if (GetEntity()->GetPhysics())
				GetEntity()->GetPhysics()->GetStateSnapshot(ser);
		}
		else if (ser.IsReading())
		{
			IStatObj* pStatObj = gEnv->p3DEngine->LoadStatObj(ser);
			if (pStatObj)
			{
				GetEntity()->SetStatObj(pStatObj, 0, true, 200.0f);
				SEntityPhysicalizeParams physicsParams;
				if (!pStatObj->GetPhysicalProperties(physicsParams.mass, physicsParams.density))
					physicsParams.mass = 200.0f;
				physicsParams.type = PE_RIGID;
				physicsParams.nFlagsOR &= pef_log_collisions;
				physicsParams.nSlot = 0;
				GetEntity()->Physicalize(physicsParams);
				if (GetEntity()->GetPhysics())
					GetEntity()->GetPhysics()->SetStateFromSnapshot(ser);
			}
		}
	}
}
