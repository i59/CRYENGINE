// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.
#pragma once

#include <CryAction/IGameObject.h>

template<class T_Parent, size_t MAX_RMI_MESSAGES = 64>
class CNetworkedEntityComponent 
	: public T_Parent
	, public IGameObjectNetListener
{
public:
	// IEntityComponent
	virtual void Initialize(IEntity &entity)
	{
		T_Parent::Initialize(entity);

		auto &gameObject = entity.AcquireExternalComponent<IGameObject>();
		gameObject.BindToNetwork();

		gameObject.AddListener(this);
	}
	// ~IEntityComponent

	virtual ~CNetworkedEntityComponent()
	{
		if (auto *pGameObject = GetEntity()->QueryComponent<IGameObject>())
		{
			pGameObject->RemoveListener(this);
		}
	}

	static void GetGameObjectExtensionRMIData(void** ppRMI, size_t* nCount)
	{
		*ppRMI = ms_statics.m_vMessages;
		*nCount = ms_statics.m_nMessages;
	}

	const void* GetRMIBase() const
	{
		return ms_statics.m_vMessages;
	}

	// WARNING: there *MUST* be at least one frame between spawning ent and using this function to send an RMI if
	// that RMI is _FAST, otherwise the dependent entity is ignored
	template<class MI, class T>
	void InvokeRemoteMethodWithDependentObject(const MI method, const T& params, unsigned where, EntityId ent, int channel = -1) const
	{
		if(auto *pGameObject = GetEntity()->QueryComponent<IGameObject>())
			pGameObject->InvokeRemoteMethod_Primitive(method, params, where, 0, 0, channel, ent);
	}

	template<class MI, class T>
	void InvokeRemoteMethod(const MI method, const T& params, unsigned where, int channel = -1) const
	{
		if (auto *pGameObject = GetEntity()->QueryComponent<IGameObject>())
			pGameObject->InvokeRemoteMethod_Primitive(method, params, where, 0, 0, channel, 0);
	}

protected:
	static const SRemoteComponentFunction* Helper_AddMessage(SRemoteComponentFunction::DecoderFunction decoder, const char* description, ERMIAttachmentType attach, bool isServerCall, ENetReliabilityType reliability, bool lowDelay)
	{
		if (ms_statics.m_nMessages >= MAX_RMI_MESSAGES)
		{
			// Assert or CryFatalError here uses gEnv, which is not yet initialized.
			__debugbreak();
			((void(*)())NULL)();
			return NULL;
		}
		SRemoteComponentFunction& rmi = ms_statics.m_vMessages[ms_statics.m_nMessages++];
		rmi.decoder = decoder;
		rmi.description = description;
		rmi.attach = attach;
		rmi.isServerCall = isServerCall;
		rmi.pBase = ms_statics.m_vMessages;
		rmi.reliability = reliability;
		rmi.pMsgDef = 0;
		rmi.lowDelay = lowDelay;
		return &rmi;
	}

private:
	struct Statics
	{
		size_t                         m_nMessages;
		SRemoteComponentFunction        m_vMessages[MAX_RMI_MESSAGES];
	};

	static Statics ms_statics;
};

template<class U, size_t N>
typename CNetworkedEntityComponent<U, N>::Statics CNetworkedEntityComponent<U, N>::ms_statics;

#define DECLARE_RMI(name, params, reliability, attachment, isServer, lowDelay)                              \
  public:                                                                                                   \
    struct MethodInfo_ ## name                                                                              \
    {                                                                                                       \
      MethodInfo_ ## name(const SRemoteComponentFunction * pMethodInfo) { this->pMethodInfo = pMethodInfo; } \
      const SRemoteComponentFunction* pMethodInfo;                                                           \
      ILINE void Verify(const params &p) const                                                              \
      {                                                                                                     \
      }                                                                                                     \
    };                                                                                                      \
  private:                                                                                                  \
    static INetAtSyncItem* Decode_ ## name(TSerialize, EntityId*, INetChannel*);                            \
    bool Handle_ ## name(const params &, INetChannel*);                                                     \
    static const ERMIAttachmentType Attach_ ## name = attachment;                                           \
    static const bool ServerCall_ ## name = isServer;                                                       \
    static const ENetReliabilityType Reliability_ ## name = reliability;                                    \
    static const bool LowDelay_ ## name = lowDelay;                                                         \
    typedef params Params_ ## name;                                                                         \
    static MethodInfo_ ## name m_info ## name;                                                              \
  public:                                                                                                   \
    static const MethodInfo_ ## name& name() { return m_info ## name; }

