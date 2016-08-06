// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

/*************************************************************************
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Description: HUD Tactical override entity
  
 -------------------------------------------------------------------------
  History:
  - 13:12:2012: Created by Dean Claassen

*************************************************************************/

#ifndef __HUD_TACTICALOVERRIDE_ENTITY_H__
#define __HUD_TACTICALOVERRIDE_ENTITY_H__

// CTacticalOverrideEntity

class CTacticalOverrideEntity : public CEntityComponentConversionHelper<CTacticalOverrideEntity>
{
public:
	CTacticalOverrideEntity();
	virtual ~CTacticalOverrideEntity();

	// IEntityComponent
	virtual bool Init(IGameObject *pGameObject);
	virtual void InitClient(int channelId);
	virtual void PostInit(IGameObject *pGameObject);
	virtual void PostInitClient(int channelId) {}
	virtual void Release();
	virtual void FullSerialize(TSerialize ser) {};
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) { return true; }
	virtual void PostSerialize() {}
	virtual void SerializeSpawnInfo(TSerialize ser) {}
	virtual ISerializableInfoPtr GetSpawnInfo() { return 0; }
	virtual void PostRemoteSpawn() {}
	virtual void HandleEvent(const SGameObjectEvent& details) {}
	virtual void ProcessEvent(const SEntityEvent& details);
	virtual void SetChannelId(uint16 id) {}
	virtual void SetAuthority(bool auth) {}
	virtual void GetMemoryUsage(ICrySizer *pSizer) const;
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params ) {}
	virtual bool GetEntityPoolSignature( TSerialize signature );
	// ~IEntityComponent

protected:
	bool m_bMappedToParent;
};

#endif // __HUD_TACTICALOVERRIDE_ENTITY_H__

