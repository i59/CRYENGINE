// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "AnimatedCharacterEventProxies.h"
#include "AnimatedCharacter.h"

#include "CryActionCVars.h"

#include <CryExtension/ClassWeaver.h>
#include <CryExtension/CryCreateClassInstance.h>

//////////////////////////////////////////////////////////////////////////

CAnimatedCharacterComponent_Base::CAnimatedCharacterComponent_Base()
	: m_pAnimCharacter(nullptr)
{
}

//////////////////////////////////////////////////////////////////////////
void CAnimatedCharacterComponent_Base::SetAnimatedCharacter(CAnimatedCharacter *pAnimCharacter)
{
	m_pAnimCharacter = pAnimCharacter;

	GetEntity()->PrePhysicsActivate(true);
}

//////////////////////////////////////////////////////////////////////////
void CAnimatedCharacterComponent_Base::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_PREPHYSICSUPDATE:
		OnPrePhysicsUpdate(event.fParam[0]);
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
CAnimatedCharacterComponent_PrepareAnimatedCharacterForUpdate::CAnimatedCharacterComponent_PrepareAnimatedCharacterForUpdate()
	: m_queuedRotation(IDENTITY)
	, m_hasQueuedRotation(false)
{
}

//////////////////////////////////////////////////////////////////////////
void CAnimatedCharacterComponent_PrepareAnimatedCharacterForUpdate::OnPrePhysicsUpdate(float)
{
	CRY_ASSERT(m_pAnimCharacter);

	if (CAnimationGraphCVars::Get().m_useQueuedRotation && m_hasQueuedRotation)
	{
		m_pEntity->SetRotation(m_queuedRotation, ENTITY_XFORM_USER | ENTITY_XFORM_NOT_REREGISTER);
		ClearQueuedRotation();
	}

	m_pAnimCharacter->PrepareAnimatedCharacterForUpdate();
}

//////////////////////////////////////////////////////////////////////////
void CAnimatedCharacterComponent_StartAnimProc::OnPrePhysicsUpdate(float elapsedTime)
{
	CRY_ASSERT(m_pAnimCharacter);

	m_pAnimCharacter->PrepareAndStartAnimProc();
}

//////////////////////////////////////////////////////////////////////////
void CAnimatedCharacterComponent_GenerateMoveRequest::OnPrePhysicsUpdate(float elapsedTime)
{
	CRY_ASSERT(m_pAnimCharacter);

	m_pAnimCharacter->GenerateMovementRequest();
}