// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "MovementExtension.h"

#include "player/extensions/InputExtension.h"

CMovementExtension::CMovementExtension()
	: m_movementSpeed(0.0f)
	, m_boostMultiplier(0.0f)
{
}

CMovementExtension::~CMovementExtension()
{
	gEnv->pConsole->UnregisterVariable("gamezero_pl_movementSpeed", true);
	gEnv->pConsole->UnregisterVariable("gamezero_pl_boostMultiplier", true);
}

void CMovementExtension::PostInitialize()
{
	REGISTER_CVAR2("gamezero_pl_movementSpeed", &m_movementSpeed, 20.0f, VF_NULL, "Player movement speed.");
	REGISTER_CVAR2("gamezero_pl_boostMultiplier", &m_boostMultiplier, 2.0f, VF_NULL, "Player boost multiplier.");

	GetEntity()->SetUpdatePolicy(EEntityUpdatePolicy_Always);
}

void CMovementExtension::PostUpdate(float frameTime)
{
	if (auto *pInputExtension = GetEntity()->QueryComponent<CInputExtension>())
	{
		Quat viewRotation = pInputExtension->GetViewRotation();

		GetEntity()->SetRotation(viewRotation.GetNormalized());

		Vec3 vDeltaMovement = pInputExtension->GetDeltaMovement();
		bool bBoost = pInputExtension->IsBoosting();

		const float boostMultiplier = bBoost ? m_boostMultiplier : 1.0f;

		GetEntity()->SetPos(GetEntity()->GetWorldPos() + viewRotation * (vDeltaMovement * frameTime * boostMultiplier * m_movementSpeed));
	}
}
