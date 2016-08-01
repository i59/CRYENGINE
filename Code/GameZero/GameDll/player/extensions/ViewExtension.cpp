// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "ViewExtension.h"
#include <IViewSystem.h>

CViewExtension::CViewExtension()
	: m_viewId(0)
	, m_camFOV(0.0f)
{
}

CViewExtension::~CViewExtension()
{
	if(auto *pGameObject = GetEntity()->QueryComponent<IGameObject>())
		pGameObject->ReleaseView(this);

	gEnv->pGame->GetIGameFramework()->GetIViewSystem()->RemoveView(m_viewId);
	gEnv->pConsole->UnregisterVariable("gamezero_cam_fov", true);
}

void CViewExtension::PostInitialize()
{
	CreateView();

	auto &gameObject = GetEntity()->AcquireExternalComponent<IGameObject>();
	gameObject.CaptureView(this);

	REGISTER_CVAR2("gamezero_cam_fov", &m_camFOV, 60.0f, VF_NULL, "Camera FOV.");
}

void CViewExtension::CreateView()
{
	IViewSystem* pViewSystem = gEnv->pGame->GetIGameFramework()->GetIViewSystem();
	IView* pView = pViewSystem->CreateView();

	if (auto *pGameObject = GetEntity()->QueryComponent<IGameObject>())
		pView->LinkTo(pGameObject);

	pViewSystem->SetActiveView(pView);

	m_viewId = pViewSystem->GetViewId(pView);
}

void CViewExtension::UpdateView(SViewParams& params)
{
	params.position = GetEntity()->GetWorldPos();
	params.rotation = GetEntity()->GetWorldRotation();
	params.fov = DEG2RAD(m_camFOV);
}
