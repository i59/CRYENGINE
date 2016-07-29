// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:   TriggerProxy.h
//  Version:     v1.00
//  Created:     5/12/2005 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description:
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __TriggerProxy_h__
#define __TriggerProxy_h__
#pragma once

#include "Entity.h"
#include "EntitySystem.h"

struct SProximityElement;

//////////////////////////////////////////////////////////////////////////
// Description:
//    Handles sounds in the entity.
//////////////////////////////////////////////////////////////////////////
class CTriggerComponent : public IEntityTriggerComponent
{
public:
	CTriggerComponent();
	~CTriggerComponent();

	// IEntityComponent
	virtual void Initialize(IEntity &entity) override;
	virtual void ProcessEvent(SEntityEvent& event) override;

	virtual void Reload(SEntitySpawnParams& params, XmlNodeRef entityNode) override;
	virtual void Update(SEntityUpdateContext& ctx) override;

	virtual void Serialize(TSerialize ser) override;

	virtual bool NeedSerialize() override;

	virtual bool GetSignature(TSerialize signature) override;
	// ~IEntityComponent

	//////////////////////////////////////////////////////////////////////////
	// IEntityTriggerProxy
	//////////////////////////////////////////////////////////////////////////
	virtual void SetTriggerBounds(const AABB& bbox) { SetAABB(bbox); };
	virtual void GetTriggerBounds(AABB& bbox)       { bbox = m_aabb; };
	virtual void ForwardEventsTo(EntityId id)       { m_forwardingEntity = id; }
	virtual void InvalidateTrigger();
	//////////////////////////////////////////////////////////////////////////

	CEntity *GetCEntity() const;

	const AABB&              GetAABB() const { return m_aabb; }
	void                     SetAABB(const AABB& aabb);

	CProximityTriggerSystem* GetTriggerSystem() { return g_pIEntitySystem->GetProximityTriggerSystem(); }

	virtual void             GetMemoryUsage(ICrySizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}
private:
	void OnMove(bool invalidateAABB = false);
	void Reset();

private:
	//////////////////////////////////////////////////////////////////////////
	// Private member variables.
	//////////////////////////////////////////////////////////////////////////
	AABB               m_aabb;
	SProximityElement* m_pProximityTrigger;
	EntityId           m_forwardingEntity;
};

#endif // __TriggerProxy_h__
