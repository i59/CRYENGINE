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
#ifndef __VEHICLEPARTDETACHEDENTITY_H__
#define __VEHICLEPARTDETACHEDENTITY_H__

class CVehiclePartDetachedEntity : public IEntityComponent
{
public:
	DECLARE_COMPONENT("VehiclePartDetachedEntity", 0x491E767E8A4448AA, 0x9C647DA183DC20BB)

	virtual ~CVehiclePartDetachedEntity() {}

	// IEntityComponent
	virtual void PostInitialize() override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	virtual void Update(SEntityUpdateContext& ctx) override;

	virtual bool NeedSerialize() override { return true; }
	virtual void Serialize(TSerialize ser) override;
	// ~IEntityComponent

protected:
	void InitVehiclePart();

	float m_timeUntilStartIsOver;
};

#endif
