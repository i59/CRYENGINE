// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#ifndef __IGAMEOBJECT_H__
#define __IGAMEOBJECT_H__

#pragma once

#define GAME_OBJECT_SUPPORTS_CUSTOM_USER_DATA 1

#include <CryEntitySystem/IEntityComponent.h>

#include <CryNetwork/SerializeFwd.h>
#include "IActionMapManager.h"
#include <CryMemory/PoolAllocator.h>
#include <CryFlowGraph/IFlowSystem.h>

inline void GameWarning(const char*, ...) PRINTF_PARAMS(1, 2);

struct IGameObjectView;
struct IActionListener;
struct IMovementController;
struct IGameObjectProfileManager;
struct IWorldQuery;

enum EBindToNetworkMode
{
	eBTNM_Normal,
	eBTNM_Force,
	eBTNM_NowInitialized
};

template<size_t N>
class CRMIAllocator
{
public:
	static ILINE void* Allocate()
	{
		if (!m_pAllocator)
			m_pAllocator = new stl::PoolAllocator<N>;
		return m_pAllocator->Allocate();
	}
	static ILINE void Deallocate(void* p)
	{
		CRY_ASSERT(m_pAllocator);
		m_pAllocator->Deallocate(p);
	}

private:
	static stl::PoolAllocator<N>* m_pAllocator;
};
template<size_t N> stl::PoolAllocator<N>* CRMIAllocator<N>::m_pAllocator = 0;

struct SRemoteComponentFunction
{
	void GetMemoryUsage(ICrySizer* pSizer) const {}
	typedef INetAtSyncItem* (*DecoderFunction)(TSerialize, EntityId*, INetChannel*);

	DecoderFunction       decoder;
	const char*           description;
	const void*           pBase;
	const SNetMessageDef* pMsgDef;
	ERMIAttachmentType    attach;
	bool                  isServerCall;
	bool                  lowDelay;
	ENetReliabilityType   reliability;
};

enum EEntityAspects
{
	eEA_All = NET_ASPECT_ALL,
	// 0x01u                       // aspect 0
	eEA_Script = 0x02u, // aspect 1
	// 0x04u                       // aspect 2
	eEA_Physics = 0x08u, // aspect 3
	eEA_GameClientStatic = 0x10u, // aspect 4
	eEA_GameServerStatic = 0x20u, // aspect 5
	eEA_GameClientDynamic = 0x40u, // aspect 6
	eEA_GameServerDynamic = 0x80u, // aspect 7
	#if NUM_ASPECTS > 8
	eEA_GameClientA = 0x0100u, // aspect 8
	eEA_GameServerA = 0x0200u, // aspect 9
	eEA_GameClientB = 0x0400u, // aspect 10
	eEA_GameServerB = 0x0800u, // aspect 11
	eEA_GameClientC = 0x1000u, // aspect 12
	eEA_GameServerC = 0x2000u, // aspect 13
	eEA_GameClientD = 0x4000u, // aspect 14
	eEA_GameClientE = 0x8000u, // aspect 15
	#endif
	#if NUM_ASPECTS > 16
	eEA_GameClientF = 0x00010000u,       // aspect 16
	eEA_GameClientG = 0x00020000u,       // aspect 17
	eEA_GameClientH = 0x00040000u,       // aspect 18
	eEA_GameClientI = 0x00080000u,       // aspect 19
	eEA_GameClientJ = 0x00100000u,       // aspect 20
	eEA_GameServerD = 0x00200000u,       // aspect 21
	eEA_GameClientK = 0x00400000u,       // aspect 22
	eEA_GameClientL = 0x00800000u,       // aspect 23
	eEA_GameClientM = 0x01000000u,       // aspect 24
	eEA_GameClientN = 0x02000000u,       // aspect 25
	eEA_GameClientO = 0x04000000u,       // aspect 26
	eEA_GameClientP = 0x08000000u,       // aspect 27
	eEA_GameServerE = 0x10000000u,       // aspect 28
	eEA_Aspect29 = 0x20000000u,       // aspect 29
	eEA_Aspect30 = 0x40000000u,       // aspect 30
	eEA_Aspect31 = 0x80000000u,       // aspect 31
	#endif
	};

enum ERMInvocation
{
	eRMI_ToClientChannel = 0x01,
	eRMI_ToOwnClient = 0x02,
	eRMI_ToOtherClients = 0x04,
	eRMI_ToAllClients = 0x08,

	eRMI_ToServer = 0x100,

	eRMI_NoLocalCalls = 0x10000,
	eRMI_NoRemoteCalls = 0x20000,

	eRMI_ToRemoteClients = eRMI_NoLocalCalls | eRMI_ToAllClients
};

struct IGameObjectNetListener
{
	virtual void SetAuthority(bool auth) {}
	virtual void InitClient(int channelId) {}
	virtual void PostInitClient(int channelId) {}

	virtual void SetChannelId(uint16 id) {}

	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) { return true; }
	virtual NetworkAspectType GetNetSerializeAspects() { return 0; }

	virtual ISerializableInfoPtr GetSpawnInfo() { return nullptr; }
	virtual void PostRemoteSpawn() {}
};

