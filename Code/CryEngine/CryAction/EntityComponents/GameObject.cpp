// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$

   -------------------------------------------------------------------------
   History:
   - 6:9:2004   12:46 : Created by Márcio Martins

*************************************************************************/
#include "StdAfx.h"
#include "GameObject.h"
#include "CryAction.h"
#include "Network/GameContext.h"
#include "Network/GameClientChannel.h"
#include "Network/GameServerChannel.h"
#include "Network/GameClientNub.h"
#include "Network/GameServerNub.h"
#include "Serialization/SerializeScriptTableWriter.h"
#include "Serialization/SerializeScriptTableReader.h"
#include <CrySystem/ITextModeConsole.h>
#include "CryActionCVars.h"

#include <CryAISystem/IAIObject.h>
#include <CryAISystem/IAIActorProxy.h>

#include "IWorldQuery.h"

// ugly: for GetMovementController()
#include "IActorSystem.h"
#include "IVehicleSystem.h"

#include <CryNetwork/INetwork.h>

static std::set<CGameObject*> g_updateSchedulingProfile;

//------------------------------------------------------------------------
CGameObject::CGameObject() :
	m_pActionDelegate(0),
	m_pViewDelegate(0),
	m_pProfileManager(0),
#if GAME_OBJECT_SUPPORTS_CUSTOM_USER_DATA
	m_pUserData(0),
#endif
	m_channelId(0),
	m_enabledAspects(NET_ASPECT_ALL),
	m_delegatableAspects(NET_ASPECT_ALL),
	m_isBoundToNetwork(false),
	m_pSchedulingProfiles(NULL),
	m_predictionHandle(0),
	m_bNoSyncPhysics(false),
	m_bNeedsNetworkRebind(false),
	m_cachedParentId(0)
{
	for (int i = 0; i < NUM_ASPECTS; i++)
		m_profiles[i] = 255;
}

//------------------------------------------------------------------------
CGameObject::~CGameObject()
{
	g_updateSchedulingProfile.erase(this);
}

//------------------------------------------------------------------------
void CGameObject::Initialize(IEntity &entity)
{
	IEntityComponent::Initialize(entity);

	m_pSchedulingProfiles = gEnv->pEntitySystem->GetEntitySchedulerProfiles(m_pEntity);

	EnableEvent(ENTITY_EVENT_INIT, 0, true);
	EnableEvent(ENTITY_EVENT_DONE, 0, true);
}

//------------------------------------------------------------------------
void CGameObject::OnEntityReload(SEntitySpawnParams& spawnParams, XmlNodeRef entityNode)
{
	m_entityId = m_pEntity->GetId();
	m_pMovementController = NULL;

	m_pActionDelegate = 0;
	m_pViewDelegate = 0;
	m_pProfileManager = 0;
	m_channelId = 0;
	m_enabledAspects = NET_ASPECT_ALL;
	m_delegatableAspects = NET_ASPECT_ALL;
	m_isBoundToNetwork = false;
	m_predictionHandle = 0;
	m_bNoSyncPhysics = false;
	m_bNeedsNetworkRebind = false;

	m_pSchedulingProfiles = gEnv->pEntitySystem->GetEntitySchedulerProfiles(m_pEntity);
}

bool CGameObject::BindToNetwork(EBindToNetworkMode mode)
{
	return BindToNetworkWithParent(mode, 0);
}

