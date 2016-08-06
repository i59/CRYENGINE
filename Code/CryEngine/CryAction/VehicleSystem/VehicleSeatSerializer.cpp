// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Description: Implements an entity class which can serialize vehicle parts

   -------------------------------------------------------------------------
   History:
   - 16:09:2005: Created by Mathieu Pinard

*************************************************************************/
#include "StdAfx.h"
#include "VehicleSeatSerializer.h"
#include "Vehicle.h"
#include "VehicleSeat.h"
#include "CryAction.h"
#include "Network/GameContext.h"

//------------------------------------------------------------------------
CVehicleSeatSerializer::CVehicleSeatSerializer()
	: m_pVehicle(0),
	m_pSeat(0)
{
}

//------------------------------------------------------------------------
CVehicleSeatSerializer::~CVehicleSeatSerializer()
{
}

//------------------------------------------------------------------------
void CVehicleSeatSerializer::PostInitialize()
{
	if (m_pSeat) // needs to be done here since anywhere earlier, EntityId is not known
		m_pSeat->SetSerializer(this);
	else if (!gEnv->bServer)
		return;

	CVehicleSeat* pSeat = NULL;
	EntityId parentId = 0;
	if (gEnv->bServer)
	{
		pSeat = static_cast<CVehicleSystem*>(CCryAction::GetCryAction()->GetIVehicleSystem())->GetInitializingSeat();
		CRY_ASSERT(pSeat);
		SetVehicle(static_cast<CVehicle*>(pSeat->GetVehicle()));
		parentId = m_pVehicle->GetEntityId();
	}

	auto &gameObject = GetEntity()->AcquireExternalComponent<IGameObject>();

	if (0 == (GetEntity()->GetFlags() & (ENTITY_FLAG_CLIENT_ONLY | ENTITY_FLAG_SERVER_ONLY)))
		if (!gameObject.BindToNetworkWithParent(eBTNM_Normal, parentId))
			return;

	GetEntity()->Hide(true);

	if (!IsDemoPlayback())
	{
		if (gEnv->bServer)
		{
			pSeat->SetSerializer(this);
			SetSeat(pSeat);
		}
		else
		{
			if (m_pVehicle)
			{
				gameObject.SetNetworkParent(m_pVehicle->GetEntityId());
			}
		}
	}
}

//------------------------------------------------------------------------
bool CVehicleSeatSerializer::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	if (m_pSeat)
		m_pSeat->SerializeActions(ser, aspect);
	return true;
}

namespace CVehicleSeatSerializerGetSpawnInfo
{
struct SInfo : public ISerializableInfo
{
	EntityId       vehicle;
	TVehicleSeatId seatId;
	void SerializeWith(TSerialize ser)
	{
		ser.Value("vehicle", vehicle, 'eid');
		ser.Value("seat", seatId, 'seat');
	}
};
}

//------------------------------------------------------------------------
ISerializableInfoPtr CVehicleSeatSerializer::GetSpawnInfo()
{
	CVehicleSeatSerializerGetSpawnInfo::SInfo* p = new CVehicleSeatSerializerGetSpawnInfo::SInfo;
	p->vehicle = m_pVehicle->GetEntityId();
	p->seatId = m_pSeat->GetSeatId();
	return p;
}

//------------------------------------------------------------------------
void CVehicleSeatSerializer::SetVehicle(CVehicle* pVehicle)
{
	m_pVehicle = pVehicle;
}

//------------------------------------------------------------------------
void CVehicleSeatSerializer::SetSeat(CVehicleSeat* pSeat)
{
	m_pSeat = pSeat;
}
