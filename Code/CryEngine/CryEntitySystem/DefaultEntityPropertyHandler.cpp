#include "StdAfx.h"
#include "DefaultEntityPropertyHandler.h"

#include <CryEntitySystem/IEntitySystem.h>

CDefaultEntityPropertyHandler::CDefaultEntityPropertyHandler(IEntityPropertyHandler::SPropertyInfo *pProperties, int numProperties, uint32 scriptFlags)
	: m_pProperties(std::move(pProperties))
	, m_numProperties(numProperties)
	, m_scriptFlags(scriptFlags) 
{
	CRY_ASSERT(m_pProperties != nullptr);

	// Make sure that our OnRemove function is called so that we can erase from the property map
	gEnv->pEntitySystem->AddSink(this, IEntitySystem::OnRemove, 0);
}

CDefaultEntityPropertyHandler::~CDefaultEntityPropertyHandler()
{
	delete[] m_pProperties;
}

bool CDefaultEntityPropertyHandler::OnRemove(IEntity *pEntity)
{
	m_entityPropertyMap.erase(pEntity);

	return true;
}

void CDefaultEntityPropertyHandler::LoadEntityXMLProperties(IEntity *pEntity, const XmlNodeRef& xml)
{
	if(auto properties = xml->findChild("Properties"))
	{
		LOADING_TIME_PROFILE_SECTION;

		// Find (or insert) property storage entry for this entity
		auto entityPropertyStorageIt = m_entityPropertyMap.find(pEntity);
		if (entityPropertyStorageIt == m_entityPropertyMap.end())
		{
			// Create the entry and reserve bucket count
			entityPropertyStorageIt = m_entityPropertyMap.insert(TEntityPropertyMap::value_type(pEntity, CDefaultEntityPropertyHandler::TPropertyStorage(m_numProperties))).first;
		}

		// Load default group of properties
		LoadEntityXMLGroupProperties(entityPropertyStorageIt->second, properties, true);

		// Load folders
		for(auto i = 0; i < properties->getChildCount(); i++)
		{
			LoadEntityXMLGroupProperties(entityPropertyStorageIt->second, properties->getChild(i), false);
		}
	}

	PropertiesChanged(pEntity);
}

void CDefaultEntityPropertyHandler::LoadEntityXMLGroupProperties(TPropertyStorage &entityProperties, const XmlNodeRef &groupNode, bool bRootNode)
{
	bool bFoundGroup = bRootNode;
	XmlString value;

	for(auto i = 0; i < m_numProperties; i++)
	{
		if(!bFoundGroup)
		{
			if(!strcmp(m_pProperties[i].name, groupNode->getTag()) && m_pProperties[i].type == IEntityPropertyHandler::FolderBegin)
			{
				bFoundGroup = true;
				continue;
			}
		}
		else
		{
			if(m_pProperties[i].type == IEntityPropertyHandler::FolderEnd || m_pProperties[i].type == IEntityPropertyHandler::FolderBegin)
				break;

			if (groupNode->getAttr(m_pProperties[i].name, value))
			{
				entityProperties[i] = value;
			}
		}
	}
}

void CDefaultEntityPropertyHandler::SetProperty(IEntity *pEntity, int index, const char *value)
{
	CRY_ASSERT(index < m_numProperties);

	auto entityPropertyStorageIt = m_entityPropertyMap.find(pEntity);
	if (entityPropertyStorageIt == m_entityPropertyMap.end())
	{
		entityPropertyStorageIt = m_entityPropertyMap.insert(TEntityPropertyMap::value_type(pEntity, CDefaultEntityPropertyHandler::TPropertyStorage(index + 1))).first;
	}

	// Update this entity's property at index
	entityPropertyStorageIt->second[index] = value;
}

const char *CDefaultEntityPropertyHandler::GetProperty(IEntity *pEntity, int index) const
{
	CRY_ASSERT(index < m_numProperties);

	auto entityPropertyStorageIt = m_entityPropertyMap.find(pEntity);
	if (entityPropertyStorageIt != m_entityPropertyMap.end())
	{
		auto entityPropertyMap = entityPropertyStorageIt->second.find(index);
		if(entityPropertyMap != entityPropertyStorageIt->second.end())
			return entityPropertyMap->second;
	}

	return m_pProperties[index].defaultValue;
}

void CDefaultEntityPropertyHandler::PropertiesChanged(IEntity *pEntity)
{
	auto propertiesChangedEvent = SEntityEvent(ENTITY_EVENT_EDITOR_PROPERTY_CHANGED);
	pEntity->SendEvent(propertiesChangedEvent);
}