// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: Shake entity

-------------------------------------------------------------------------
History:
- 27:04:2006: Created by Marco Koegler

*************************************************************************/
#ifndef __SHAKE_H__
#define __SHAKE_H__
#pragma once

class CShake : public CEntityComponentConversionHelper<CShake>
{
public:
	DECLARE_COMPONENT("Shake", 0xCC8DB2F2AD9E4366, 0xB4566C6824A3C0D6)

	CShake();
	virtual ~CShake();

	// IEntityComponent
	virtual bool Init(IGameObject *pGameObject);
	virtual void InitClient(int channelId) {};
	virtual void PostInit(IGameObject *pGameObject);
	virtual void PostInitClient(int channelId) {};
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params ) {}
	virtual bool GetEntityPoolSignature( TSerialize signature );
	virtual void Release();
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int pflags) { return true; }
	virtual void FullSerialize(TSerialize ser);
	virtual void PostSerialize() {}
	virtual void SerializeSpawnInfo( TSerialize ser ) {}
	virtual ISerializableInfoPtr GetSpawnInfo() {return 0;}
	virtual void Update( SEntityUpdateContext &ctx);
	virtual void PostRemoteSpawn() {};
	virtual void HandleEvent( const SGameObjectEvent &);
	virtual void ProcessEvent(const SEntityEvent &);
	virtual void SetChannelId(uint16 id) {}
	virtual void SetAuthority(bool auth);
	virtual void GetMemoryUsage(ICrySizer *pSizer) const { pSizer->Add(*this); }
	
	// ~IEntityComponent


protected:
	float	m_radius;
	float	m_shake;
};

#endif //__SHAKE_H__
