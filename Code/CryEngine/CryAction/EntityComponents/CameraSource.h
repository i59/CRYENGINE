// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#ifndef __GAMEOBJECTCAMERASOURCE_H__
#define __GAMEOBJECTCAMERASOURCE_H__

#pragma once

#include <IViewSystem.h>

#include <CryAction/IGameObject.h>

class CCameraSource 
	: public IEntityComponent
	, public IGameObjectView
{
public:
	DECLARE_COMPONENT("CameraSource", 0xCE62FA63F9CC4967, 0xB526D426CC452347)

	virtual ~CCameraSource() {}

	// IEntityComponent
	virtual void PostInitialize() override;

	virtual void GetMemoryUsage(ICrySizer* s) const override
	{
		s->AddObject(this, sizeof(*this));
	}
	// ~IEntityComponent

	// IGameObjectView
	virtual void UpdateView(SViewParams &params) override;
	virtual void PostUpdateView(SViewParams &params) override {}
	// ~IGameObjectView
};

#endif // __GAMEOBJECTCAMERASOURCE_H__