#define DECLARE_INTERFACE_RMI(name, params, reliability, attachment, isServer, lowDelay)                    \
  protected:                                                                                                \
    static const ERMIAttachmentType Attach_ ## name = attachment;                                           \
    static const bool ServerCall_ ## name = isServer;                                                       \
    static const ENetReliabilityType Reliability_ ## name = reliability;                                    \
    static const bool LowDelay_ ## name = lowDelay;                                                         \
    typedef params Params_ ## name;                                                                         \
  public:                                                                                                   \
    struct MethodInfo_ ## name                                                                              \
    {                                                                                                       \
      MethodInfo_ ## name(const SRemoteComponentFunction * pMethodInfo) { this->pMethodInfo = pMethodInfo; } \
      const SRemoteComponentFunction* pMethodInfo;                                                           \
      ILINE void Verify(const params &p) const                                                              \
      {                                                                                                     \
      }                                                                                                     \
    };                                                                                                      \
    virtual const MethodInfo_ ## name& name() = 0

#define DECLARE_IMPLEMENTATION_RMI(name)                                  \
  private:                                                                \
    INetAtSyncItem * Decode_ ## name(TSerialize, EntityId, INetChannel*); \
    static bool Handle_ ## name(const Params_ ## name &, INetChannel*);   \
    static MethodInfo_ ## name m_info ## name;                            \
  public:                                                                 \
    const MethodInfo_ ## name& name() { return m_info ## name; }

#define IMPLEMENT_RMI(cls, name)                                                                                                                                                                                            \
  cls::MethodInfo_ ## name cls::m_info ## name = cls::Helper_AddMessage(&cls::Decode_ ## name, "RMI:" # cls ":" # name, cls::Attach_ ## name, cls::ServerCall_ ## name, cls::Reliability_ ## name, cls::LowDelay_ ## name); \
  INetAtSyncItem* cls::Decode_ ## name(TSerialize ser, EntityId * pID, INetChannel * pChannel)                                                                                                                              \
  {                                                                                                                                                                                                                         \
    CRY_ASSERT(pID);                                                                                                                                                                                                        \
    Params_ ## name params;                                                                                                                                                                                                 \
    params.SerializeWith(ser);                                                                                                                                                                                              \
    NetLogRMIReceived(params, pChannel);                                                                                                                                                                                    \
    return CRMIAtSyncItem<Params_ ## name, cls>::Create(params, *pID, m_info ## name.pMethodInfo, &cls::Handle_ ## name, pChannel);                                                                                         \
  }                                                                                                                                                                                                                         \
  ILINE bool cls::Handle_ ## name(const Params_ ## name & params, INetChannel * pNetChannel)

#define IMPLEMENT_INTERFACE_RMI(cls, name)                                                                                                                                                                                  \
  cls::MethodInfo_ ## name cls::m_info ## name = cls::Helper_AddMessage(&cls::Decode_ ## name, "RMI:" # cls ":" # name, cls::Attach_ ## name, cls::ServerCall_ ## name, cls::Reliability_ ## name, cls::LowDelay_ ## name); \
  INetAtSyncItem* cls::Decode_ ## name(TSerialize ser, INetChannel * pChannel)                                                                                                                                              \
  {                                                                                                                                                                                                                         \
    Params_ ## name params;                                                                                                                                                                                                 \
    params.SerializeWith(ser);                                                                                                                                                                                              \
    return CRMIAtSyncItem<Params_ ## name, cls>::Create(params, id, m_info ## name.pMethodInfo, &cls::Handle_ ## name, pChannel);                                                                                           \
  }                                                                                                                                                                                                                         \
  ILINE bool cls::Handle_ ## name(const Params_ ## name & params, INetChannel * pNetChannel)

/*
* _FAST versions may send the RMI without waiting for the frame to end; be sure that consistency with the entity is not important!
*/

//
// PreAttach/PostAttach RMI's cannot have their reliability specified (see CGameObjectDispatch::RegisterInterface() for details)
#define DECLARE_SERVER_RMI_PREATTACH(name, params)             DECLARE_RMI(name, params, eNRT_UnreliableOrdered, eRAT_PreAttach, true, false)
#define DECLARE_CLIENT_RMI_PREATTACH(name, params)             DECLARE_RMI(name, params, eNRT_UnreliableOrdered, eRAT_PreAttach, false, false)
#define DECLARE_SERVER_RMI_POSTATTACH(name, params)            DECLARE_RMI(name, params, eNRT_UnreliableOrdered, eRAT_PostAttach, true, false)
#define DECLARE_CLIENT_RMI_POSTATTACH(name, params)            DECLARE_RMI(name, params, eNRT_UnreliableOrdered, eRAT_PostAttach, false, false)
#define DECLARE_SERVER_RMI_NOATTACH(name, params, reliability) DECLARE_RMI(name, params, reliability, eRAT_NoAttach, true, false)
#define DECLARE_CLIENT_RMI_NOATTACH(name, params, reliability) DECLARE_RMI(name, params, reliability, eRAT_NoAttach, false, false)

