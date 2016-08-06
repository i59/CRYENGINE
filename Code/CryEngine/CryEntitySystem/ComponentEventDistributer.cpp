// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.
#include "stdafx.h"
#include "ComponentEventDistributer.h"

#include "EntitySystem.h"
#include "Entity.h"

CBulkEntityEventDistributor::CBulkEntityEventDistributor()
{
	// Catch new flags being added exceeding TSubscribedEventFlags capability, increase size if so
	COMPILE_TIME_ASSERT(sizeof(TSubscribedEventFlags) * 8 >= ENTITY_EVENT_LAST);

	// We currently only handle prephys update
	// Most other events are sent directly to individual entities
	m_trackedEventFlags = ENTITY_EVENT_BIT(ENTITY_EVENT_PREPHYSICSUPDATE);
}

void CBulkEntityEventDistributor::EnableEvent(IEntity* pEntity, EEntityEvent event, uint32 priority, bool bEnable)
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_ENTITY);
	
	auto eventIt = m_entityEventMap.find(event);
	if (bEnable)
	{
		if (eventIt == m_entityEventMap.end())
		{
			eventIt = m_entityEventMap.insert(TEntityEventMap::value_type(event, TEntityEventList())).first;
		}

		stl::push_back_unique(eventIt->second, pEntity);
	}
	else if (eventIt != m_entityEventMap.end())
	{
		struct SRemovePredicate
		{
			SRemovePredicate(EntityId id)
				: entityId(id) {}

			bool operator()(const IEntity *pEntity)
			{
				return entityId == pEntity->GetId();
			}

			EntityId entityId;
		};

		eventIt->second.erase(std::remove_if(eventIt->second.begin(), eventIt->second.end(), SRemovePredicate(pEntity->GetId())), eventIt->second.end());
	}
}

void CBulkEntityEventDistributor::SendEventToEntities(const SEntityEvent &event)
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_ENTITY);

	auto entityStorageIt = m_entityEventMap.find(event.event);
	if (entityStorageIt != m_entityEventMap.end())
	{
		for (auto it = entityStorageIt->second.begin(); it != entityStorageIt->second.end(); ++it)
		{
			(*it)->SendEvent(event);
		}
	}
}

void CBulkEntityEventDistributor::Reset()
{
	m_entityEventMap.clear();
}