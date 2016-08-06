// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:   BoidsProxy.cpp
//  Version:     v1.00
//  Created:     2/10/2004 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include <CryEntitySystem/IEntitySystem.h>
#include "BoidsProxy.h"
#include "Flock.h"
#include <CryNetwork/ISerialize.h>

//////////////////////////////////////////////////////////////////////////
CBoidsProxy::CBoidsProxy()
	: m_pFlock(NULL)
	, m_playersInCount(0)
{
}

//////////////////////////////////////////////////////////////////////////
CBoidsProxy::~CBoidsProxy()
{
	if (m_pFlock)
		delete m_pFlock;
}

//////////////////////////////////////////////////////////////////////////
void CBoidsProxy::PostInitialize()
{
	// Make sure render proxy also exist.
	GetEntity()->AcquireExternalComponent<IEntityRenderComponent>();

	SetUpdatePolicy(EEntityUpdatePolicy_InRange);
}

//////////////////////////////////////////////////////////////////////////
void CBoidsProxy::OnEntityReload(SEntitySpawnParams& params, XmlNodeRef entityNode)
{
	if (m_pFlock)
		m_pFlock->SetPos(GetEntity()->GetWorldPos());
	
	// Make sure render and trigger proxy also exist.
	GetEntity()->AcquireExternalComponent<IEntityRenderComponent>();
	m_playersInCount = 0;
}

//////////////////////////////////////////////////////////////////////////
void CBoidsProxy::Update( SEntityUpdateContext &ctx )
{
	if (m_pFlock)
		m_pFlock->Update( ctx.pCamera );
}

//////////////////////////////////////////////////////////////////////////
void CBoidsProxy::ProcessEvent(const SEntityEvent &event )
{
	switch (event.event) {
	case ENTITY_EVENT_XFORM:
		if (m_pFlock)
			m_pFlock->SetPos( m_pEntity->GetWorldPos() );
		break;
	case ENTITY_EVENT_PRE_SERIALIZE:
		if (m_pFlock)
			m_pFlock->DeleteEntities(true);
		break;
	case ENTITY_EVENT_ENTERAREA:
		OnTrigger(true,event);
		break;
	case ENTITY_EVENT_LEAVEAREA:
		OnTrigger(false,event);
		break;
	case ENTITY_EVENT_RESET:
		if (m_pFlock)
			m_pFlock->Reset();
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
void CBoidsProxy::Serialize( TSerialize ser )
{
}

//////////////////////////////////////////////////////////////////////////
void CBoidsProxy::SetFlock( CFlock *pFlock )
{
	m_pFlock = pFlock;

	if (!pFlock)
		return;
	
	// Update trigger based on new visibility distance settings.
	float fMaxDist = pFlock->GetMaxVisibilityDistance();

	/*
	CTriggerProxy *pTrigger = (CTriggerProxy*)m_pEntity->CreateProxy( ENTITY_PROXY_TRIGGER );
	if (!pTrigger)
		return;

	AABB bbox;
	bbox.min = -Vec3(fMaxDist,fMaxDist,fMaxDist);
	bbox.max = Vec3(fMaxDist,fMaxDist,fMaxDist);
	pTrigger->SetTriggerBounds( bbox );
	*/

	auto &areaComponent = GetEntity()->AcquireExternalComponent<IEntityAreaComponent>();

	areaComponent.SetFlags( areaComponent.GetFlags() & IEntityAreaComponent::FLAG_NOT_SERIALIZE );
	areaComponent.SetSphere( Vec3(0,0,0),fMaxDist );
	if(gEnv->pEntitySystem->EntitiesUseGUIDs())
		areaComponent.AddEntity( m_pEntity->GetGuid() );
	else
		areaComponent.AddEntity( m_pEntity->GetId() ); // add itself.

}

//////////////////////////////////////////////////////////////////////////
void CBoidsProxy::OnTrigger( bool bEnter, const SEntityEvent &event )
{
	EntityId whoId = (EntityId)event.nParam[0];
	IEntity *pEntity = gEnv->pEntitySystem->GetEntity(whoId);

	if (pEntity)
	{
		if (pEntity->GetFlags() & ENTITY_FLAG_LOCAL_PLAYER)
		{
			if (bEnter)
				m_playersInCount++;
			else
				m_playersInCount--;

			if (m_playersInCount == 1)
			{
				// Activates entity when player is nearby.
				if (m_pFlock)
					m_pFlock->SetEnabled(true);
			}
			if (m_playersInCount <= 0)
			{
				// Activates entity when player is nearby.
				m_playersInCount = 0;
				if (m_pFlock)
					m_pFlock->SetEnabled(false);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
CBoidObjectProxy::CBoidObjectProxy()
	: m_pBoid(NULL)
	, m_pEntity(NULL)
{
}

//////////////////////////////////////////////////////////////////////////
CBoidObjectProxy::~CBoidObjectProxy()
{
}

void CBoidObjectProxy::PostInitialize()
{
	EnableEvent(ENTITY_EVENT_DONE, 0, true);
	EnableEvent(ENTITY_EVENT_COLLISION, 0, true);
}

//////////////////////////////////////////////////////////////////////////
void CBoidObjectProxy::ProcessEvent(const SEntityEvent &event )
{
	if (m_pBoid)
		m_pBoid->OnEntityEvent( event );
}
