// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

class CMovementExtension : public IEntityComponent
{
public:
	DECLARE_COMPONENT("Movement", 0x636D790420F347B4, 0xB7F8760D0CCF26EE)

	// IEntityComponent
	virtual void PostInitialize() override;
	virtual void PostUpdate(float frameTime) override;
	// ~IEntityComponent

	CMovementExtension();
	virtual ~CMovementExtension();

private:
	float m_movementSpeed;
	float m_boostMultiplier;
};
