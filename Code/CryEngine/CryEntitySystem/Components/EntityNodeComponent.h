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

class CEntityNodeComponent : public IEntityNodeComponent
{
public:
	virtual ~CEntityNodeComponent() {}

	// IEntityComponent
	virtual void PostInitialize() override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	virtual void GetMemoryUsage(ICrySizer* pSizer) const override
	{
		pSizer->AddObject(this, sizeof(*this));
	}
	// ~IEntityComponent
};

#endif
