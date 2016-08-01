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

class CComponentEventDistributor
{
public:
	// Struct containing a component and the priority that the event it belongs to should receive
	struct SEventComponentInfo
	{
		SEventComponentInfo() : m_pComponent(nullptr) {}
		SEventComponentInfo(IEntityComponent *pComponent, uint32 priority)
			: m_pComponent(pComponent)
			, eventPriority(priority)
		{
		}

		// Pointer to the component
		IEntityComponent *m_pComponent;
		// Priority of the event, in order to determine which component gets it first
		uint32 eventPriority;
	};

	// Struct containing information on the components that have requested to receive this event
	struct SEventComponents
	{
		struct SCompare 
		{
			bool operator() (const SEventComponentInfo& lhs, const SEventComponentInfo& rhs) const 
			{
				if (lhs.eventPriority > rhs.eventPriority)
				{
					return true;
				}

				if (lhs.eventPriority < rhs.eventPriority)
				{
					return false;
				}

				return lhs.m_pComponent->GetEntityId() > rhs.m_pComponent->GetEntityId();
			}
		};

		std::set<SEventComponentInfo, SCompare> m_components;
	};

public:
	CComponentEventDistributor();

	void EnableEvent(IEntityComponent &component, EEntityEvent event, uint32 priority, bool bEnable);
	void SendEvent(const SEntityEvent& event);
	void SendEventToEntity(IEntity &entity, const SEntityEvent& event);

	void RemapEntityID(EntityId oldID, EntityId newID);
	void OnEntityDeleted(IEntity* piEntity);

	void Reset();

protected:
	typedef std::unordered_map<EEntityEvent, SEventComponents> TEventComponentMap;
	TEventComponentMap m_eventComponentMap;

	// Pick type that can fit all entity events as bitwise flags
	typedef uint64 TSubscribedEventFlags;

	struct SEntitySubscribedEvents
	{
		// Bit flags indicating which events are subscribed
		TSubscribedEventFlags m_subscribedEventFlags;
	};

	typedef std::unordered_map<EntityId, SEntitySubscribedEvents> TEntityReverseLookupMap;

	// Reverse lookup map to speed up finding and erasing events from the component storage
	// Otherwise we would have to go through every component's subscribed event
	TEntityReverseLookupMap m_entityReverseLookupMap;
};