bool CGameObject::BindToNetworkWithParent(EBindToNetworkMode mode, EntityId parentId)
{
	CGameContext* pGameContext = CCryAction::GetCryAction()->GetGameContext();
	if (!pGameContext)
		return false;
	INetContext* pNetContext = pGameContext->GetNetContext();
	if (!pNetContext)
		return false;

	if (m_isBoundToNetwork)
	{
		switch (mode)
		{
		case eBTNM_NowInitialized:
			if (!m_isBoundToNetwork)
				return false;
			CRY_ASSERT(parentId == 0);
			parentId = m_cachedParentId;
		// fall through
		case eBTNM_Force:
			m_isBoundToNetwork = false;
			break;
		case eBTNM_Normal:
			return true;
		}
	}
	else if (mode == eBTNM_NowInitialized)
		return false;

	if (GetEntity()->GetFlags() & (ENTITY_FLAG_CLIENT_ONLY | ENTITY_FLAG_SERVER_ONLY))
		return false;

	if (!GetEntity()->IsInitialized())
	{
		m_cachedParentId = parentId;
		m_isBoundToNetwork = true;
		return true;
	}

	static const NetworkAspectType gameObjectAspects =
	  eEA_GameClientDynamic |
	  eEA_GameServerDynamic |
	  eEA_GameClientStatic |
	  eEA_GameServerStatic |
	  eEA_Aspect31 |
	  eEA_GameClientA |
	  eEA_GameServerA |
	  eEA_GameClientB |
	  eEA_GameServerB |
	  eEA_GameClientC |
	  eEA_GameServerC |
	  eEA_GameClientD |
	  eEA_GameClientE |
	  eEA_GameClientF |
	  eEA_GameClientG |
	  eEA_GameClientH |
	  eEA_GameClientI |
	  eEA_GameClientJ |
	  eEA_GameClientK |
	  eEA_GameServerD |
	  eEA_GameClientL |
	  eEA_GameClientM |
	  eEA_GameClientN |
	  eEA_GameClientO |
	  eEA_GameClientP |
	  eEA_GameServerE;

	NetworkAspectType aspects = gameObjectAspects;
	INetChannel* pControllingChannel = NULL;

	if (!m_bNoSyncPhysics && (GetEntity()->QueryComponent<IEntityPhysicsComponent>() || m_pProfileManager))
	{
		aspects |= eEA_Physics;
		//		aspects &= ~eEA_Volatile;
	}
	if(auto *pScriptComponent = GetEntity()->QueryComponent<IEntityScriptComponent>())
		aspects |= eEA_Script;

	if (gEnv->bServer)
	{
		if (m_channelId)
		{
			CGameServerNub* pServerNub = CCryAction::GetCryAction()->GetGameServerNub();
			if (!pServerNub)
			{
				GameWarning("Unable to bind object to network (%s)", GetEntity()->GetName());
				return false;
			}
			CGameServerChannel* pServerChannel = pServerNub->GetChannel(m_channelId);
			if (!pServerChannel)
			{
				GameWarning("Unable to bind object to network (%s)", GetEntity()->GetName());
				return false;
			}
			pServerChannel->SetPlayerId(GetEntity()->GetId());
			pControllingChannel = pServerChannel->GetNetChannel();
		}

		bool isStatic = CCryAction::GetCryAction()->IsInLevelLoad();
		if (GetEntity()->GetFlags() & ENTITY_FLAG_NEVER_NETWORK_STATIC)
			isStatic = false;

		aspects &= GetNetSerializeAspects();

		pNetContext->BindObject(GetEntityId(), parentId, aspects, isStatic);
		pNetContext->SetDelegatableMask(GetEntityId(), m_delegatableAspects);
		if (pControllingChannel)
		{
			//pNetContext->DelegateAuthority( GetEntityId(), pControllingChannel );
		}
	}

	// will this work :)
	if (pControllingChannel)
	{
		pControllingChannel->DeclareWitness(GetEntityId());
	}

	m_isBoundToNetwork = true;
	g_updateSchedulingProfile.insert(this);

	return true;
}

bool CGameObject::IsAspectDelegatable(NetworkAspectType aspect)
{
	return (m_delegatableAspects & aspect) ? true : false;
}

void CGameObject::UpdateSchedulingProfile()
{
	bool remove = false;
	if (m_isBoundToNetwork && m_pSchedulingProfiles)
	{
		// We need to check NetContext here, because it's NULL in a dummy editor game session (or at least while starting up the editor).
		INetContext* pNetContext = CCryAction::GetCryAction()->GetGameContext()->GetNetContext();
		if (pNetContext && pNetContext->SetSchedulingParams(GetEntityId(), m_pSchedulingProfiles->normal, m_pSchedulingProfiles->owned))
			remove = true;
	}
	else
	{
		remove = true;
	}
	if (remove)
	{
		g_updateSchedulingProfile.erase(this);
	}
}

void CGameObject::UpdateSchedulingProfiles()
{
	for (std::set<CGameObject*>::iterator it = g_updateSchedulingProfile.begin(); it != g_updateSchedulingProfile.end(); )
	{
		std::set<CGameObject*>::iterator next = it;
		++next;
		(*it)->UpdateSchedulingProfile();
		it = next;
	}
}

