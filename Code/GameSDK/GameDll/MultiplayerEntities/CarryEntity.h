// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

/*************************************************************************
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Description: Carry entity, used for CTFFlag
  
 -------------------------------------------------------------------------
  History:
  - 03:06:2011: Created by Colin Gulliver

*************************************************************************/

#ifndef __CARRYENTITY_H__
#define __CARRYENTITY_H__

#include "NetworkedPhysicsEntity.h"

typedef CNetworkedPhysicsEntity TParent;

struct IAttachment;

class CCarryEntity :	public TParent
{
public:
	DECLARE_COMPONENT("EntityRenderComponent", 0x88CB1353BF7C4AD0, 0x8019762F3A416EFF)

	CCarryEntity();
	virtual ~CCarryEntity() {}

	// IEntityComponent
	virtual bool Init(IGameObject *pGameObject);
	virtual void GetMemoryUsage(ICrySizer *pSizer) const;
	virtual void PostInit(IGameObject *pGameObject);
	// ~IEntityComponent

	void SetSpawnedWeaponId(EntityId weaponId);
	void AttachTo(EntityId actorId);

private:
	void Attach(EntityId actorId);
	void Detach();

	typedef CryFixedStringT<8> TSmallString;

	TSmallString m_actionSuffix;
	TSmallString m_actionSuffixAG;

	uint32 m_playerTagCRC;

	EntityId m_spawnedWeaponId;
	EntityId m_attachedActorId;
	EntityId m_previousWeaponId;
	EntityId m_cachedLastItemId;

	QuatT m_oldAttachmentRelTrans;

	bool m_bSpawnedWeaponAttached;
};

#endif //__CARRYENTITY_H__

