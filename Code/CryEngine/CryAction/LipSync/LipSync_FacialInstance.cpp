// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:   LipSync_FacialInstance.cpp
//  Version:     v1.00
//  Created:     2014-08-29 by Christian Werle.
//  Description: Automatic start of facial animation when a sound is being played back.
//               Legacy version that uses CryAnimation's FacialInstance.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "LipSync_FacialInstance.h"

//=============================================================================
//
// CLipSyncProvider_FacialInstance
//
//=============================================================================

CLipSyncProvider_FacialInstance::CLipSyncProvider_FacialInstance(EntityId entityId)
	: m_entityId(entityId)
{
}

void CLipSyncProvider_FacialInstance::RequestLipSync(IEntityAudioComponent* pProxy, const AudioControlId audioTriggerId, const ELipSyncMethod lipSyncMethod)
{
	// actually facial sequence is triggered in OnSoundEvent SOUND_EVENT_ON_PLAYBACK_STARTED of the CSoundProxy
	// when playback is started, it will start facial sequence as well
}

void CLipSyncProvider_FacialInstance::StartLipSync(IEntityAudioComponent* pProxy, const AudioControlId audioTriggerId, const ELipSyncMethod lipSyncMethod)
{
	if (lipSyncMethod != eLSM_None)
	{
		LipSyncWithSound(audioTriggerId);
	}
}

void CLipSyncProvider_FacialInstance::PauseLipSync(IEntityAudioComponent* pProxy, const AudioControlId audioTriggerId, const ELipSyncMethod lipSyncMethod)
{
	if (lipSyncMethod != eLSM_None)
	{
		LipSyncWithSound(audioTriggerId, true);
	}
}

void CLipSyncProvider_FacialInstance::UnpauseLipSync(IEntityAudioComponent* pProxy, const AudioControlId audioTriggerId, const ELipSyncMethod lipSyncMethod)
{
	if (lipSyncMethod != eLSM_None)
	{
		LipSyncWithSound(audioTriggerId);
	}
}

void CLipSyncProvider_FacialInstance::StopLipSync(IEntityAudioComponent* pProxy, const AudioControlId audioTriggerId, const ELipSyncMethod lipSyncMethod)
{
	if (lipSyncMethod != eLSM_None)
	{
		LipSyncWithSound(audioTriggerId, true);
	}
}

void CLipSyncProvider_FacialInstance::UpdateLipSync(IEntityAudioComponent* pProxy, const AudioControlId audioTriggerId, const ELipSyncMethod lipSyncMethod)
{
}

void CLipSyncProvider_FacialInstance::FullSerialize(TSerialize ser)
{
	ser.BeginGroup("LipSyncProvider_FacialInstance");
	ser.EndGroup();
}

void CLipSyncProvider_FacialInstance::LipSyncWithSound(const AudioControlId audioTriggerId, bool bStop /*= false*/)
{
	if (IEntity* pEntity = gEnv->pEntitySystem->GetEntity(m_entityId))
	{
		if (ICharacterInstance* pChar = pEntity->GetCharacter(0))
		{
			if (IFacialInstance* pFacial = pChar->GetFacialInstance())
			{
				pFacial->LipSyncWithSound(audioTriggerId, bStop);
			}
		}
	}
}

//=============================================================================
//
// CLipSync_FacialInstance
//
//=============================================================================

CLipSync_FacialInstance::~CLipSync_FacialInstance()
{
	if (auto *pAudioComponent = GetEntity()->QueryComponent<IEntityAudioComponent>())
	{
		REINST(add SetLipSyncProvider to interface)
			//pSoundProxy->SetLipSyncProvider(ILipSyncProviderPtr());
	}
}

void CLipSync_FacialInstance::InjectLipSyncProvider()
{
	IEntity* pEntity = GetEntity();
	auto &audioComponent = pEntity->AcquireExternalComponent<IEntityAudioComponent>();
	m_pLipSyncProvider.reset(new CLipSyncProvider_FacialInstance(pEntity->GetId()));
	REINST(add SetLipSyncProvider to interface)
	//pSoundProxy->SetLipSyncProvider(m_pLipSyncProvider);
}

void CLipSync_FacialInstance::GetMemoryUsage(ICrySizer* pSizer) const
{
	pSizer->Add(*this);
	if (m_pLipSyncProvider)
	{
		pSizer->Add(*m_pLipSyncProvider);
	}
}

void CLipSync_FacialInstance::PostInitialize()
{
	InjectLipSyncProvider();
}

void CLipSync_FacialInstance::OnEntityReload(SEntitySpawnParams& params, XmlNodeRef entityNode)
{
	InjectLipSyncProvider();
}

void CLipSync_FacialInstance::Serialize(TSerialize ser)
{
	ser.BeginGroup("LipSync_FacialInstance");

	bool bLipSyncProviderIsInjected = (m_pLipSyncProvider != NULL);
	ser.Value("bLipSyncProviderIsInjected", bLipSyncProviderIsInjected);
	if (bLipSyncProviderIsInjected && !m_pLipSyncProvider)
	{
		CRY_ASSERT(ser.IsReading());
		InjectLipSyncProvider();
	}
	if (m_pLipSyncProvider)
	{
		m_pLipSyncProvider->FullSerialize(ser);
	}

	ser.EndGroup();
}