// Summary
// Game object component, responsible for network management (remote method invocations)
// Also contains legacy helpers such as CaptureView and CaptureProfileManager
struct IGameObject
	: public IActionListener
	, public IEntityComponent
{
protected:
	class CRMIBody : public IRMIMessageBody
	{
	public:
		CRMIBody(const SRemoteComponentFunction* method, EntityId id, IRMIListener* pListener, int userId, EntityId dependentId) :
			IRMIMessageBody(method->reliability, method->attach, id, method->pMsgDef, pListener, userId, dependentId)
		{
		}
	};

	template<class T>
	class CRMIBodyImpl : public CRMIBody
	{
	public:
		void SerializeWith(TSerialize ser)
		{
			m_params.SerializeWith(ser);
		}

		size_t GetSize()
		{
			return sizeof(*this);
		}

#if ENABLE_RMI_BENCHMARK
		virtual const SRMIBenchmarkParams* GetRMIBenchmarkParams()
		{
			return NetGetRMIBenchmarkParams<T>(m_params);
		}
#endif

		static CRMIBodyImpl* Create(const SRemoteComponentFunction* method, EntityId id, const T& params, IRMIListener* pListener, int userId, EntityId dependentId)
		{
			return new(CRMIAllocator<sizeof(CRMIBodyImpl)>::Allocate())CRMIBodyImpl(method, id, params, pListener, userId, dependentId);
		}

		void DeleteThis()
		{
			this->~CRMIBodyImpl();
			CRMIAllocator<sizeof(CRMIBodyImpl)>::Deallocate(this);
		}

	private:
		T m_params;

		CRMIBodyImpl(const SRemoteComponentFunction* method, EntityId id, const T& params, IRMIListener* pListener, int userId, EntityId dependentId) :
			CRMIBody(method, id, pListener, userId, dependentId),
			m_params(params)
		{
		}
	};

public:
	DECLARE_COMPONENT("GameObject", 0x4731B35A5694458C, 0xBA2CB251E98306DC)

	// bind this entity to the network system (it gets synchronized then...)
	virtual bool                  BindToNetwork(EBindToNetworkMode mode = eBTNM_Normal) = 0;
	// bind this entity to the network system, with a dependency on its parent
	virtual bool                  BindToNetworkWithParent(EBindToNetworkMode mode, EntityId parentId) = 0;

	virtual bool IsBoundToNetwork() = 0;

	// flag that we have changed the state of the game object aspect
	virtual void                  ChangedNetworkState(NetworkAspectType aspects) = 0;
	// enable/disable network aspects on game object
	virtual void                  EnableAspect(NetworkAspectType aspects, bool enable) = 0;
	// enable/disable delegatable aspects
	virtual void                  EnableDelegatableAspect(NetworkAspectType aspects, bool enable) = 0;
	// A one off call to never enable the physics aspect, this needs to be done *before* the BindToNetwork (typically in the GameObject's Init function)
	virtual void                  DontSyncPhysics() = 0;
	
	// get/set network channel
	virtual uint16                GetChannelId() const = 0;
	virtual void SetChannelId(uint16) = 0;
	virtual INetChannel*          GetNetChannel() const = 0;
	// serialize some aspects of the game object
	virtual void                  FullSerialize(TSerialize ser) = 0;
	virtual bool                  NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int pflags) = 0;
	// change the profile of an aspect
	virtual bool                  SetAspectProfile(EEntityAspects aspect, uint8 profile, bool fromNetwork = false) = 0;
	virtual uint8                 GetAspectProfile(EEntityAspects aspect) = 0;
	virtual void                  SetNetworkParent(EntityId id) = 0;
	virtual void                  Pulse(uint32 pulse) = 0;
	virtual void                  RegisterAsPredicted() = 0;
	virtual void                  RegisterAsValidated(IGameObject* pGO, int predictionHandle) = 0;
	virtual int                   GetPredictionHandle() = 0;

	// register a partial update in the netcode without actually serializing - useful only for working around other bugs
	virtual void RequestRemoteUpdate(NetworkAspectType aspectMask) = 0;

	// for extensions to register for special things
	virtual bool                       CaptureView(IGameObjectView* pGOV) = 0;
	virtual void                       ReleaseView(IGameObjectView* pGOV) = 0;
	virtual IGameObjectView* GetViewDelegate() = 0;
	virtual bool                       CaptureActions(IActionListener* pAL) = 0;
	virtual void                       ReleaseActions(IActionListener* pAL) = 0;
	virtual bool                       CaptureProfileManager(IGameObjectProfileManager* pPH) = 0;
	virtual void                       ReleaseProfileManager(IGameObjectProfileManager* pPH) = 0;
	virtual IWorldQuery*               GetWorldQuery() = 0;

	ILINE void                         SetMovementController(IMovementController* pMC) { m_pMovementController = pMC; }
	virtual ILINE IMovementController* GetMovementController()                         { return m_pMovementController; }

	virtual void                       GetMemoryUsage(ICrySizer* pSizer) const         {};

