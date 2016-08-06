#pragma once

#include <CryEntitySystem/INetworkedEntityComponent.h>
#include <IGameRulesSystem.h>

// Summary
//   Structure used to define a game object event
struct SGameObjectEvent
{
	SGameObjectEvent(uint32 event, uint16 flags, uint32 unusedTarget = 0, void* _param = 0)
	{
		this->event = event;
		this->flags = flags;
		this->param = _param;
	}
	uint32                         event;
	uint16                         flags;
	// optional parameter of event (ugly)
	union
	{
		void* param;
		bool  paramAsBool;
	};
};

struct IComponent
{
	enum
	{
		EComponentFlags_Enable = BIT(0),
		EComponentFlags_Disable = BIT(1),
	};
};

enum EUpdateEnableCondition
{
	eUEC_Never,
	eUEC_Always,
	eUEC_Visible,
	eUEC_InRange,
	eUEC_VisibleAndInRange,
	eUEC_VisibleOrInRange,
	eUEC_VisibleOrInRangeIgnoreAI,
	eUEC_VisibleIgnoreAI,
	eUEC_WithoutAI,
};

enum EGameObjectEventFlags
{
	eGOEF_ToScriptSystem = 0x0001,
	eGOEF_ToGameObject = 0x0002,
	eGOEF_ToExtensions = 0x0004,
	eGOEF_LoggedPhysicsEvent = 0x0008, // was this a logged or immediate physics event

	eGOEF_ToAll = eGOEF_ToScriptSystem | eGOEF_ToGameObject | eGOEF_ToExtensions
};

enum EPrePhysicsUpdate
{
	ePPU_Never,
	ePPU_Always,
	ePPU_WhenAIActivated
};

struct IGameObjectSystem
{
	static const uint32 InvalidExtensionID = 0;
};

struct IWrappedGameObject : public IGameObject
{
	void EnablePostUpdates(IEntityComponent *pComponent) {}
	void EnableUpdateSlot(IEntityComponent *pComponent, int slot) {}
	void DisableUpdateSlot(IEntityComponent *pComponent, int slot) {}

	void SetUpdateSlotEnableCondition(IEntityComponent *pComponent, int slot, EUpdateEnableCondition condition)
	{
		unsigned int updateFlags = 0;

		switch (condition)
		{
		case eUEC_Never:
			updateFlags = EEntityUpdatePolicy_Never;
			break;
		case eUEC_Always:
		case eUEC_WithoutAI:
			updateFlags = EEntityUpdatePolicy_Always;
			break;
		case eUEC_Visible:
		case eUEC_VisibleIgnoreAI:
			updateFlags = EEntityUpdatePolicy_Visible;
			break;
		case eUEC_VisibleAndInRange: // And not actually supported in new system
		case eUEC_VisibleOrInRange:
		case eUEC_VisibleOrInRangeIgnoreAI:
			updateFlags = EEntityUpdatePolicy_Visible | EEntityUpdatePolicy_InRange;
			break;
		}

		pComponent->SetUpdatePolicy(updateFlags);
	}

	void SendEvent(const SGameObjectEvent &event)
	{
		GetEntity()->SendComponentEvent(event.event, event.param);
	}

	// WARNING: there *MUST* be at least one frame between spawning ent and using this function to send an RMI if
	// that RMI is _FAST, otherwise the dependent entity is ignored
	template<class MI, class T>
	void InvokeRMIWithDependentObject(const MI method, const T& params, unsigned where, EntityId ent, int channel = -1) const
	{
		InvokeRMI_Primitive(method, params, where, 0, 0, channel, ent);
	}

	template<class MI, class T>
	void InvokeRMI(const MI method, const T& params, unsigned where, int channel = -1) const
	{
		InvokeRMI_Primitive(method, params, where, 0, 0, channel, 0);
	}
};

enum EWrappedGameObjectEvents
{
	eGFE_OnCollision = eGFE_End,
	eGFE_BecomeLocalPlayer,
	eGFE_OnPostStep,

	eGFE_Last
};

