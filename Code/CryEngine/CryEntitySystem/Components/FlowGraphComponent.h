// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:   FlowGraphProxy.h
//  Version:     v1.00
//  Created:     6/6/2005 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description:
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __FlowGraphProxy_h__
#define __FlowGraphProxy_h__
#pragma once

#include <CryNetwork/ISerialize.h>

//////////////////////////////////////////////////////////////////////////
// Description:
//    Handles sounds in the entity.
//////////////////////////////////////////////////////////////////////////
class CFlowGraphComponent : public IEntityFlowGraphComponent
{
public:
	CFlowGraphComponent();
	virtual ~CFlowGraphComponent();

	// IEntityComponent
	virtual void PostInitialize() override;

	virtual void ProcessEvent(const SEntityEvent& event) override;
	virtual void Reload(SEntitySpawnParams& params, XmlNodeRef entityNode) override;

	virtual void Update(SEntityUpdateContext& ctx) override;

	virtual void Serialize(TSerialize ser) override;
	virtual void SerializeXML(XmlNodeRef& entityNode, bool bLoading, bool bFromInit) override;

	virtual bool NeedSerialize() override;

	virtual void GetMemoryUsage(ICrySizer* pSizer) const override
	{
		pSizer->AddObject(this, sizeof(*this));
	}
	// ~IEntityComponent

	//////////////////////////////////////////////////////////////////////////
	// IEntityFlowGraphProxy interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void        SetFlowGraph(IFlowGraph* pFlowGraph);
	virtual IFlowGraph* GetFlowGraph();
	virtual void        AddEventListener(IEntityEventListener* pListener);
	virtual void        RemoveEventListener(IEntityEventListener* pListener);
	//////////////////////////////////////////////////////////////////////////
private:
	void OnMove();

private:
	//////////////////////////////////////////////////////////////////////////
	// Private member variables.
	//////////////////////////////////////////////////////////////////////////
	IFlowGraph* m_pFlowGraph;

	typedef std::list<IEntityEventListener*> Listeners;
	Listeners m_listeners;
};

#endif // __FlowGraphProxy_h__
