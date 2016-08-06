// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#ifndef _LIGHTNING_ARC_H_
#define _LIGHTNING_ARC_H_

#pragma once


#include "Effects/GameEffects/LightningGameEffect.h"



class CLightningArc : public CEntityComponentConversionHelper<CLightningArc>
{
public:
	DECLARE_COMPONENT("LightningArc", 0xDD0A3E13D8B7473F, 0xAA5BBF8D10DEF3E0)

	CLightningArc();

	virtual void GetMemoryUsage(ICrySizer *pSizer) const;
	virtual bool Init(IGameObject* pGameObject);
	virtual void PostInit( IGameObject * pGameObject );
	virtual void InitClient(int channelId);
	virtual void PostInitClient(int channelId);
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual bool GetEntityPoolSignature( TSerialize signature );
	virtual void Release();
	virtual void FullSerialize( TSerialize ser );
	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, uint8 profile, int pflags );
	virtual void PostSerialize();
	virtual void SerializeSpawnInfo( TSerialize ser );
	virtual ISerializableInfoPtr GetSpawnInfo();
	virtual void Update( SEntityUpdateContext& ctx );
	virtual void HandleEvent( const SGameObjectEvent& event );
	virtual void ProcessEvent(const SEntityEvent& event );	
	virtual void SetChannelId(uint16 id);
	virtual void SetAuthority( bool auth );
	virtual void PostUpdate( float frameTime );
	virtual void PostRemoteSpawn();

	void TriggerSpark();
	void Enable(bool enable);
	void ReadLuaParameters();

private:
	void Reset(bool jumpingIntoGame);

	const char* m_lightningPreset;
	float m_delay;
	float m_delayVariation;
	float m_timer;
	bool m_enabled;
	bool m_inGameMode;
};



#endif
