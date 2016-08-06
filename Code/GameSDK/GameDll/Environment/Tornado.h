// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: Tornado entity

-------------------------------------------------------------------------
History:
- 21:02:2006: Created by Marco Koegler

*************************************************************************/
#ifndef __TORNADO_H__
#define __TORNADO_H__
#pragma once


class CFlowTornadoWander;
struct IGroundEffect;

class CTornado : public CEntityComponentConversionHelper<CTornado>
{
public:
	DECLARE_COMPONENT("EntityRenderComponent", 0xC0D2F46A38544EB1, 0xAD97A11770271211)

	CTornado();
	virtual ~CTornado();

	// IEntityComponent
	virtual bool Init(IGameObject *pGameObject);
	virtual void InitClient(int channelId) {};
	virtual void PostInit(IGameObject *pGameObject);
	virtual void PostInitClient(int channelId) {};
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params ) {}
	virtual bool GetEntityPoolSignature( TSerialize signature );
	virtual void Release();
	virtual void FullSerialize( TSerialize ser );
	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, uint8 profile, int flags );
	virtual void PostSerialize() {}
	virtual void SerializeSpawnInfo( TSerialize ser ) {}
	virtual ISerializableInfoPtr GetSpawnInfo() {return 0;}
	virtual void Update( SEntityUpdateContext &ctx);
	virtual void PostRemoteSpawn() {};
	virtual void HandleEvent( const SGameObjectEvent &);
	virtual void ProcessEvent(const SEntityEvent &);
	virtual void SetChannelId(uint16 id) {}
	virtual void SetAuthority(bool auth);
	virtual void GetMemoryUsage(ICrySizer *pSizer) const;
	
	// ~IEntityComponent

	void	SetTarget(IEntity *pTargetEntity, CFlowTornadoWander *pCallback);

	bool Reset();

protected:

	bool	UseFunnelEffect(const char* effectName);
	void	UpdateParticleEmitters();
	void	UpdateTornadoSpline();
	void	UpdateFlow();
	void	OutputDistance();

protected:
	static const NetworkAspectType POSITION_ASPECT = eEA_GameServerStatic;

	IPhysicalEntity*	m_pPhysicalEntity;

	// appearance of tornado
	IParticleEffect*	m_pFunnelEffect;
	IParticleEffect*	m_pCloudConnectEffect;
	IParticleEffect*	m_pTopEffect;
	IGroundEffect*		m_pGroundEffect;
	float							m_cloudHeight;
	bool							m_isOnWater;
	bool							m_isInAir;
	float							m_radius;
	int								m_curMatID;

	// spline
	Vec3							m_points[4];
	Vec3							m_oldPoints[4];

	// wandering
	Vec3							m_wanderDir;
	float							m_wanderSpeed;
	Vec3							m_currentPos;

	// target
	IEntity*						m_pTargetEntity;	// the tornado will try to reach this entity
	CFlowTornadoWander*	m_pTargetCallback;

	//
	float m_nextEntitiesCheck;

	float m_spinImpulse;
	float m_attractionImpulse;
	float m_upImpulse;

	std::vector<int> m_spinningEnts;
};

#endif //__TORNADO_H__
