// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

/*************************************************************************
-------------------------------------------------------------------------
$Id: ReplayObject.h$
$DateTime$
Description: A replay entity spawned during KillCam replay

-------------------------------------------------------------------------
History:
- 03/19/2010 09:15:00: Created by Martin Sherburn

*************************************************************************/

#ifndef __REPLAYOBJECT_H__
#define __REPLAYOBJECT_H__

#include <ICryMannequin.h>

class CReplayObjectAction : public TAction<SAnimationContext>
{
private:
	typedef TAction<SAnimationContext> BaseClass;
public:
	CReplayObjectAction(FragmentID fragID, const TagState &fragTags, uint32 optionIdx, bool trumpsPrevious, const int priority );
	virtual EPriorityComparison ComparePriority(const IAction &actionCurrent) const;
private:
	bool m_trumpsPrevious;
};

class CReplayItemList
{
public:
	CReplayItemList(){}
	~CReplayItemList(){}
	void AddItem( const EntityId itemId );
	void OnActionControllerDeleted();

private:
	typedef std::vector<EntityId> TItemVec;
	TItemVec m_items;
};

class CReplayObject : public CEntityComponentConversionHelper<CReplayObject>
{
public:
	DECLARE_COMPONENT("ReplayObject", 0x977B54B8FCDE438B, 0x8DD1E32B1E71CEA2)

	CReplayObject();
	virtual ~CReplayObject();

	// IEntityComponent
	virtual bool Init(IGameObject *pGameObject);
	virtual void InitClient(int channelId) {}
	virtual void PostInit(IGameObject *pGameObject) {}
	virtual void PostInitClient(int channelId) {}
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params ) {}
	virtual bool GetEntityPoolSignature( TSerialize signature );
	virtual void Release() { delete this; }
	virtual void FullSerialize( TSerialize ser ) {}
	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, uint8 profile, int flags ) { return true; }
	virtual void PostSerialize() {}
	virtual void SerializeSpawnInfo( TSerialize ser ) {}
	virtual ISerializableInfoPtr GetSpawnInfo() {return 0;}
	virtual void Update( SEntityUpdateContext &ctx) {}
	virtual void PostRemoteSpawn() {}
	virtual void HandleEvent( const SGameObjectEvent &) {}
	virtual void ProcessEvent(const SEntityEvent &) {}
	virtual void SetChannelId(uint16 id) {}
	virtual void SetAuthority(bool auth) {}
	virtual void GetMemoryUsage(ICrySizer * s) const {}
	// ~IEntityComponent

	void SetTimeSinceSpawn(float time) { assert(time >= 0); m_timeSinceSpawn = time; }

protected:
	float m_timeSinceSpawn;
};

#endif //!__REPLAYOBJECT_H__
