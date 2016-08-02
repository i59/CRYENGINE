// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:   ScriptProxy.h
//  Version:     v1.00
//  Created:     18/5/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description:
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ScriptProxy_h__
#define __ScriptProxy_h__
#pragma once

// forward declarations.
class CEntityScript;
class CEntity;
struct IScriptTable;
struct SScriptState;

#include "EntityScript.h"

//////////////////////////////////////////////////////////////////////////
// Description:
//    CScriptComponent object handles all the interaction of the entity with
//    the entity script.
//////////////////////////////////////////////////////////////////////////
class CScriptComponent : public IEntityScriptComponent
{
public:

	CScriptComponent();
	~CScriptComponent();

	// IEntityComponent
	virtual void PostInitialize() override;

	virtual void ProcessEvent(const SEntityEvent& event) override;

	virtual void OnEntityReload(SEntitySpawnParams& params, XmlNodeRef entityNode) override;
	virtual void Update(SEntityUpdateContext& ctx) override;

	virtual void SerializeXML(XmlNodeRef& entityNode, bool bLoading, bool bFromInit) override;
	virtual void Serialize(TSerialize ser) override;

	virtual bool NeedSerialize() override;
	// ~IEntityComponent

	void InitializeScript(IEntityScript *pScript, IScriptTable *pPropertiesTable);

	//////////////////////////////////////////////////////////////////////////
	// IEntityScriptComponent implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void          SetScriptUpdateRate(float fUpdateEveryNSeconds) { m_fScriptUpdateRate = fUpdateEveryNSeconds; };
	virtual IScriptTable* GetScriptTable()                                { return m_pThis; };

	virtual void          CallEvent(const char* sEvent);
	virtual void          CallEvent(const char* sEvent, float fValue);
	virtual void          CallEvent(const char* sEvent, bool bValue);
	virtual void          CallEvent(const char* sEvent, const char* sValue);
	virtual void          CallEvent(const char* sEvent, EntityId nEntityId);
	virtual void          CallEvent(const char* sEvent, const Vec3& vValue);

	void                  CallInitEvent(bool bFromReload);

	virtual void          SendScriptEvent(int Event, IScriptTable* pParamters, bool* pRet = NULL);
	virtual void          SendScriptEvent(int Event, const char* str, bool* pRet = NULL);
	virtual void          SendScriptEvent(int Event, int nParam, bool* pRet = NULL);

	virtual void          ChangeScript(IEntityScript* pScript, IScriptTable *pScriptTable);
	//////////////////////////////////////////////////////////////////////////

	virtual void OnCollision(CEntity* pTarget, int matId, const Vec3& pt, const Vec3& n, const Vec3& vel, const Vec3& targetVel, int partId, float mass);
	virtual void OnPreparedFromPool();

	//////////////////////////////////////////////////////////////////////////
	// State Management public interface.
	//////////////////////////////////////////////////////////////////////////
	virtual bool GotoState(const char* sStateName);
	virtual bool GotoStateId(int nState) { return GotoState(nState); };
	bool         GotoState(int nState);
	bool         IsInState(const char* sStateName);
	bool         IsInState(int nState);
	const char*  GetState();
	int          GetStateId();
	void         RegisterForAreaEvents(bool bEnable);
	bool         IsRegisteredForAreaEvents() const;

	void         SerializeProperties(TSerialize ser);

	virtual void GetMemoryUsage(ICrySizer* pSizer) const;

private:
	SScriptState*  CurrentState() { return m_pScript->GetState(m_nCurrStateId); }
	void           CreateScriptTable(IScriptTable *pPropertiesTable);
	void           SetEventTargets(XmlNodeRef& eventTargets);
	IScriptSystem* GetIScriptSystem() const { return gEnv->pScriptSystem; }

	void           SerializeTable(TSerialize ser, const char* name);
	bool           HaveTable(const char* name);

private:
	CEntityScript* m_pScript;
	IScriptTable*  m_pThis;

	float          m_fScriptUpdateTimer;
	float          m_fScriptUpdateRate;

	// Cache Tables.
	SmartScriptTable m_hitTable;

	uint32           m_nCurrStateId           : 8;
	uint32           m_bUpdateScript          : 1;
	bool             m_bEnableSoundAreaEvents : 1;
};

#endif // __ScriptProxy_h__
