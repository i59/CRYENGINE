// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Description:

   -------------------------------------------------------------------------
   History:
   - 6:9:2004   12:44 : Created by Márcio Martins

*************************************************************************/
#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <IViewSystem.h>
#include <IActionMapManager.h>
#include <CryAction/IGameObject.h>
#include <CryCore/BitFiddling.h>

class CGameObject;
struct SEntitySchedulingProfiles;

struct SBasicSpawnParams : public ISerializable
{
	string name;
	uint16 classId;
	Vec3   pos;
	Quat   rotation;
	Vec3   scale;
	bool   bClientActor;
	uint16 nChannelId;
	uint32 flags;

	virtual void SerializeWith(TSerialize ser)
	{
		if (ser.GetSerializationTarget() == eST_Network)
		{
			ser.Value("name", name, 'sstr');
			ser.Value("classId", classId, 'clas');
			ser.Value("pos", pos, 'spos');
			bool bHasScale = false;
			Vec3 vScale(1.0f, 1.0f, 1.0f);

			if (ser.IsWriting())
			{
				bHasScale = (scale.x != 1.0f) || (scale.y != 1.0f) || (scale.z != 1.0f);
				vScale = scale;
			}

			//As this is used in an RMI, we can branch on bHasScale and save ourselves 96bits in the
			//	vast majority of cases, at the cost of a single bit.
			ser.Value("hasScale", bHasScale, 'bool');

			if (bHasScale)
			{
				//We can't just use a scalar here. Some (very few) objects have non-uniform scaling.
				ser.Value("scale", vScale, 'sscl');
			}

			scale = vScale;

			ser.Value("rotation", rotation, 'srot');
			ser.Value("bClientActor", bClientActor, 'bool');
			ser.Value("nChannelId", nChannelId, 'schl');
			ser.Value("flags", flags, 'ui32');
		}
		else
		{
			ser.Value("name", name);
			ser.Value("classId", classId);
			ser.Value("pos", pos);
			ser.Value("scale", scale);
			ser.Value("rotation", rotation);
			ser.Value("bClientActor", bClientActor);
			ser.Value("nChannelId", nChannelId);
			ser.Value("flags", flags, 'ui32');
		}
	}
};

class CGameObject : public IGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();

	// IEntityComponent
	virtual void Initialize(IEntity &entity) override;

	virtual void ProcessEvent(const SEntityEvent& event) override;
	virtual void OnEntityReload(SEntitySpawnParams& params, XmlNodeRef entityNode) override;
	
	virtual void Release() override { delete this; }

	virtual void Serialize(TSerialize ser) override;

	virtual bool NeedSerialize() override;

	virtual void GetMemoryUsage(ICrySizer* pSizer) const override;
	// ~IEntityComponent

	// IActionListener
	virtual void OnAction(const ActionId& actionId, int activationMode, float value);
	virtual void AfterAction();
	// ~IActionListener

	// IGameObject
	virtual bool BindToNetwork(EBindToNetworkMode mode = eBTNM_Normal);
	virtual bool                  BindToNetworkWithParent(EBindToNetworkMode mode, EntityId parentId);
	virtual bool IsBoundToNetwork() override { return m_isBoundToNetwork; }

	virtual void                  ChangedNetworkState(NetworkAspectType aspects);
	virtual void                  EnableAspect(NetworkAspectType aspects, bool enable);
	virtual void                  EnableDelegatableAspect(NetworkAspectType aspects, bool enable);
	
	virtual void                  SetChannelId(uint16 id);
	virtual uint16                GetChannelId() const { return m_channelId; }
	virtual INetChannel*          GetNetChannel() const;
	virtual bool                  CaptureView(IGameObjectView* pGOV);
	virtual void                  ReleaseView(IGameObjectView* pGOV);
	virtual bool                  CaptureActions(IActionListener* pAL);
	virtual void                  ReleaseActions(IActionListener* pAL);
	virtual bool                  CaptureProfileManager(IGameObjectProfileManager* pPH);
	virtual void                  ReleaseProfileManager(IGameObjectProfileManager* pPH);
	virtual void                  FullSerialize(TSerialize ser);
	virtual bool                  NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags);
	virtual NetworkAspectType     GetNetSerializeAspects();
	virtual bool                  SetAspectProfile(EEntityAspects aspect, uint8 profile, bool fromNetwork);
	virtual uint8                 GetAspectProfile(EEntityAspects aspect);
	virtual IWorldQuery*          GetWorldQuery();
	virtual IMovementController*  GetMovementController();
	virtual void                  Pulse(uint32 pulse);
	virtual void                  RegisterAsPredicted();
	virtual void                  RegisterAsValidated(IGameObject* pGO, int predictionHandle);
	virtual int                   GetPredictionHandle();
	virtual void                  RequestRemoteUpdate(NetworkAspectType aspectMask);
	
	virtual void SetNetworkParent(EntityId id);

	virtual void DoInvokeRMI(_smart_ptr<CRMIBody> pBody, unsigned, int) const override;

	virtual void AddListener(IGameObjectNetListener *pListener) override { stl::push_back_unique(m_listeners, pListener); }
	virtual void RemoveListener(IGameObjectNetListener *pListener) override { stl::find_and_erase(m_listeners, pListener); }
	// ~IGameObject

	virtual void     UpdateView(SViewParams& viewParams);
	virtual void     PostUpdateView(SViewParams& viewParams);
	virtual bool     CanUpdateView() const { return m_pViewDelegate != NULL; }
	virtual IGameObjectView* GetViewDelegate()     { return m_pViewDelegate; }

#if GAME_OBJECT_SUPPORTS_CUSTOM_USER_DATA
	virtual void* GetUserData() const;
	virtual void  SetUserData(void* ptr);
#endif

	bool IsAspectDelegatable(NetworkAspectType aspect);

	//----------------------------------------------------------------------
	// Network related functions

	// we have gained (or lost) control of this object
	virtual void         SetAuthority(bool auth);
	virtual void         InitClient(int channelId);
	virtual void         PostInitClient(int channelId);

	ISerializableInfoPtr GetSpawnInfo();

	NetworkAspectType    GetEnabledAspects() const { return m_enabledAspects; }
	uint8                GetDefaultProfile(EEntityAspects aspect);

	// called from CGameObject::BoundObject -- we have become bound on a client
	void         BecomeBound()                { m_isBoundToNetwork = true; }
	
	void         PostRemoteSpawn();

	static void  UpdateSchedulingProfiles();

	virtual void DontSyncPhysics() { m_bNoSyncPhysics = true; }

private:
	IActionListener*           m_pActionDelegate;
	IGameObjectView*           m_pViewDelegate;
	IGameObjectProfileManager* m_pProfileManager;

	uint8                      m_profiles[NUM_ASPECTS];

#if GAME_OBJECT_SUPPORTS_CUSTOM_USER_DATA
	void* m_pUserData;
#endif

	uint16            m_channelId;
	NetworkAspectType m_enabledAspects;
	NetworkAspectType m_delegatableAspects;
	bool              m_isBoundToNetwork    : 1;
	bool              m_bNoSyncPhysics      : 1;
	bool              m_bNeedsNetworkRebind : 1;
	
	int                              m_predictionHandle;

	const SEntitySchedulingProfiles* m_pSchedulingProfiles;
	uint32                           m_currentSchedulingProfile;
	EntityId                         m_cachedParentId;

	std::vector<IGameObjectNetListener *> m_listeners;

	bool DoSetAspectProfile(EEntityAspects aspect, uint8 profile, bool fromNetwork);
	
	void UpdateSchedulingProfile();
};

#endif //__GAMEOBJECT_H__
