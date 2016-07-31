// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "stdafx.h"
#include "DynamicResponseComponent.h"
#include <CryDynamicResponseSystem/IDynamicResponseSystem.h>
#include <CryNetwork/ISerialize.h>

//////////////////////////////////////////////////////////////////////////
CDynamicResponseComponent::CDynamicResponseComponent()
	: m_pResponseActor(nullptr)
{
}

CDynamicResponseComponent::~CDynamicResponseComponent()
{
	gEnv->pDynamicResponseSystem->ReleaseResponseActor(m_pResponseActor);
}

//////////////////////////////////////////////////////////////////////////
void CDynamicResponseComponent::Initialize(IEntity &entity)
{
	IEntityComponent::Initialize(entity);

	const char* szEntityName = m_pEntity->GetName();
	m_pResponseActor = gEnv->pDynamicResponseSystem->GetResponseActor(szEntityName);
	if (m_pResponseActor)
	{
		CryWarning(VALIDATOR_MODULE_DRS, VALIDATOR_ERROR_DBGBRK, "DrsActor with name '%s' already exists. Actors need to have unique names to be referenced correctly", szEntityName);
	}
	else
	{
		m_pResponseActor = gEnv->pDynamicResponseSystem->CreateResponseActor(szEntityName, m_pEntity->GetId());
	}
	SET_DRS_USER_SCOPED("DrsProxy Initialize");
	m_pResponseActor->GetLocalVariables()->SetVariableValue("Name", CHashedString(szEntityName));
}

//////////////////////////////////////////////////////////////////////////
void CDynamicResponseComponent::ProcessEvent(const SEntityEvent& event)
{
	if (event.event == ENTITY_EVENT_RESET)
	{
		SET_DRS_USER_SCOPED("DrsProxy Reset Event");
		m_pResponseActor->GetLocalVariables()->SetVariableValue("Name", m_pResponseActor->GetName());
	}
}

//////////////////////////////////////////////////////////////////////////
DRS::IVariableCollection* CDynamicResponseComponent::GetLocalVariableCollection() const
{
	CRY_ASSERT_MESSAGE(m_pResponseActor, "DRS Proxy without an Actor detected. Should never happen.");
	return m_pResponseActor->GetLocalVariables();
}

//////////////////////////////////////////////////////////////////////////
DRS::IResponseActor* CDynamicResponseComponent::GetResponseActor() const
{
	CRY_ASSERT_MESSAGE(m_pResponseActor, "DRS Proxy without an Actor detected. Should never happen.");
	return m_pResponseActor;
}