template<class T_Derived, class T_Parent = CNetworkedEntityComponent<T_Derived, IEntityComponent>, size_t MAX_STATIC_MESSAGES = 64>
class CEntityComponentConversionHelper : public T_Parent
{
public:
	// IEntityComponent
	virtual void Initialize(IEntity &entity) override
	{
		T_Parent::Initialize(entity);

		EnableEvent(ENTITY_EVENT_POST_SERIALIZE, 0, true);
		EnableEvent(ENTITY_EVENT_BECOME_LOCAL_PLAYER, 0, true);
		EnableEvent(ENTITY_EVENT_PHYS_POSTSTEP, 0, true);

		auto &gameObject = GetEntity()->AcquireExternalComponent<IGameObject>();
		Init(&gameObject);
	}

	virtual void PostInitialize() override
	{
		T_Parent::PostInitialize();

		PostInit(GetGameObject());
	}

	virtual void ProcessEvent(const SEntityEvent& event) override
	{
		T_Parent::ProcessEvent(event);

		switch (event.event)
		{
			case ENTITY_EVENT_POST_SERIALIZE:
			{
				PostSerialize();
			}
			break;
			case ENTITY_EVENT_BECOME_LOCAL_PLAYER:
			{
				SGameObjectEvent gameObjectEvent(eGFE_BecomeLocalPlayer, 0, 0, nullptr);
				HandleEvent(gameObjectEvent);
			}
			break;
			case ENTITY_EVENT_PHYS_POSTSTEP:
			{
				SGameObjectEvent gameObjectEvent(eGFE_OnPostStep, 0, 0, (void *)event.nParam[0]);
				HandleEvent(gameObjectEvent);
			}
			break;
		}
	}

	virtual void OnComponentEvent(uint32 eventId, void *pUserData) override
	{
		T_Parent::OnComponentEvent(eventId, pUserData);

		SGameObjectEvent gameObjectEvent(eventId, 0, 0, pUserData);
		HandleEvent(gameObjectEvent);
	}

	virtual bool NeedSerialize() override { return true; }

	virtual void Serialize(TSerialize ser) override
	{
		T_Parent::Serialize(ser);

		FullSerialize(ser);
	}

	virtual void OnEntityReload(SEntitySpawnParams& params, XmlNodeRef entityNode) override
	{
		T_Parent::OnEntityReload(params, entityNode);

		ReloadExtension(GetGameObject(), params);
		PostReloadExtension(GetGameObject(), params);
	}
	// ~IEntityComponent

	// IGameObjectExtension legacy functions
	virtual bool Init(IGameObject * pGameObject) { return true; }
	virtual void PostInit(IGameObject * pGameObject) {}

	inline IWrappedGameObject *GetGameObject() const
	{
		return GetEntity()->QueryComponent<IWrappedGameObject>();
	}

	virtual void FullSerialize(TSerialize ser) {}

	virtual bool ReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params) { return true; }
	virtual void PostReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params) {}

	virtual void SerializeSpawnInfo(TSerialize ser) {} // Not called anymore
	virtual void PostSerialize() {}

	virtual void HandleEvent(const SGameObjectEvent&) {}

	void SetGameObject(IGameObject *pGameObject) {}
	void ResetGameObject() {}

	void EnablePrePhysicsUpdate(EPrePhysicsUpdate eUpdate)
	{
		switch (eUpdate)
		{
		case ePPU_Always:
		case ePPU_WhenAIActivated: // We don't actually check this
			EnableEvent(ENTITY_EVENT_PREPHYSICSUPDATE, 0, true);
			break;
		case ePPU_Never:
			EnableEvent(ENTITY_EVENT_PREPHYSICSUPDATE, 0, false);
			break;
		}
	}

	void RegisterEvent(EEntityEvent event, uint32 componentFlags)
	{
		EnableEvent(event, 0, (componentFlags & IComponent::EComponentFlags_Enable) != 0);
	}
	// ~IGameObjectExtension
};

