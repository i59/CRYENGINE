// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:   BoidsProxy.h
//  Version:     v1.00
//  Created:     2/10/2004 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __BoidsProxy_h__
#define __BoidsProxy_h__
#pragma once

#include <CryEntitySystem/IEntityComponent.h>

class CFlock;
class CBoidObject;

//////////////////////////////////////////////////////////////////////////
// Description:
//    Handles sounds in the entity.
//////////////////////////////////////////////////////////////////////////
struct CBoidsProxy : public IEntityComponent
{
	DECLARE_COMPONENT("BoidsProxy", 0xF594D3A2795742B3, 0xACBCAAC041BF11E6)

	CBoidsProxy();
	virtual ~CBoidsProxy();

	// IEntityComponent
	virtual void PostInitialize() override;
	virtual void OnEntityReload(SEntitySpawnParams& params, XmlNodeRef entityNode) override;

	virtual void Release() override { delete this; }

	virtual	void Update(SEntityUpdateContext &ctx) override;
	virtual	void ProcessEvent(const SEntityEvent &event) override;

	virtual void Serialize(TSerialize ser) override;
	// ~IEntityComponent

	//////////////////////////////////////////////////////////////////////////
	void SetFlock( CFlock *pFlock );
	CFlock* GetFlock() { return m_pFlock; }
	void OnTrigger( bool bEnter, const SEntityEvent &event );

	virtual void GetMemoryUsage(ICrySizer *pSizer )const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_pFlock);
	}
private:
	void OnMove();

private:
	// Flock of items.
	CFlock *m_pFlock;

	int m_playersInCount;
};

//////////////////////////////////////////////////////////////////////////
// Description:
//    Handles sounds in the entity.
//////////////////////////////////////////////////////////////////////////
struct CBoidObjectProxy : public IEntityComponent
{
	DECLARE_COMPONENT("EntityPhysicsComponent", 0x831620AEA44D475C, 0x99C33C7C0B394F51)

	CBoidObjectProxy();
	virtual ~CBoidObjectProxy();
	IEntity* GetEntity() const { return m_pEntity; };

	// IEntityComponent
	virtual void PostInitialize() override;

	virtual void Release() override { delete this; }

	virtual	void ProcessEvent(const SEntityEvent &event) override;
	// ~IEntityComponent

	//////////////////////////////////////////////////////////////////////////
	void SetBoid( CBoidObject *pBoid ) { m_pBoid = pBoid; };
	CBoidObject* GetBoid() { return m_pBoid; }

	virtual void GetMemoryUsage(ICrySizer *pSizer )const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_pBoid);
	}
private:
	//////////////////////////////////////////////////////////////////////////
	// Private member variables.
	//////////////////////////////////////////////////////////////////////////
	// Host entity.
	IEntity *m_pEntity;
	// Host Flock.
	CBoidObject *m_pBoid;
};

#endif //__BoidsProxy_h__
