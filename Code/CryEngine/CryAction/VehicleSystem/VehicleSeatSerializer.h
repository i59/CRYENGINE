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
#ifndef __VEHICLESEATSERIALIZER_H__
#define __VEHICLESEATSERIALIZER_H__

#include <CryAction/IGameObject.h>

#include <CryEntitySystem/INetworkedEntityComponent.h>

class CVehicle;
class CVehicleSeat;

class CVehicleSeatSerializer : public CNetworkedEntityComponent<CVehicleSeatSerializer, IEntityComponent>
{
public:
	DECLARE_COMPONENT("VehicleSeatSerializer", 0xBE5D6187A05A42DB, 0xBDAE7A1335EAF4F9)

	CVehicleSeatSerializer();
	virtual ~CVehicleSeatSerializer();

	// IEntityComponent
	virtual void PostInitialize() override;

	virtual void Release() override { delete this; }

	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) override;

	virtual ISerializableInfoPtr GetSpawnInfo() override;
	// ~IEntityComponent

	void                         SetVehicle(CVehicle* pVehicle);
	void                         SetSeat(CVehicleSeat* pSeat);

protected:
	CVehicle*     m_pVehicle;
	CVehicleSeat* m_pSeat;
};

#endif
