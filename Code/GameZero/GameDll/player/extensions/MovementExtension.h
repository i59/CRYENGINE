// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <CryEntitySystem/IEntityComponent.h>

class CMovementExtension : public IEntityComponent
{
public:
	DECLARE_COMPONENT("Movement", 0x636D790420F347B4, 0xB7F8760D0CCF26EE)

	// IEntityComponent
	virtual void PostInitialize() override;

	virtual void Release() override { delete this; }

	virtual void ProcessEvent(const SEntityEvent &event) override;
	// ~IEntityComponent

	CMovementExtension();
	virtual ~CMovementExtension();

private:
	float m_movementSpeed;
	float m_boostMultiplier;
};
