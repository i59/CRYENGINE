// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#ifndef __DANGEROUS_RIGID_BODY_H__
#define __DANGEROUS_RIGID_BODY_H__

class CDangerousRigidBody : public CEntityComponentConversionHelper<CDangerousRigidBody>
{
public:
	DECLARE_COMPONENT("EntityRenderComponent", 0xFCBCFC7086354457, 0x8B12DAD3E6C23EC4)

	static const NetworkAspectType ASPECT_DAMAGE_STATUS	= eEA_GameServerC;
	
	static int sDangerousRigidBodyHitTypeId;

	CDangerousRigidBody();

	// IEntityComponent
	virtual bool Init(IGameObject *pGameObject);
	virtual void InitClient(int channelId);
	virtual void PostInit(IGameObject *pGameObject) {}
	virtual void PostInitClient(int channelId) {}
	virtual void Release();
	virtual void FullSerialize(TSerialize ser) {};
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags);
	virtual void PostSerialize() {}
	virtual void SerializeSpawnInfo(TSerialize ser) {}
	virtual ISerializableInfoPtr GetSpawnInfo() { return 0; }
	virtual void Update(SEntityUpdateContext &ctx) {};
	virtual void PostRemoteSpawn() {}
	virtual void HandleEvent(const SGameObjectEvent& event) {};
	virtual void ProcessEvent(const SEntityEvent& event);
	virtual void SetChannelId(uint16 id) {}
	virtual void SetAuthority(bool auth) {}
	virtual void GetMemoryUsage(ICrySizer *pSizer) const;
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params ) {};
	virtual bool GetEntityPoolSignature( TSerialize signature );
	// IEntityComponent

	void SetIsDangerous(bool isDangerous, EntityId triggerPlayerId);

private:
	void Reset();

	float	m_damageDealt;
	float m_lastHitTime;
	float m_timeBetweenHits;
	bool m_dangerous;
	bool m_friendlyFireEnabled;
	uint8 m_activatorTeam;
};

#endif //__DANGEROUS_RIGID_BODY_H__
