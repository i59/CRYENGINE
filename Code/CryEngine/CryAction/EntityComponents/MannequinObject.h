// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  Created          : 13/05/2014 by Jean Geffroy
//
////////////////////////////////////////////////////////////////////////////

#ifndef __MANNEQUIN_OBJECT_H__
#define __MANNEQUIN_OBJECT_H__

#include "IAnimationGraph.h"

class CMannequinObject
	: public IEntityComponent
{
public:
	DECLARE_COMPONENT("MannequinObject", 0x31E2516BF11346DB, 0x982F2AFCA87BB74F)

	CMannequinObject();
	virtual ~CMannequinObject() {}

	// IEntityComponent
	virtual void PostInitialize() override;
	virtual void ProcessEvent(const SEntityEvent& evt) override;

	virtual void Release() override { delete this; }
	// ~IEntityComponent

protected:
	void Reset();
	void OnScriptEvent(const char* eventName);

private:
	IAnimatedCharacter* m_pAnimatedCharacter;
};

#endif
