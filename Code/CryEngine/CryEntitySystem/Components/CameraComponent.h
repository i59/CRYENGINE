// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:   CameraProxy.h
//  Version:     v1.00
//  Created:     5/12/2005 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description:
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __CameraProxy_h__
#define __CameraProxy_h__
#pragma once

#include "Entity.h"
#include "EntitySystem.h"

struct SProximityElement;

//////////////////////////////////////////////////////////////////////////
// Description:
//    Handles sounds in the entity.
//////////////////////////////////////////////////////////////////////////
class CCameraComponent : public IEntityCameraComponent
{
public:
	virtual ~CCameraComponent() {}

	// IEntityComponent
	virtual void Initialize(IEntity &entity) override;
	virtual void ProcessEvent(const SEntityEvent& event) override;
	virtual void OnEntityReload(SEntitySpawnParams& params, XmlNodeRef entityNode) override;

	virtual void Release() override { delete this; }

	virtual void Serialize(TSerialize ser) override {}

	virtual void GetMemoryUsage(ICrySizer* pSizer) const override
	{
		pSizer->AddObject(this, sizeof(*this));
	}
	// ~IEntityComponent

	// IEntityCameraComponent
	virtual void     SetCamera(CCamera& cam);
	virtual CCamera& GetCamera() { return m_camera; };
	// ~IEntityCameraComponent

	void         UpdateMaterialCamera();

private:;
	CCamera  m_camera;
};

#endif // __CameraProxy_h__
