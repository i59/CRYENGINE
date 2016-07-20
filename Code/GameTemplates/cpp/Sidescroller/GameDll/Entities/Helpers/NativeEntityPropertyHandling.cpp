#include "StdAfx.h"
#include "NativeEntityPropertyHandling.h"

#include <CryEntitySystem/IEntitySystem.h>

CNativeEntityPropertyHandler::CNativeEntityPropertyHandler(SNativeEntityPropertyInfo *pProperties, int numProperties, uint32 scriptFlags)
	: m_pProperties(pProperties)
	, m_numProperties(numProperties)
	, m_scriptFlags(scriptFlags) 
{
	gEnv->pEntitySystem->AddSink(this, IEntitySystem::OnRemove, 0);
}

CNativeEntityPropertyHandler::~CNativeEntityPropertyHandler()
{
	delete[] m_pProperties;
}

bool CNativeEntityPropertyHandler::OnRemove(IEntity *pEntity)
{
	if (m_entityPropertyMap.find(pEntity) != m_entityPropertyMap.end())
		m_entityPropertyMap.erase(pEntity);

	return true;
}

void CNativeEntityPropertyHandler::LoadEntityXMLProperties(IEntity *pEntity, const XmlNodeRef& xml)
{
	if(auto properties = xml->findChild("Properties"))
	{
		// Load default
		LoadEntityXMLGroupProperties(pEntity, properties, true);

		// Load folders
		for(int i = 0; i < properties->getChildCount(); i++)
		{
			XmlNodeRef groupNode = properties->getChild(i);
			LoadEntityXMLGroupProperties(pEntity, groupNode, false);
		}
	}

	PropertiesChanged(pEntity);
}

void CNativeEntityPropertyHandler::LoadEntityXMLGroupProperties(IEntity *pEntity, const XmlNodeRef &groupNode, bool bRootNode)
{
	LOADING_TIME_PROFILE_SECTION;

	const char *groupName = groupNode->getTag();
	bool bFoundGroup = bRootNode;

	SPropertyInfo info;
	int numProperties = GetPropertyCount();

	if (m_entityPropertyMap.find(pEntity) == m_entityPropertyMap.end())
		m_entityPropertyMap[pEntity] = CNativeEntityPropertyHandler::TPropertyStorage(numProperties);

	for(int i = 0; i < numProperties; i++)
	{
		GetPropertyInfo(i, info);

		if(!bFoundGroup)
		{
			if(!strcmp(info.name, groupName) && info.type == IEntityPropertyHandler::FolderBegin)
			{
				bFoundGroup = true;
				continue;
			}
		}
		else
		{
			if(info.type == IEntityPropertyHandler::FolderEnd || info.type == IEntityPropertyHandler::FolderBegin)
				break;

			XmlString value;
			if (groupNode->getAttr(info.name, value))
			{
				m_entityPropertyMap[pEntity][i] = value;
			}
		}
	}
}

void CNativeEntityPropertyHandler::SetProperty(IEntity *pEntity, int index, const char *value)
{
	if (m_entityPropertyMap.find(pEntity) == m_entityPropertyMap.end())
		m_entityPropertyMap[pEntity] = CNativeEntityPropertyHandler::TPropertyStorage(index + 1);

	m_entityPropertyMap[pEntity][index] = value;
}

const char *CNativeEntityPropertyHandler::GetProperty(IEntity *pEntity, int index) const
{
	auto propertyIt = m_entityPropertyMap.find(pEntity);
	if (propertyIt != m_entityPropertyMap.end())
	{
		auto propertyStoreIter = propertyIt->second.find(index);
		if(propertyStoreIter != propertyIt->second.end())
			return propertyStoreIter->second;
	}

	return GetDefaultProperty(index);
}

void CNativeEntityPropertyHandler::PropertiesChanged(IEntity *pEntity)
{
	SEntityEvent propertiesChangedEvent = SEntityEvent(ENTITY_EVENT_EDITOR_PROPERTY_CHANGED);
	pEntity->SendEvent(propertiesChangedEvent);
}