//------------------------------------------------------------------------
void CGameObject::ProcessEvent(const SEntityEvent& event)
{
	FUNCTION_PROFILER(GetISystem(), PROFILE_ACTION);

	if (m_pEntity)
	{
		switch (event.event)
		{
		case ENTITY_EVENT_INIT:
			if (m_bNeedsNetworkRebind)
			{
				m_bNeedsNetworkRebind = false;
				BindToNetwork(eBTNM_Force);
			}
			break;

		case ENTITY_EVENT_DONE:
			if (m_isBoundToNetwork)
			{
				// check if we're still bound
				CGameContext* pGameContext = CCryAction::GetCryAction()->GetGameContext();
				if (pGameContext)
				{
					INetContext* pNetContext = pGameContext->GetNetContext();
					if (pNetContext)
					{
						m_isBoundToNetwork = pNetContext->IsBound(GetEntityId());
						m_bNeedsNetworkRebind = !m_isBoundToNetwork;
					}
				}
			}
			break;
		}
	}
}

//------------------------------------------------------------------------
bool CGameObject::CaptureActions(IActionListener* pAL)
{
	if (m_pActionDelegate || !pAL)
		return false;
	m_pActionDelegate = pAL;
	return true;
}

bool CGameObject::CaptureView(IGameObjectView* pGOV)
{
	if (m_pViewDelegate || !pGOV)
		return false;
	m_pViewDelegate = pGOV;
	return true;
}

bool CGameObject::CaptureProfileManager(IGameObjectProfileManager* pPM)
{
	if (m_pProfileManager || !pPM)
		return false;
	m_pProfileManager = pPM;
	return true;
}

void CGameObject::ReleaseActions(IActionListener* pAL)
{
	if (m_pActionDelegate != pAL)
		return;
	m_pActionDelegate = 0;
}

void CGameObject::ReleaseView(IGameObjectView* pGOV)
{
	if (m_pViewDelegate != pGOV)
		return;
	m_pViewDelegate = 0;
}

void CGameObject::ReleaseProfileManager(IGameObjectProfileManager* pPM)
{
	if (m_pProfileManager != pPM)
		return;
	m_pProfileManager = 0;
}

//------------------------------------------------------------------------
void CGameObject::UpdateView(SViewParams& viewParams)
{
	if (m_pViewDelegate)
		m_pViewDelegate->UpdateView(viewParams);
}

//------------------------------------------------------------------------
void CGameObject::PostUpdateView(SViewParams& viewParams)
{
	if (m_pViewDelegate)
		m_pViewDelegate->PostUpdateView(viewParams);
}

//------------------------------------------------------------------------
void CGameObject::OnAction(const ActionId& actionId, int activationMode, float value)
{
	if (m_pActionDelegate)
		m_pActionDelegate->OnAction(actionId, activationMode, value);
	//	else
	//		GameWarning("Action sent to an entity that doesn't implement action-listener; entity class is %s, and action is %s",GetEntity()->GetClass()->GetName(), actionId.c_str());
}

void CGameObject::AfterAction()
{
	if (m_pActionDelegate)
		m_pActionDelegate->AfterAction();
}

//////////////////////////////////////////////////////////////////////////
bool CGameObject::NeedSerialize()
{
	return true;
}

static const char* AspectProfileSerializationName(int i)
{
	static char buffer[11] = { 0 };

	if (!buffer[0])
	{
		buffer[0] = 'a';
		buffer[1] = 'p';
		buffer[2] = 'r';
		buffer[3] = 'o';
		buffer[4] = 'f';
		buffer[5] = 'i';
		buffer[6] = 'l';
		buffer[7] = 'e';
	}

	assert(i >= 0 && i < 256);
	i = clamp_tpl<int>(i, 0, 255);
	buffer[8] = 0;
	buffer[9] = 0;
	buffer[10] = 0;
	itoa(i, &(buffer[8]), 10);

	return buffer;
}

//------------------------------------------------------------------------
void CGameObject::Serialize(TSerialize ser)
{
	FullSerialize(ser);
}

struct SExtensionSerInfo
{
	uint32      extensionLocalIndex;
	uint32      priority;
	static bool SortingComparison(const SExtensionSerInfo& s0, const SExtensionSerInfo& s1)
	{
		return s0.priority < s1.priority;
	}
};

