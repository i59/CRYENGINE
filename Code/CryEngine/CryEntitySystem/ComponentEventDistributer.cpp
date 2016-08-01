// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.
#include "stdafx.h"
#include "ComponentEventDistributer.h"

CComponentEventDistributor::CComponentEventDistributor()
{
	// Catch new flags being added exceeding TSubscribedEventFlags capability, increase size if so
	COMPILE_TIME_ASSERT(sizeof(TSubscribedEventFlags) * 8 >= ENTITY_EVENT_LAST);
}

void CComponentEventDistributor::EnableEvent(IEntityComponent &component, EEntityEvent event, uint32 priority, bool bEnable)
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_ENTITY);

	auto eventIt = m_eventComponentMap.find(event);
	if (bEnable)
	{
		if (eventIt == m_eventComponentMap.end())
		{
			eventIt = m_eventComponentMap.insert(TEventComponentMap::value_type(event, SEventComponents())).first;
		}

		// Sorting is "automatic" as we're using std::set with a custom compare implementation.
		// This ensures that we only need to sort when an event is enabled
		eventIt->second.m_components.insert(SEventComponentInfo(&component, priority));

		auto reverseLookupIt = m_entityReverseLookupMap.find(component.GetEntityId());
		if (reverseLookupIt == m_entityReverseLookupMap.end())
		{
			reverseLookupIt = m_entityReverseLookupMap.insert(TEntityReverseLookupMap::value_type(component.GetEntityId(), SEntitySubscribedEvents())).first;
		}

		reverseLookupIt->second.m_subscribedEventFlags |= ENTITY_EVENT_BIT(event);
	}
	else if (eventIt != m_eventComponentMap.end())
	{
		for (auto it = eventIt->second.m_components.begin(); it != eventIt->second.m_components.end(); ++it)
		{
			if (it->m_pComponent == &component)
			{
				eventIt->second.m_components.erase(it);
				break;
			}
		}
	}
}

void CComponentEventDistributor::SendEvent(const SEntityEvent& event)
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_ENTITY);

	auto eventIt = m_eventComponentMap.find(event.event);
	if (eventIt != m_eventComponentMap.end())
	{
		// Sorting for priority is done when the entity registers interest in an event
		for (auto componentInfoIt = eventIt->second.m_components.begin(); componentInfoIt != eventIt->second.m_components.end(); ++componentInfoIt)
		{
			// Send event
			componentInfoIt->m_pComponent->ProcessEvent(event);
		}
	}
}

void CComponentEventDistributor::SendEventToEntity(IEntity &entity, const SEntityEvent& event)
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_ENTITY);

	auto reverseEntityLookupIt = m_entityReverseLookupMap.find(entity.GetId());
	if (reverseEntityLookupIt == m_entityReverseLookupMap.end())
		return;

	if (reverseEntityLookupIt->second.m_subscribedEventFlags & (1ui64 << event.event))
	{
		auto eventIt = m_eventComponentMap.find(event.event);
		CRY_ASSERT(eventIt != m_eventComponentMap.end());

		for (auto it = eventIt->second.m_components.begin(); it != eventIt->second.m_components.end(); ++it)
		{
			if (it->m_pComponent->GetEntityId() == entity.GetId())
			{
				it->m_pComponent->ProcessEvent(event);
			}
		}
	}
}

void CComponentEventDistributor::OnEntityDeleted(IEntity *pEntity)
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_ENTITY);

	auto reverseEntityLookupIt = m_entityReverseLookupMap.find(pEntity->GetId());
	if (reverseEntityLookupIt == m_entityReverseLookupMap.end())
		return;

	for(TSubscribedEventFlags i = 0; i < ENTITY_EVENT_LAST; i++)
	{
		if (reverseEntityLookupIt->second.m_subscribedEventFlags & ENTITY_EVENT_BIT(i))
		{
			auto eventIt = m_eventComponentMap.find((EEntityEvent)i);
			CRY_ASSERT(eventIt != m_eventComponentMap.end());

			for (auto it = eventIt->second.m_components.begin(); it != eventIt->second.m_components.end(); ++it)
			{
				if (it->m_pComponent->GetEntityId() == pEntity->GetId())
				{
					eventIt->second.m_components.erase(it);
					break;
				}
			}
		}
	}
}

void CComponentEventDistributor::RemapEntityID(EntityId oldID, EntityId newID)
{
	auto reverseEntityLookupIt = m_entityReverseLookupMap.find(oldID);
	if (reverseEntityLookupIt == m_entityReverseLookupMap.end())
		return;

	auto subscribedEvents = reverseEntityLookupIt->second;
	m_entityReverseLookupMap.erase(reverseEntityLookupIt);

	m_entityReverseLookupMap.insert(TEntityReverseLookupMap::value_type(newID, subscribedEvents));
}

void CComponentEventDistributor::Reset()
{
	m_eventComponentMap.clear();
}
