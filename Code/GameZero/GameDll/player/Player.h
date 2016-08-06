// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <CryEntitySystem/IEntityComponent.h>

class CPlayer : public IEntityComponent
{
public:
	DECLARE_COMPONENT("Player", 0x0B742BAF041E4801, 0x882B3EAF73ED5DC9)

	CPlayer();
	virtual ~CPlayer();

	// IEntityComponent
	virtual void Initialize(IEntity &entity) override;

	virtual void Release() override { delete this; }
	// ~IEntityComponent
};
