// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#ifndef __MP_PATH_H__
#define __MP_PATH_H__


class CMPPath : public CEntityComponentConversionHelper<CMPPath>
{
public:
	DECLARE_COMPONENT("MPPath", 0x9C80789762884D95, 0xAA6CF7354D194FEB)

	virtual ~CMPPath() {}

	// IEntityComponent
	virtual bool Init(IGameObject *pGameObject);
	virtual void InitClient(int channelId) {};
	virtual void PostInit(IGameObject *pGameObject) {};
	virtual void PostInitClient(int channelId) {}
	virtual void Release();
	virtual void FullSerialize(TSerialize ser) {};
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) { return true; }
	virtual void PostSerialize() {}
	virtual void SerializeSpawnInfo(TSerialize ser) {}
	virtual ISerializableInfoPtr GetSpawnInfo() { return 0; }
	virtual void Update(SEntityUpdateContext &ctx) {};
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
};

#endif // __MP_PATH_H__