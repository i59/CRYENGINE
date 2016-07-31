// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "stdafx.h"

#include "EntityAttributesComponent.h"

#include <CrySerialization/IArchive.h>
#include <CrySerialization/IArchiveHost.h>

//////////////////////////////////////////////////////////////////////////
void CEntityAttributesComponent::Initialize(IEntity &entity)
{
	IEntityComponent::Initialize(entity);

	if (m_attributes.empty())
	{
		if (IEntityArchetype* pArchetype = m_pEntity->GetArchetype())
		{
			EntityAttributeUtils::CloneAttributes(pArchetype->GetAttributes(), m_attributes);
		}
		else
		{
			EntityAttributeUtils::CloneAttributes(m_pEntity->GetClass()->GetEntityAttributes(), m_attributes);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityAttributesComponent::Reload(SEntitySpawnParams& params, XmlNodeRef entityNode)
{
	if (params.pArchetype)
	{
		EntityAttributeUtils::CloneAttributes(params.pArchetype->GetAttributes(), m_attributes);
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityAttributesComponent::SerializeXML(XmlNodeRef& entityNodeXML, bool bLoading, bool bFromInit)
{
	if (bLoading)
	{
		if (XmlNodeRef attributesNodeXML = entityNodeXML->findChild("Attributes"))
		{
			SEntityAttributesSerializer serializer(m_attributes);
			Serialization::LoadXmlNode(serializer, attributesNodeXML);
		}
	}
	else
	{
		if (!m_attributes.empty())
		{
			SEntityAttributesSerializer serializer(m_attributes);
			if (XmlNodeRef attributesNodeXML = Serialization::SaveXmlNode(serializer, "Attributes"))
			{
				entityNodeXML->addChild(attributesNodeXML);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityAttributesComponent::Serialize(TSerialize serialize) {}

//////////////////////////////////////////////////////////////////////////
bool CEntityAttributesComponent::NeedSerialize()
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CEntityAttributesComponent::GetMemoryUsage(ICrySizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
}

//////////////////////////////////////////////////////////////////////////
void CEntityAttributesComponent::SetAttributes(const TEntityAttributeArray& attributes)
{
	const size_t attributeCount = attributes.size();
	m_attributes.resize(attributeCount);
	for (size_t iAttribute = 0; iAttribute < attributeCount; ++iAttribute)
	{
		IEntityAttribute* pSrc = attributes[iAttribute].get();
		IEntityAttribute* pDst = m_attributes[iAttribute].get();
		if ((pDst != NULL) && (strcmp(pSrc->GetName(), pDst->GetName()) == 0))
		{
			Serialization::CloneBinary(*pDst, *pSrc);
		}
		else if (pSrc != NULL)
		{
			m_attributes[iAttribute] = pSrc->Clone();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
TEntityAttributeArray& CEntityAttributesComponent::GetAttributes()
{
	return m_attributes;
}

//////////////////////////////////////////////////////////////////////////
const TEntityAttributeArray& CEntityAttributesComponent::GetAttributes() const
{
	return m_attributes;
}
