// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:   LipSync_FacialInstance.h
//  Version:     v1.00
//  Created:     2014-08-29 by Christian Werle.
//  Description: Automatic start of facial animation when a sound is being played back.
//               Legacy version that uses CryAnimation's FacialInstance.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#pragma once

class CLipSyncProvider_FacialInstance : public ILipSyncProvider
{
public:
	explicit CLipSyncProvider_FacialInstance(EntityId entityId);

	// ILipSyncProvider
	virtual void RequestLipSync(IEntityAudioComponent* pProxy, const AudioControlId audioTriggerId, const ELipSyncMethod lipSyncMethod) override;
	virtual void StartLipSync(IEntityAudioComponent* pProxy, const AudioControlId audioTriggerId, const ELipSyncMethod lipSyncMethod) override;
	virtual void PauseLipSync(IEntityAudioComponent* pProxy, const AudioControlId audioTriggerId, const ELipSyncMethod lipSyncMethod) override;
	virtual void UnpauseLipSync(IEntityAudioComponent* pProxy, const AudioControlId audioTriggerId, const ELipSyncMethod lipSyncMethod) override;
	virtual void StopLipSync(IEntityAudioComponent* pProxy, const AudioControlId audioTriggerId, const ELipSyncMethod lipSyncMethod) override;
	virtual void UpdateLipSync(IEntityAudioComponent* pProxy, const AudioControlId audioTriggerId, const ELipSyncMethod lipSyncMethod) override;
	// ~ILipSyncProvider

	void FullSerialize(TSerialize ser);

private:
	void LipSyncWithSound(const AudioControlId audioTriggerId, bool bStop = false);
	EntityId m_entityId;
};
DECLARE_SHARED_POINTERS(CLipSyncProvider_FacialInstance);

class CLipSync_FacialInstance : public IEntityComponent
{
public:
	virtual ~CLipSync_FacialInstance();

	// IEntityComponent
	virtual void PostInitialize() override;
	virtual void OnEntityReload(SEntitySpawnParams& params, XmlNodeRef entityNode) override;

	virtual bool NeedSerialize() override { return true; }
	virtual void Serialize(TSerialize ser) override;

	virtual void GetMemoryUsage(ICrySizer* pSizer) const override;
	// ~IEntityComponent

private:
	void InjectLipSyncProvider();

private:
	CLipSyncProvider_FacialInstancePtr m_pLipSyncProvider;
};
