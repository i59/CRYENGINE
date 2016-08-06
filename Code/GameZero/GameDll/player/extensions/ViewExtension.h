// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <CryAction/IGameObject.h>

class CViewExtension 
	: public IEntityComponent
	, IGameObjectView
{
public:
	DECLARE_COMPONENT("View", 0x70C06CFAF79B4053, 0x98B3C955A409D9C0)

	// IEntityComponent
	virtual void PostInitialize() override;
	// ~IEntityComponent

	// IGameObjectView
	virtual void UpdateView(SViewParams& params) override;
	virtual void PostUpdateView(SViewParams& viewParams) override {}
	// ~IGameObjectView

	CViewExtension();
	virtual ~CViewExtension();

private:
	void CreateView();

	unsigned int m_viewId;
	float        m_camFOV;
};
