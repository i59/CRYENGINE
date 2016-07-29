// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:   EntityNodeProxy.h
//  Version:     v1.00
//  Created:     23/11/2010 by Benjamin B.
//  Description:
// -------------------------------------------------------------------------
//  History:	The EntityNodeProxy handles events that are specific to EntityNodes
//						(e.g. Footsteps).
//
////////////////////////////////////////////////////////////////////////////

#ifndef __EntityNodeProxy_h__
#define __EntityNodeProxy_h__
#pragma once

#include "EntitySystem.h"
#include <CryNetwork/ISerialize.h>

class CEntityNodeComponent : public IEntityComponent
{
public:
	DECLARE_COMPONENT("EntityNodeComponent", 0x3592CE70D61B47FF, 0xBCC75AA894E236F7)

	// IEntityComponent
	virtual void ProcessEvent(SEntityEvent& event) override;

	virtual void Serialize(TSerialize ser) override {}

	virtual bool GetSignature(TSerialize signature) override { return true; }

	virtual void GetMemoryUsage(ICrySizer* pSizer) const override
	{
		pSizer->AddObject(this, sizeof(*this));
	}
	// ~IEntityComponent
};

#endif
