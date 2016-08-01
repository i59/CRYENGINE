// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "CameraSource.h"
#include "IViewSystem.h"

#include "EntityComponents/GameObject.h"

void CCameraSource::PostInitialize()
{
	auto &gameObject = GetEntity()->AcquireComponent<CGameObject>();

	gameObject.CaptureView(this);
}

//------------------------------------------------------------------------
void CCameraSource::UpdateView(SViewParams& params)
{
	// update params
	const Matrix34& mat = GetEntity()->GetWorldTM();
	params.position = mat.GetTranslation();
	params.rotation = Quat(mat);
}
