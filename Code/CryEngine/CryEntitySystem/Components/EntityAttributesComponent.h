// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#ifndef __ENTITYATTRIBUTESPROXY_H__
#define __ENTITYATTRIBUTESPROXY_H__

#pragma once

#include "EntitySystem.h"
#include <CryEntitySystem/IEntityAttributesProxy.h>
#include <CryEntitySystem/IEntityClass.h>
#include <CryNetwork/ISerialize.h>

//////////////////////////////////////////////////////////////////////////
// Description:
//    Proxy for storage of entity attributes.
//////////////////////////////////////////////////////////////////////////
class CEntityAttributesComponent : public IEntityAttributesComponent
{
public:
	virtual ~CEntityAttributesComponent() {}

	// IEntityComponent
	virtual void Initialize(IEntity &entity) override;
	virtual void OnEntityReload(SEntitySpawnParams& params, XmlNodeRef entityNode) override;

	virtual void Serialize(TSerialize ser) override;
	virtual void SerializeXML(XmlNodeRef& entityNode, bool bLoading, bool bFromInit) override;

	virtual bool NeedSerialize() override;
	
	virtual void GetMemoryUsage(ICrySizer* pSizer) const override;
	// ~IEntityComponent

	// IEntityAttributesProxy
	virtual void                         SetAttributes(const TEntityAttributeArray& attributes) override;
	virtual TEntityAttributeArray&       GetAttributes() override;
	virtual const TEntityAttributeArray& GetAttributes() const override;
	// ~IEntityAttributesProxy

private:

	TEntityAttributeArray m_attributes;
};

DECLARE_SHARED_POINTERS(CEntityAttributesComponent)

#endif //__ENTITYATTRIBUTESPROXY_H__
