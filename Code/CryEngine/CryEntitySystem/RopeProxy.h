// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:   RopeProxy.h
//  Version:     v1.00
//  Created:     23/10/2006 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description:
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __RopeProxy_h__
#define __RopeProxy_h__
#pragma once

// forward declarations.
struct SEntityEvent;
struct IPhysicalEntity;
struct IPhysicalWorld;

//////////////////////////////////////////////////////////////////////////
// Description:
//    Implements rope proxy class for entity.
//////////////////////////////////////////////////////////////////////////
class CRopeComponent : public IEntityRopeComponent
{
public:
	CRopeComponent();
	virtual ~CRopeComponent();

	// IEntityComponent
	virtual void Initialize(IEntity &entity) override;
	virtual void ProcessEvent(SEntityEvent& event) override;

	virtual void Reload(SEntitySpawnParams& params, XmlNodeRef entityNode) override;

	virtual void Serialize(TSerialize ser) override;
	virtual void SerializeXML(XmlNodeRef& entityNode, bool bLoading, bool bFromInit) override;

	virtual bool NeedSerialize() override { return true; }
	virtual bool GetSignature(TSerialize signature) override;

	virtual void GetMemoryUsage(ICrySizer* pSizer) const override
	{
		pSizer->AddObject(this, sizeof(*this));
	}
	// ~IEntityComponent

	//////////////////////////////////////////////////////////////////////////
	/// IEntityRopeProxy
	//////////////////////////////////////////////////////////////////////////
	virtual IRopeRenderNode* GetRopeRenderNode() { return m_pRopeRenderNode; };
	//////////////////////////////////////////////////////////////////////////

	void PreserveParams();
protected:
	IRopeRenderNode* m_pRopeRenderNode;
	int              m_nSegmentsOrg;
	float            m_texTileVOrg;
};

#endif // __RopeProxy_h__