#if GAME_OBJECT_SUPPORTS_CUSTOM_USER_DATA
	virtual void* GetUserData() const = 0;
	virtual void  SetUserData(void* ptr) = 0;
#endif

	template<class MI, class T>
	void InvokeRMI_Primitive(const MI method, const T& params, unsigned where, IRMIListener* pListener, int userId, int channel, EntityId dependentId) const
	{
		method.Verify(params);

		DoInvokeRMI(IGameObject::CRMIBodyImpl<T>::Create(method.pMethodInfo, GetEntityId(), params, pListener, userId, dependentId), where, channel);
	}

	// invoke an RMI call
	virtual void                  DoInvokeRMI(_smart_ptr<CRMIBody> pBody, unsigned where, int channel) const = 0;

	virtual void AddListener(IGameObjectNetListener *pListener) = 0;
	virtual void RemoveListener(IGameObjectNetListener *pListener) = 0;

protected:
	IGameObject() : m_pMovementController(0) {}
	IMovementController* m_pMovementController;
};

struct IRMIAtSyncItem : public INetAtSyncItem, public IRMICppLogger {};

template<class T, class Obj>
class CRMIAtSyncItem : public IRMIAtSyncItem
{
public:
	typedef bool (Obj::* CallbackFunc)(const T&, INetChannel*);

	// INetAtSyncItem
	// INetAtSyncItem

	static ILINE CRMIAtSyncItem* Create(const T& params, EntityId id, const SRemoteComponentFunction* pRMI, CallbackFunc callback, INetChannel* pChannel)
	{
		return new(CRMIAllocator<sizeof(CRMIAtSyncItem)>::Allocate())CRMIAtSyncItem(params, id, pRMI, callback, pChannel);
	}

	bool Sync()
	{
		bool ok = false;
		bool foundObject = false;
		char msg[256];
		msg[0] = 0;

		if(auto *pEntity = gEnv->pEntitySystem->GetEntity(m_id))
		{
			INDENT_LOG_DURING_SCOPE(true, "During game object sync: %s %s", pEntity->GetEntityTextDescription().c_str(), m_pRMI->pMsgDef->description);

			if (auto *pComponent = static_cast<Obj *>(pEntity->GetComponentWithRMIBase(m_pRMI->pBase)))
			{
				ok = (pComponent->*m_callback)(m_params, m_pChannel);
				foundObject = true;
			}
			else
			{
				cry_sprintf(msg, "Entity %u component for RMI %s not found", m_id, m_pRMI->pMsgDef->description);
			}
		}
		else
		{
			cry_sprintf(msg, "Entity %u for RMI %s not found", m_id, m_pRMI->pMsgDef->description);
		}

		if (!ok)
		{
			GameWarning("Error handling RMI %s", m_pRMI->pMsgDef->description);

			if (!foundObject && !gEnv->bServer && !m_pChannel->IsInTransition())
			{
				CRY_ASSERT(msg[0]);
				m_pChannel->Disconnect(eDC_ContextCorruption, msg);
			}
			else
			{
				ok = true;
				// fake for singleplayer/multiplayer server
				// singleplayer - 'impossible' to get right during quick-load
				// multiplayer server - object can be deleted while the message is in flight
			}
		}

		if (!foundObject)
			return true; // for editor
		else
			return ok;
	}

	bool SyncWithError(EDisconnectionCause& disconnectCause, string& disconnectMessage)
	{
		return Sync();
	}

	void DeleteThis()
	{
		this->~CRMIAtSyncItem();
		CRMIAllocator<sizeof(CRMIAtSyncItem)>::Deallocate(this);
	}
	// ~INetAtSyncItem

	// IRMICppLogger
	virtual const char* GetName()
	{
		return m_pRMI->description;
	}
	virtual void SerializeParams(TSerialize ser)
	{
		m_params.SerializeWith(ser);
	}
	// ~IRMICppLogger

private:
	CRMIAtSyncItem(const T& params, EntityId id, const SRemoteComponentFunction* pRMI, CallbackFunc callback, INetChannel* pChannel) : m_params(params), m_id(id), m_pRMI(pRMI), m_callback(callback), m_pChannel(pChannel) {}

	T                              m_params;
	EntityId                       m_id;
	const SRemoteComponentFunction* m_pRMI;
	CallbackFunc                   m_callback;
	INetChannel*                   m_pChannel;
};

struct SViewParams;

struct IGameObjectView
{
	virtual ~IGameObjectView(){}
	virtual void UpdateView(SViewParams& params) = 0;
	virtual void PostUpdateView(SViewParams& params) = 0;
};

struct IGameObjectProfileManager
{
	virtual ~IGameObjectProfileManager(){}
	virtual bool  SetAspectProfile(EEntityAspects aspect, uint8 profile) = 0;
	virtual uint8 GetDefaultProfile(EEntityAspects aspect) = 0;
};
#endif
