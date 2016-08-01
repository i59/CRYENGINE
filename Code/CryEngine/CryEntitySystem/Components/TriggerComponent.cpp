// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:   TriggerProxy.h
//  Version:     v1.00
//  Created:     5/12/2005 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description:
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TriggerComponent.h"
#include <CryNetwork/ISerialize.h>
#include "ProximityTriggerSystem.h"

//////////////////////////////////////////////////////////////////////////
CTriggerComponent::CTriggerComponent()
	: m_forwardingEntity(0)
	, m_pProximityTrigger(NULL)
	, m_aabb(AABB::RESET)
{
}

//////////////////////////////////////////////////////////////////////////
CTriggerComponent::~CTriggerComponent()
{
	GetTriggerSystem()->RemoveTrigger(m_pProximityTrigger);
}

//////////////////////////////////////////////////////////////////////////
void CTriggerComponent::Initialize(IEntity &entity)
{
	IEntityComponent::Initialize(entity);

	m_pProximityTrigger = GetTriggerSystem()->CreateTrigger();
	m_pProximityTrigger->id = m_pEntity->GetId();

	Reset();
}

//////////////////////////////////////////////////////////////////////////
CEntity *CTriggerComponent::GetCEntity() const
{
	return static_cast<CEntity *>(m_pEntity);
}

//////////////////////////////////////////////////////////////////////////
void CTriggerComponent::Reload(SEntitySpawnParams& params, XmlNodeRef entityNode)
{
	assert(m_pProximityTrigger);
	if (m_pProximityTrigger)
	{
		m_pProximityTrigger->id = m_pEntity->GetId();
	}

	Reset();
}

//////////////////////////////////////////////////////////////////////////
void CTriggerComponent::Reset()
{
	GetCEntity()->m_bTrigger = true;
	m_forwardingEntity = 0;

	// Release existing proximity entity if present, triggers should not trigger themself.
	if (GetCEntity()->m_pProximityEntity)
	{
		GetTriggerSystem()->RemoveEntity(GetCEntity()->m_pProximityEntity);
		GetCEntity()->m_pProximityEntity = 0;
	}

	EnableEvent(ENTITY_EVENT_XFORM, 0, true);
	EnableEvent(ENTITY_EVENT_ENTERAREA, 0, true);
	EnableEvent(ENTITY_EVENT_LEAVEAREA, 0, true);
	EnableEvent(ENTITY_EVENT_PRE_SERIALIZE, 0, true);
	EnableEvent(ENTITY_EVENT_POST_SERIALIZE, 0, true);
}

//////////////////////////////////////////////////////////////////////////
void CTriggerComponent::Update(SEntityUpdateContext& ctx)
{
}

//////////////////////////////////////////////////////////////////////////
void CTriggerComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_XFORM:
		OnMove();
		break;
	case ENTITY_EVENT_ENTERAREA:
	case ENTITY_EVENT_LEAVEAREA:
		if (m_forwardingEntity)
		{
			IEntity* pEntity = gEnv->pEntitySystem->GetEntity(m_forwardingEntity);
			if (pEntity && (pEntity != this->GetEntity()))
			{
				pEntity->SendEvent(event);
			}
		}
		break;
	case ENTITY_EVENT_PRE_SERIALIZE:
		break;
	case ENTITY_EVENT_POST_SERIALIZE:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CTriggerComponent::NeedSerialize()
{
	return true;
};

//////////////////////////////////////////////////////////////////////////
void CTriggerComponent::Serialize(TSerialize ser)
{
	if (ser.GetSerializationTarget() != eST_Network)
	{
		if (ser.BeginOptionalGroup("TriggerProxy", true))
		{
			ser.Value("BoxMin", m_aabb.min);
			ser.Value("BoxMax", m_aabb.max);
			ser.EndGroup();
		}

		if (ser.IsReading())
			OnMove();
	}
}

//////////////////////////////////////////////////////////////////////////
void CTriggerComponent::OnMove(bool invalidateAABB)
{
	Vec3 pos = m_pEntity->GetWorldPos();
	AABB box = m_aabb;
	box.min += pos;
	box.max += pos;
	GetTriggerSystem()->MoveTrigger(m_pProximityTrigger, box, invalidateAABB);
}

//////////////////////////////////////////////////////////////////////////
void CTriggerComponent::InvalidateTrigger()
{
	OnMove(true);
}

//////////////////////////////////////////////////////////////////////////
void CTriggerComponent::SetAABB(const AABB& aabb)
{
	m_aabb = aabb;
	OnMove();
}