// PreAttach/PostAttach RMI's cannot have their reliability specified (see CGameObjectDispatch::RegisterInterface() for details)
#define DECLARE_SERVER_RMI_PREATTACH_FAST(name, params)             DECLARE_RMI(name, params, eNRT_UnreliableOrdered, eRAT_PreAttach, true, true)
#define DECLARE_CLIENT_RMI_PREATTACH_FAST(name, params)             DECLARE_RMI(name, params, eNRT_UnreliableOrdered, eRAT_PreAttach, false, true)
#define DECLARE_SERVER_RMI_POSTATTACH_FAST(name, params)            DECLARE_RMI(name, params, eNRT_UnreliableOrdered, eRAT_PostAttach, true, true)
#define DECLARE_CLIENT_RMI_POSTATTACH_FAST(name, params)            DECLARE_RMI(name, params, eNRT_UnreliableOrdered, eRAT_PostAttach, false, true)
#define DECLARE_SERVER_RMI_NOATTACH_FAST(name, params, reliability) DECLARE_RMI(name, params, reliability, eRAT_NoAttach, true, true)
#define DECLARE_CLIENT_RMI_NOATTACH_FAST(name, params, reliability) DECLARE_RMI(name, params, reliability, eRAT_NoAttach, false, true)

#if ENABLE_URGENT_RMIS
#define DECLARE_SERVER_RMI_URGENT(name, params, reliability) DECLARE_RMI(name, params, reliability, eRAT_Urgent, true, false)
#define DECLARE_CLIENT_RMI_URGENT(name, params, reliability) DECLARE_RMI(name, params, reliability, eRAT_Urgent, false, false)
#else
#define DECLARE_SERVER_RMI_URGENT(name, params, reliability) DECLARE_SERVER_RMI_NOATTACH(name, params, reliability)
#define DECLARE_CLIENT_RMI_URGENT(name, params, reliability) DECLARE_CLIENT_RMI_NOATTACH(name, params, reliability)
#endif // ENABLE_URGENT_RMIS

#if ENABLE_INDEPENDENT_RMIS
#define DECLARE_SERVER_RMI_INDEPENDENT(name, params, reliability) DECLARE_RMI(name, params, reliability, eRAT_Independent, true, false)
#define DECLARE_CLIENT_RMI_INDEPENDENT(name, params, reliability) DECLARE_RMI(name, params, reliability, eRAT_Independent, false, false)
#else
#define DECLARE_SERVER_RMI_INDEPENDENT(name, params, reliability) DECLARE_SERVER_RMI_NOATTACH(name, params, reliability)
#define DECLARE_CLIENT_RMI_INDEPENDENT(name, params, reliability) DECLARE_CLIENT_RMI_NOATTACH(name, params, reliability)
#endif // ENABLE_INDEPENDENT_RMIS

/*
// Todo:
//		Temporary, until a good solution for sending noattach fast messages can be found
#define DECLARE_SERVER_RMI_NOATTACH_FAST(a,b,c) DECLARE_SERVER_RMI_NOATTACH(a,b,c)
#define DECLARE_CLIENT_RMI_NOATTACH_FAST(a,b,c) DECLARE_CLIENT_RMI_NOATTACH(a,b,c)
*/

//
#define DECLARE_INTERFACE_SERVER_RMI_PREATTACH(name, params, reliability)       DECLARE_INTERFACE_RMI(name, params, reliability, eRAT_PreAttach, true, false)
#define DECLARE_INTERFACE_CLIENT_RMI_PREATTACH(name, params, reliability)       DECLARE_INTERFACE_RMI(name, params, reliability, eRAT_PreAttach, false, false)
#define DECLARE_INTERFACE_SERVER_RMI_POSTATTACH(name, params, reliability)      DECLARE_INTERFACE_RMI(name, params, reliability, eRAT_PostAttach, true, false)
#define DECLARE_INTERFACE_CLIENT_RMI_POSTATTACH(name, params, reliability)      DECLARE_INTERFACE_RMI(name, params, reliability, eRAT_PostAttach, false, false)

#define DECLARE_INTERFACE_SERVER_RMI_PREATTACH_FAST(name, params, reliability)  DECLARE_INTERFACE_RMI(name, params, reliability, eRAT_PreAttach, true, true)
#define DECLARE_INTERFACE_CLIENT_RMI_PREATTACH_FAST(name, params, reliability)  DECLARE_INTERFACE_RMI(name, params, reliability, eRAT_PreAttach, false, true)
#define DECLARE_INTERFACE_SERVER_RMI_POSTATTACH_FAST(name, params, reliability) DECLARE_INTERFACE_RMI(name, params, reliability, eRAT_PostAttach, true, true)
#define DECLARE_INTERFACE_CLIENT_RMI_POSTATTACH_FAST(name, params, reliability) DECLARE_INTERFACE_RMI(name, params, reliability, eRAT_PostAttach, false, true)