#define CHANGED_NETWORK_STATE(object, aspects)     do { /* IEntity * pEntity = object->GetGameObject()->GetEntity(); CryLogAlways("%s changed aspect %x (%s %d)", pEntity ? pEntity->GetName() : "NULL", aspects, __FILE__, __LINE__); */ object->GetGameObject()->ChangedNetworkState(aspects); } while (0)
#define CHANGED_NETWORK_STATE_GO(object, aspects)  do { /* IEntity * pEntity = object->GetEntity(); CryLogAlways("%s changed aspect %x (%s %d)", pEntity ? pEntity->GetName() : "NULL", aspects, __FILE__, __LINE__); */ object->ChangedNetworkState(aspects); } while (0)
#define CHANGED_NETWORK_STATE_REF(object, aspects) do { /* IEntity * pEntity = object.GetGameObject()->GetEntity(); CryLogAlways("%s changed aspect %x (%s %d)", pEntity ? pEntity->GetName() : "NULL", aspects, __FILE__, __LINE__); */ object.GetGameObject()->ChangedNetworkState(aspects); } while (0)

struct IWrappedGameRules : IGameRules
{
	struct SGameCollision
	{
		const EventPhysCollision* pCollision;
		IGameObject*              pSrc;
		IGameObject*              pTrg;
		IEntity*                  pSrcEntity;
		IEntity*                  pTrgEntity;
	};

	// Summary
	//		Gets called when two entities collide, gamerules should dispatch this
	//		call also to Script functions
	// Parameters
	//  pEvent - physics event containing the necessary info
	virtual bool OnCollision(const SGameCollision& event) = 0;

	// Summary
	//		Gets called when two entities collide, and determines if AI should receive stiulus
	// Parameters
	//  pEvent - physics event containing the necessary info
	virtual void OnCollision_NotifyAI(const EventPhys* pEvent) = 0;

	// IGameRules
	virtual bool ApproveCollision(const EventPhysCollision& collision) override
	{
		OnCollision_NotifyAI(&collision);

		SGameCollision gameCollision;
		memset(&gameCollision, 0, sizeof(SGameCollision));
		gameCollision.pCollision = &collision;
		if (collision.iForeignData[0] == PHYS_FOREIGN_ID_ENTITY)
		{
			//gameCollision.pSrcEntity = gEnv->pEntitySystem->GetEntityFromPhysics(gameCollision.collision.pEntity[0]);
			gameCollision.pSrcEntity = (IEntity*)collision.pForeignData[0];

			if(gameCollision.pSrcEntity != nullptr)
				gameCollision.pSrc = gameCollision.pSrcEntity->QueryComponent<IGameObject>();
		}
		if (collision.iForeignData[1] == PHYS_FOREIGN_ID_ENTITY)
		{
			//gameCollision.pTrgEntity = gEnv->pEntitySystem->GetEntityFromPhysics(gameCollision.collision.pEntity[1]);
			gameCollision.pTrgEntity = (IEntity*)collision.pForeignData[1];

			if (gameCollision.pTrgEntity != nullptr)
				gameCollision.pTrg = gameCollision.pTrgEntity->QueryComponent<IGameObject>();
		}

		if (gameCollision.pSrcEntity != nullptr)
		{
			gameCollision.pSrcEntity->SendComponentEvent(eGFE_OnCollision, (void *)&collision);
		}
		if (gameCollision.pTrgEntity != nullptr)
		{
			gameCollision.pTrgEntity->SendComponentEvent(eGFE_OnCollision, (void *)&collision);
		}

		if (gameCollision.pSrc)
		{
			IRenderNode* pNode = NULL;
			if (collision.iForeignData[1] == PHYS_FOREIGN_ID_ENTITY)
			{
				IEntity* pTarget = (IEntity*)collision.pForeignData[1];
				if (pTarget)
				{
					auto* pRenderProxy = pTarget->QueryComponent<IEntityRenderComponent>();
					if (pRenderProxy)
						pNode = pRenderProxy->GetRenderNode();
				}
			}
			else if (collision.iForeignData[1] == PHYS_FOREIGN_ID_STATIC)
				pNode = (IRenderNode*)collision.pForeignData[1];
			/*else
			if (collision.iForeignData[1] == PHYS_FOREIGN_ID_FOLIAGE)
			{
			CStatObjFoliage *pFolliage = (CStatObjFoliage*)collision.pForeignData[1];
			if (pFolliage)
			pNode = pFolliage->m_pVegInst;
			}*/
			if (pNode)
				gEnv->p3DEngine->SelectEntity(pNode);
		}

		return OnCollision(gameCollision);
	}
	// ~IGameRules
};

#define DECLARE_GAMEOBJECT_FACTORY DECLARE_FACTORY