void CGameObject::FullSerialize(TSerialize ser)
{
	CRY_ASSERT(ser.GetSerializationTarget() != eST_Network);

	if (ser.BeginOptionalGroup("GameObject", true))
	{
		const uint8 profileDefault = 255;

		if (ser.IsWriting())
		{
			uint8 profile = profileDefault;
			if (m_pProfileManager)
			{
				for (NetworkAspectID i = 0; i < NUM_ASPECTS; i++)
				{
					NetworkAspectType aspect = 1 << i;
					profile = GetAspectProfile((EEntityAspects)aspect);

					ser.ValueWithDefault(AspectProfileSerializationName(i), profile, profileDefault);
				}
			}
		}
		else //reading
		{
			//physicalize after serialization with the correct parameters
			if (m_pProfileManager)
			{
				for (NetworkAspectID i = 0; i < NUM_ASPECTS; i++)
				{
					NetworkAspectType aspect = 1 << i;

					uint8 profile = 0;
					ser.ValueWithDefault(AspectProfileSerializationName(i), profile, profileDefault);
					SetAspectProfile((EEntityAspects)aspect, profile, false);
				}
			}

		}

		ser.EndGroup(); //GameObject
	}
}

//------------------------------------------------------------------------
bool CGameObject::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		(*it)->NetSerialize(ser, aspect, profile, flags);
	}

	if (aspect == eEA_Physics && !m_pProfileManager)
	{
		if (IEntityPhysicsComponent* pProxy = (IEntityPhysicsComponent*) GetEntity()->QueryComponent<IEntityPhysicsComponent>())
			pProxy->Serialize(ser);
		else
			return false;
	}

	return true;
}

NetworkAspectType CGameObject::GetNetSerializeAspects()
{
	NetworkAspectType aspects = 0;

	for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		aspects |= (*it)->GetNetSerializeAspects();
	}

	if (!m_bNoSyncPhysics && (GetEntity()->QueryComponent<IEntityPhysicsComponent>() || m_pProfileManager))
	{
		aspects |= eEA_Physics;
	}

	if (auto *pScriptComponent = GetEntity()->QueryComponent<IEntityScriptComponent>())
	{
		aspects |= eEA_Script;
	}

	return aspects;
}

//------------------------------------------------------------------------
void CGameObject::SetAuthority(bool auth)
{
	for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		(*it)->SetAuthority(auth);
	}
}

//------------------------------------------------------------------------
void CGameObject::InitClient(int channelId)
{
	for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		(*it)->InitClient(channelId);
	}
}

//------------------------------------------------------------------------
void CGameObject::PostInitClient(int channelId)
{
	for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		(*it)->PostInitClient(channelId);
	}
}

//------------------------------------------------------------------------
void CGameObject::ChangedNetworkState(NetworkAspectType aspects)
{
	CGameContext* pGameContext = CCryAction::GetCryAction()->GetGameContext();
	if (!pGameContext)
		return;
	INetContext* pNetContext = pGameContext->GetNetContext();
	if (!pNetContext)
		return;
	IEntity* pEntity = GetEntity();
	if (!pEntity)
		return;
	pNetContext->ChangedAspects(pEntity->GetId(), aspects);
}

//------------------------------------------------------------------------
void CGameObject::EnableAspect(NetworkAspectType aspects, bool enable)
{
	CGameContext* pGameContext = CCryAction::GetCryAction()->GetGameContext();
	if (!pGameContext)
		return;

	if (enable)
		m_enabledAspects |= aspects;
	else
		m_enabledAspects &= ~aspects;

	pGameContext->EnableAspects(GetEntityId(), aspects, enable);
}

//------------------------------------------------------------------------
void CGameObject::EnableDelegatableAspect(NetworkAspectType aspects, bool enable)
{
	if (enable)
	{
		m_delegatableAspects |= aspects;
	}
	else
	{
		m_delegatableAspects &= ~aspects;
	}
}

void CGameObject::SetChannelId(uint16 id)
{
	m_channelId = id;

	for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		(*it)->SetChannelId(id);
	}
}

INetChannel* CGameObject::GetNetChannel() const
{
	CGameServerNub* pServerNub = CCryAction::GetCryAction()->GetGameServerNub();
	if (pServerNub)
	{
		CGameServerChannel* pChannel = pServerNub->GetChannel(m_channelId);
		if (pChannel)
			return pChannel->GetNetChannel();
	}

	return 0;
}

