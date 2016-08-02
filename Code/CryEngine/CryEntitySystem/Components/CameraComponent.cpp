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

#include "stdafx.h"
#include "CameraComponent.h"
#include <CryNetwork/ISerialize.h>

//////////////////////////////////////////////////////////////////////////
void CCameraComponent::Initialize(IEntity &entity)
{
	IEntityComponent::Initialize(entity);

	UpdateMaterialCamera();

	EnableEvent(ENTITY_EVENT_INIT, 0, true);
	EnableEvent(ENTITY_EVENT_XFORM, 0, true);
}

//////////////////////////////////////////////////////////////////////////
void CCameraComponent::OnEntityReload(SEntitySpawnParams& params, XmlNodeRef entityNode)
{
	UpdateMaterialCamera();
}

//////////////////////////////////////////////////////////////////////////
void CCameraComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_INIT:
	case ENTITY_EVENT_XFORM:
		{
			UpdateMaterialCamera();
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
void CCameraComponent::UpdateMaterialCamera()
{
	float fov = m_camera.GetFov();
	m_camera = GetISystem()->GetViewCamera();
	Matrix34 wtm = m_pEntity->GetWorldTM();
	wtm.OrthonormalizeFast();
	m_camera.SetMatrix(wtm);
	m_camera.SetFrustum(m_camera.GetViewSurfaceX(), m_camera.GetViewSurfaceZ(), fov, m_camera.GetNearPlane(), m_camera.GetFarPlane(), m_camera.GetPixelAspectRatio());

	IMaterial* pMaterial = m_pEntity->GetMaterial();
	if (pMaterial)
		pMaterial->SetCamera(m_camera);
}

//////////////////////////////////////////////////////////////////////////
void CCameraComponent::SetCamera(CCamera& cam)
{
	m_camera = cam;
	UpdateMaterialCamera();
}