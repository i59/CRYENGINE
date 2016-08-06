// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:   EntityEventDistributer.h
//  Version:     v1.00
//  Created:     14/06/2012 by Steve North
//  Compilers:   Visual Studio.NET 2010
//  Description:
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#include <CryEntitySystem/IEntitySystem.h>
#include <CryEntitySystem/IEntityComponent.h>
#include <CryCore/CryFlags.h>

#include <bitset>

// Class responsible for sending entity events en masse as opposed to more common per-entity events
class CBulkEntityEventDistributor
{
public:
	// Pick type that can fit all entity events as bitwise flags
	typedef uint64 TSubscribedEventFlags;

	CBulkEntityEventDistributor();

	TSubscribedEventFlags GetTrackedEvents() const { return m_trackedEventFlags; }
	bool IsTrackingEvent(EEntityEvent event) const { return (m_trackedEventFlags & ENTITY_EVENT_BIT(event)) != 0;  }

	void EnableEvent(IEntity* pEntity, EEntityEvent event, uint32 priority, bool bEnable);
	
	void Reset();
	
	// Specialized functions below that send to all entities simultaneously (if subscribed)
	void SendEventToEntities(const SEntityEvent &entity);

protected:
	typedef std::vector<IEntity*> TEntityEventList;
	typedef std::unordered_map<EEntityEvent, TEntityEventList> TEntityEventMap;
	TEntityEventMap m_entityEventMap;

	// Flags indicating which events are handled by the distributor
	TSubscribedEventFlags m_trackedEventFlags;
};