void CGameObject::DoInvokeRMI(_smart_ptr<CRMIBody> pBody, unsigned where, int channel) const
{
	// 'where' flag validation
	if (where & eRMI_ToClientChannel)
	{
		if (channel <= 0)
		{
			GameWarning("InvokeRMI: ToClientChannel specified, but no channel specified");
			return;
		}
		if (where & eRMI_ToOwnClient)
		{
			GameWarning("InvokeRMI: ToOwnClient and ToClientChannel specified - not supported");
			return;
		}
	}
	if (where & eRMI_ToOwnClient)
	{
		if (m_channelId == 0)
		{
			GameWarning("InvokeRMI: ToOwnClient specified, but no own client");
			return;
		}
		where &= ~eRMI_ToOwnClient;
		where |= eRMI_ToClientChannel;
		channel = m_channelId;
	}
	if (where & eRMI_ToAllClients)
	{
		where &= ~eRMI_ToAllClients;
		where |= eRMI_ToOtherClients;
		channel = -1;
	}

	CCryAction* pFramework = CCryAction::GetCryAction();

	if (where & eRMI_ToServer)
	{
		CGameClientNub* pClientNub = pFramework->GetGameClientNub();
		bool called = false;
		if (pClientNub)
		{
			CGameClientChannel* pChannel = pClientNub->GetGameClientChannel();
			if (pChannel)
			{
				INetChannel* pNetChannel = pChannel->GetNetChannel();
				bool isLocal = pNetChannel->IsLocal();
				bool send = true;
				if ((where & eRMI_NoLocalCalls) != 0)
					if (isLocal)
						send = false;
				if ((where & eRMI_NoRemoteCalls) != 0)
					if (!isLocal)
						send = false;
				if (send)
				{
					NET_PROFILE_SCOPE_RMI(GetEntity()->GetClass()->GetName(), false);
					NET_PROFILE_SCOPE_RMI(GetEntity()->GetName(), false);
					pNetChannel->DispatchRMI(&*pBody);
					called = true;
				}
			}
		}
		if (!called)
		{
			GameWarning("InvokeRMI: RMI via client (to server) requested but we are not a client");
		}
	}
	if (where & (eRMI_ToClientChannel | eRMI_ToOtherClients))
	{
		CGameServerNub* pServerNub = pFramework->GetGameServerNub();
		if (pServerNub)
		{
			TServerChannelMap* pChannelMap = pServerNub->GetServerChannelMap();
			for (TServerChannelMap::iterator iter = pChannelMap->begin(); iter != pChannelMap->end(); ++iter)
			{
				bool isOwn = iter->first == channel;
				if (isOwn && !(where & eRMI_ToClientChannel) && !IsDemoPlayback())
					continue;
				if (!isOwn && !(where & eRMI_ToOtherClients))
					continue;
				INetChannel* pNetChannel = iter->second->GetNetChannel();
				if (!pNetChannel)
					continue;
				bool isLocal = pNetChannel->IsLocal();
				if (isLocal && (where & eRMI_NoLocalCalls) != 0)
					continue;
				if (!isLocal && (where & eRMI_NoRemoteCalls) != 0)
					continue;
				NET_PROFILE_SCOPE_RMI(GetEntity()->GetClass()->GetName(), false);
				NET_PROFILE_SCOPE_RMI(GetEntity()->GetName(), false);
				pNetChannel->DispatchRMI(&*pBody);
			}
		}
		else if (pFramework->GetGameContext() &&
		         gEnv->bMultiplayer)
		{
			GameWarning("InvokeRMI: RMI via server (to client) requested but we are not a server");
		}
	}
}

IWorldQuery* CGameObject::GetWorldQuery()
{
	return GetEntity()->QueryComponent<IWorldQuery>();
}

IMovementController* CGameObject::GetMovementController()
{
	IActor* pActor = CCryAction::GetCryAction()->GetIActorSystem()->GetActor(m_pEntity->GetId());
	if (pActor != NULL)
		return pActor->GetMovementController();
	else if (IVehicle* pVehicle = CCryAction::GetCryAction()->GetIVehicleSystem()->GetVehicle(m_pEntity->GetId()))
		return pVehicle->GetMovementController();
	else
		return NULL;
}

uint8 CGameObject::GetAspectProfile(EEntityAspects aspect)
{
	uint8 profile = 255;
	if (m_isBoundToNetwork)
	{
		if (CGameContext* pGC = CCryAction::GetCryAction()->GetCryAction()->GetGameContext())
		{
			if (INetContext* pNC = pGC->GetNetContext())
				profile = pNC->GetAspectProfile(GetEntityId(), aspect);
		}
	}
	else
		profile = m_profiles[BitIndex(NetworkAspectType(aspect))];

	return profile;
}

bool CGameObject::SetAspectProfile(EEntityAspects aspect, uint8 profile, bool fromNetwork)
{
	bool ok = DoSetAspectProfile(aspect, profile, fromNetwork);

	if (ok && aspect == eEA_Physics && m_isBoundToNetwork && gEnv->bMultiplayer)
	{
		if (IPhysicalEntity* pEnt = GetEntity()->GetPhysics())
		{
			pe_params_flags flags;
			flags.flagsOR = pef_log_collisions;
			pEnt->SetParams(&flags);
		}
	}

	return ok;
}

