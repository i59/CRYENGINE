// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "RuntimeAreaObject.h"

CRuntimeAreaObject::TAudioControlMap CRuntimeAreaObject::m_audioControls;

CRuntimeAreaObject::~CRuntimeAreaObject()
{
	// Stop all of the currently playing sounds controlled by this RuntimeAreaObject instance.
	for (TEntitySoundsMap::iterator iEntityData = m_activeEntitySounds.begin(),
		iEntityDataEnd = m_activeEntitySounds.end(); iEntityData != iEntityDataEnd; ++iEntityData)
	{
		StopEntitySounds(iEntityData->first, iEntityData->second);
	}
}

///////////////////////////////////////////////////////////////////////////
void CRuntimeAreaObject::PostInitialize()
{
	EnableEvent(ENTITY_EVENT_ENTERAREA, 0, true);
	EnableEvent(ENTITY_EVENT_LEAVEAREA, 0, true);
	EnableEvent(ENTITY_EVENT_MOVEINSIDEAREA, 0, true);
}

///////////////////////////////////////////////////////////////////////////
void CRuntimeAreaObject::ProcessEvent(const SEntityEvent& entityEvent)
{
	switch (entityEvent.event)
	{
	case ENTITY_EVENT_ENTERAREA:
		{
			EntityId const nEntityID = static_cast<EntityId>(entityEvent.nParam[0]);

			IEntity* const pEntity = gEnv->pEntitySystem->GetEntity(nEntityID);
			if ((pEntity != NULL) && ((pEntity->GetFlagsExtended() & ENTITY_FLAG_EXTENDED_CAN_COLLIDE_WITH_MERGED_MESHES) != 0))
			{
				TAudioParameterMap cNewEntityParamMap;

				UpdateParameterValues(pEntity, cNewEntityParamMap);

				m_activeEntitySounds.insert(
				  std::pair<EntityId, TAudioParameterMap>(static_cast<EntityId>(nEntityID), cNewEntityParamMap));
			}

			break;
		}
	case ENTITY_EVENT_LEAVEAREA:
		{
			EntityId const nEntityID = static_cast<EntityId>(entityEvent.nParam[0]);

			TEntitySoundsMap::iterator iFoundPair = m_activeEntitySounds.find(nEntityID);
			if (iFoundPair != m_activeEntitySounds.end())
			{
				StopEntitySounds(iFoundPair->first, iFoundPair->second);
				m_activeEntitySounds.erase(nEntityID);
			}

			break;
		}
	case ENTITY_EVENT_MOVEINSIDEAREA:
		{
			EntityId const nEntityID = static_cast<EntityId>(entityEvent.nParam[0]);
			TEntitySoundsMap::iterator iFoundPair = m_activeEntitySounds.find(nEntityID);

			if (iFoundPair != m_activeEntitySounds.end())
			{
				IEntity* const pEntity = gEnv->pEntitySystem->GetEntity(nEntityID);
				if ((pEntity != NULL) && ((pEntity->GetFlagsExtended() & ENTITY_FLAG_EXTENDED_CAN_COLLIDE_WITH_MERGED_MESHES) != 0))
				{
					UpdateParameterValues(pEntity, iFoundPair->second);
				}
			}

			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void CRuntimeAreaObject::GetMemoryUsage(ICrySizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
}

//////////////////////////////////////////////////////////////////////////
void CRuntimeAreaObject::UpdateParameterValues(IEntity* const pEntity, TAudioParameterMap& paramMap)
{
	static float const fParamEpsilon = 0.001f;
	static float const fMaxDensity = 256.0f;

	auto &audioComponent = pEntity->AcquireExternalComponent<IEntityAudioComponent>();

	ISurfaceType* aSurfaceTypes[MMRM_MAX_SURFACE_TYPES];
	memset(aSurfaceTypes, 0x0, sizeof(aSurfaceTypes));

	float aDensities[MMRM_MAX_SURFACE_TYPES];
	memset(aDensities, 0x0, sizeof(aDensities));

	gEnv->p3DEngine->GetIMergedMeshesManager()->QueryDensity(pEntity->GetPos(), aSurfaceTypes, aDensities);

	for (int i = 0; i < MMRM_MAX_SURFACE_TYPES && (aSurfaceTypes[i] != NULL); ++i)
	{
		float const fNewParamValue = aDensities[i] / fMaxDensity;
		TSurfaceCRC const nSurfaceCrc = CCrc32::ComputeLowercase(aSurfaceTypes[i]->GetName());

		TAudioParameterMap::iterator iSoundPair = paramMap.find(nSurfaceCrc);
		if (iSoundPair == paramMap.end())
		{
			if (fNewParamValue > 0.0f)
			{
				// The sound for this surface is not yet playing on this entity, needs to be started.
				TAudioControlMap::const_iterator const iAudioControls = m_audioControls.find(nSurfaceCrc);
				if (iAudioControls != m_audioControls.end())
				{
					SAudioControls const& rAudioControls = iAudioControls->second;

					audioComponent.SetRtpcValue(rAudioControls.audioRtpcId, fNewParamValue);
					audioComponent.ExecuteTrigger(rAudioControls.audioTriggerId);

					paramMap.insert(
						std::pair<TSurfaceCRC, SAreaSoundInfo>(
						nSurfaceCrc,
						SAreaSoundInfo(rAudioControls, fNewParamValue)));
				}
			}
		}
		else
		{
			SAreaSoundInfo& oSoundInfo = iSoundPair->second;
			if (fabs_tpl(fNewParamValue - oSoundInfo.parameter) >= fParamEpsilon)
			{
				oSoundInfo.parameter = fNewParamValue;
				audioComponent.SetRtpcValue(oSoundInfo.audioControls.audioRtpcId, oSoundInfo.parameter);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void CRuntimeAreaObject::StopEntitySounds(EntityId const entityId, TAudioParameterMap& paramMap)
{
	IEntity* const pEntity = gEnv->pEntitySystem->GetEntity(entityId);
	if (pEntity != NULL)
	{
		auto &audioComponent = pEntity->AcquireExternalComponent<IEntityAudioComponent>();

		for (TAudioParameterMap::const_iterator iSoundPair = paramMap.begin(), iSoundPairEnd = paramMap.end(); iSoundPair != iSoundPairEnd; ++iSoundPair)
		{
			audioComponent.StopTrigger(iSoundPair->second.audioControls.audioTriggerId);
			audioComponent.SetRtpcValue(iSoundPair->second.audioControls.audioRtpcId, 0.0f);
		}

		paramMap.clear();
	}
}