bool CGameObject::DoSetAspectProfile(EEntityAspects aspect, uint8 profile, bool fromNetwork)
{
	if (m_isBoundToNetwork)
	{
		if (fromNetwork)
		{
			if (m_pProfileManager)
			{
				if (m_pProfileManager->SetAspectProfile(aspect, profile))
				{
					m_profiles[BitIndex(NetworkAspectType(aspect))] = profile;
					return true;
				}
			}
			else
				return false;
		}
		else
		{
			if (CGameContext* pGameContext = CCryAction::GetCryAction()->GetGameContext())
			{
				if (INetContext* pNetContext = pGameContext->GetNetContext())
				{
					pNetContext->SetAspectProfile(GetEntityId(), aspect, profile);
					m_profiles[BitIndex(NetworkAspectType(aspect))] = profile;
					return true;
				}
			}
			return false;
		}
	}
	else if (m_pProfileManager)
	{
		//CRY_ASSERT( !fromNetwork );
		if (m_pProfileManager->SetAspectProfile(aspect, profile))
		{
			m_profiles[BitIndex(NetworkAspectType(aspect))] = profile;
			return true;
		}
	}
	return false;
}

struct SContainerSer : public ISerializableInfo
{
	void SerializeWith(TSerialize ser)
	{
		for (size_t i = 0; i < m_children.size(); i++)
			m_children[i]->SerializeWith(ser);
	}

	std::vector<ISerializableInfoPtr> m_children;
};

ISerializableInfoPtr CGameObject::GetSpawnInfo()
{
	_smart_ptr<SContainerSer> pC;

	for(auto it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		ISerializableInfoPtr pS = (*it)->GetSpawnInfo();
		if (pS)
		{
			if (!pC)
				pC = new SContainerSer;
			pC->m_children.push_back(pS);
		}
	}

	return &*pC;
}

void CGameObject::SetNetworkParent(EntityId id)
{
	if (!GetEntity()->IsInitialized())
	{
		m_cachedParentId = id;
		return;
	}

	CCryAction* pCryAction = CCryAction::GetCryAction();
	INetContext* pNetContext = pCryAction ? pCryAction->GetNetContext() : NULL;

	if (pNetContext)
	{
		pNetContext->SetParentObject(GetEntityId(), id);
	}
}

uint8 CGameObject::GetDefaultProfile(EEntityAspects aspect)
{
	if (m_pProfileManager)
		return m_pProfileManager->GetDefaultProfile(aspect);
	else
		return 0;
}

void CGameObject::Pulse(uint32 pulse)
{
	if (CGameContext* pGC = CCryAction::GetCryAction()->GetGameContext())
		pGC->GetNetContext()->PulseObject(GetEntityId(), pulse);
}

void CGameObject::PostRemoteSpawn()
{
	for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		(*it)->PostRemoteSpawn();
	}
}

void CGameObject::GetMemoryUsage(ICrySizer* s) const
{
	SIZER_SUBCOMPONENT_NAME(s, "GameObject");
	s->AddObject(this, sizeof(*this));
}

void CGameObject::RegisterAsPredicted()
{
	CRY_ASSERT(!m_predictionHandle);
	m_predictionHandle = CCryAction::GetCryAction()->GetNetContext()->RegisterPredictedSpawn(CCryAction::GetCryAction()->GetClientChannel(), GetEntityId());
}

int CGameObject::GetPredictionHandle()
{
	return m_predictionHandle;
}

void CGameObject::RegisterAsValidated(IGameObject* pGO, int predictionHandle)
{
	if (!pGO)
		return;
	m_predictionHandle = predictionHandle;
	CCryAction::GetCryAction()->GetNetContext()->RegisterValidatedPredictedSpawn(pGO->GetNetChannel(), m_predictionHandle, GetEntityId());
}

void CGameObject::RequestRemoteUpdate(NetworkAspectType aspectMask)
{
	if (INetContext* pNC = CCryAction::GetCryAction()->GetNetContext())
		pNC->RequestRemoteUpdate(GetEntityId(), aspectMask);
}

#if GAME_OBJECT_SUPPORTS_CUSTOM_USER_DATA
void* CGameObject::GetUserData() const
{
	return m_pUserData;
}

void CGameObject::SetUserData(void* ptr)
{
	m_pUserData = ptr;
}
#endif