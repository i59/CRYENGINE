// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:   Entity.h
//  Version:     v1.00
//  Created:     18/5/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description:
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __Entity_h__
	#define __Entity_h__

	#include <CryEntitySystem/IEntity.h>

//////////////////////////////////////////////////////////////////////////
// These headers cannot be replaced with forward references.
// They are needed for correct up casting from IEntityComponent to real component class.
#include "Components/RenderComponent.h"
#include "Components/PhysicsComponent.h"
#include "Components/ScriptComponent.h"
#include "Components/SubstitutionComponent.h"
//////////////////////////////////////////////////////////////////////////

// forward declarations.
struct IEntitySystem;
struct IEntityArchetype;
class CEntitySystem;
struct AIObjectParams;
struct SGridLocation;
struct SProximityElement;

// (MATT) This should really live in a minimal AI include, which right now we don't have  {2009/04/08}
	#ifndef INVALID_AIOBJECTID
typedef uint32 tAIObjectID;
		#define INVALID_AIOBJECTID ((tAIObjectID)(0))
	#endif

//////////////////////////////////////////////////////////////////////
class CEntity : public IEntity
{
	// Struct containing a component and the priority that the event it belongs to should receive
	struct SEventComponentInfo
	{
		SEventComponentInfo() : m_pComponent(nullptr) {}
		SEventComponentInfo(IEntityComponent *pComponent, uint32 priority)
			: m_pComponent(pComponent)
			, eventPriority(priority)
		{
		}

		// Pointer to the component
		IEntityComponent *m_pComponent;
		// Priority of the event, in order to determine which component gets it first
		uint32 eventPriority;
	};

	// Struct containing information on the components that have requested to receive this event
	struct SEventComponents
	{
		struct SCompare
		{
			bool operator() (const SEventComponentInfo& lhs, const SEventComponentInfo& rhs) const
			{
				if (lhs.eventPriority > rhs.eventPriority)
				{
					return true;
				}

				if (lhs.eventPriority < rhs.eventPriority)
				{
					return false;
				}

				if (lhs.m_pComponent->GetInterfaceId().hipart > rhs.m_pComponent->GetInterfaceId().hipart)
				{
					return true;
				}

				if (lhs.m_pComponent->GetInterfaceId().hipart < rhs.m_pComponent->GetInterfaceId().hipart)
				{
					return false;
				}

				return lhs.m_pComponent->GetInterfaceId().lopart > rhs.m_pComponent->GetInterfaceId().lopart;
			}
		};

		std::set<SEventComponentInfo, SCompare> m_components;
	};

	// Entity constructor.
	// Should only be called from Entity System.
	CEntity(SEntitySpawnParams& params);

public:
	// Entity destructor.
	// Should only be called from Entity System.
	virtual ~CEntity();

	IEntitySystem* GetEntitySystem() const { return g_pIEntitySystem; };

	// Called by entity system to complete initialization of the entity.
	bool Init(SEntitySpawnParams& params);
	// Called by EntitySystem every frame for each active entity.
	void Update(SEntityUpdateContext& ctx);
	// Called by EntitySystem at the end of each frame for each active entity.
	void PostUpdate(float frameTime);
	// Called by EntitySystem before entity is destroyed.
	void ShutDown(bool bRemoveAI = true, bool bRemoveProxies = true);

	//////////////////////////////////////////////////////////////////////////
	// IEntity interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual EntityId   GetId() const override   { return m_nID; }
	virtual EntityGUID GetGuid() const override { return m_guid; }

	//////////////////////////////////////////////////////////////////////////
	virtual IEntityClass*     GetClass() const override     { return m_pClass; }
	virtual IEntityArchetype* GetArchetype() const override { return m_pArchetype; }

	//////////////////////////////////////////////////////////////////////////
	virtual void   SetFlags(uint32 flags) override;
	virtual uint32 GetFlags() const override                                 { return m_flags; }
	virtual void   AddFlags(uint32 flagsToAdd) override                      { SetFlags(m_flags | flagsToAdd); }
	virtual void   ClearFlags(uint32 flagsToClear) override                  { SetFlags(m_flags & (~flagsToClear)); }
	virtual bool   CheckFlags(uint32 flagsToCheck) const override            { return (m_flags & flagsToCheck) == flagsToCheck; }

	virtual void   SetFlagsExtended(uint32 flags) override                   { m_flagsExtended = flags; }
	virtual uint32 GetFlagsExtended() const override                         { return m_flagsExtended; }

	virtual bool   IsGarbage() const override                                { return m_bGarbage; }
	virtual bool   IsLoadedFromLevelFile() const override                    { return m_bLoadedFromLevelFile; }
	ILINE void     SetLoadedFromLevelFile(const bool bIsLoadedFromLevelFile) { m_bLoadedFromLevelFile = bIsLoadedFromLevelFile; }

	//////////////////////////////////////////////////////////////////////////
	virtual void        SetName(const char* sName) override;
	virtual const char* GetName() const override { return m_szName.c_str(); }
	virtual string GetEntityTextDescription() const override;

	//////////////////////////////////////////////////////////////////////////
	virtual void     AttachChild(IEntity* pChildEntity, const SChildAttachParams& attachParams) override;
	virtual void     DetachAll(int nDetachFlags = 0) override;
	virtual void     DetachThis(int nDetachFlags = 0, int nWhyFlags = 0) override;
	virtual int      GetChildCount() const override;
	virtual IEntity* GetChild(int nIndex) const override;
	virtual void     EnableInheritXForm(bool bEnable);
	virtual IEntity* GetParent() const override { return (m_pBinds) ? m_pBinds->pParent : NULL; }
	virtual Matrix34 GetParentAttachPointWorldTM() const override;
	virtual bool     IsParentAttachmentValid() const override;
	virtual IEntity* GetAdam()
	{
		IEntity* pParent, * pAdam = this;
		while (pParent = pAdam->GetParent()) pAdam = pParent;
		return pAdam;
	}

	//////////////////////////////////////////////////////////////////////////
	virtual void SetWorldTM(const Matrix34& tm, int nWhyFlags = 0) override;
	virtual void SetLocalTM(const Matrix34& tm, int nWhyFlags = 0) override;

	// Set position rotation and scale at once.
	virtual void            SetPosRotScale(const Vec3& vPos, const Quat& qRotation, const Vec3& vScale, int nWhyFlags = 0) override;

	virtual Matrix34        GetLocalTM() const override;
	virtual const Matrix34& GetWorldTM() const override { return m_worldTM; }

	virtual void            GetWorldBounds(AABB& bbox) const override;
	virtual void            GetLocalBounds(AABB& bbox) const override;

	//////////////////////////////////////////////////////////////////////////
	virtual void        SetPos(const Vec3& vPos, int nWhyFlags = 0, bool bRecalcPhyBounds = false, bool bForce = false) override;
	virtual const Vec3& GetPos() const override { return m_vPos; }

	virtual void        SetRotation(const Quat& qRotation, int nWhyFlaSetUpdateStatusgs = 0) override;
	virtual const Quat& GetRotation() const override { return m_qRotation; }

	virtual void        SetScale(const Vec3& vScale, int nWhyFlags = 0) override;
	virtual const Vec3& GetScale() const override { return m_vScale; }

	virtual Vec3        GetWorldPos() const override { return m_worldTM.GetTranslation(); }
	virtual Ang3        GetWorldAngles() const override;
	virtual Quat        GetWorldRotation() const override;
	virtual const Vec3& GetForwardDir() const override { ComputeForwardDir(); return m_vForwardDir; }
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	virtual bool IsActive() const override { return ShouldUpdate(); }

	//////////////////////////////////////////////////////////////////////////
	virtual void Serialize(TSerialize ser, int nFlags) override;

	virtual void SetTimer(int nTimerId, int nMilliSeconds) override;
	virtual void KillTimer(int nTimerId) override;

	virtual void ScheduleRemoval(float timeToRemoveSeconds, EScheduledRemovalType type) override;
	virtual void CancelScheduledRemoval() override;

	virtual void Hide(bool bHide) override;
	virtual bool IsHidden() const override { return m_bHidden; }

	virtual void Invisible(bool bInvisible) override;
	virtual bool IsInvisible() const override { return m_bInvisible; }

	//////////////////////////////////////////////////////////////////////////
	virtual IAIObject*  GetAI() override                       { return (m_aiObjectID ? GetAIObject() : NULL); }
	virtual bool        HasAI() const override                 { return m_aiObjectID != 0; }
	virtual tAIObjectID GetAIObjectID() const override         { return m_aiObjectID; }
	virtual void        SetAIObjectID(tAIObjectID id) override { m_aiObjectID = id; }
	//////////////////////////////////////////////////////////////////////////
	virtual bool        RegisterInAISystem(const AIObjectParams& params) override;
	//////////////////////////////////////////////////////////////////////////
	// reflect changes in position or orientation to the AI object
	void UpdateAIObject();
	//////////////////////////////////////////////////////////////////////////

	virtual void RegisterComponent(const CryInterfaceID &interfaceID, std::shared_ptr<IEntityComponent> pComponent) override;
	virtual IEntityComponent *GetComponentByTypeId(const CryInterfaceID &interfaceID) const override;

	virtual IEntityComponent *CreateComponentByTypeId(const CryInterfaceID &interfaceID) override;

	virtual void EnableEvent(bool bEnable, IEntityComponent &component, EEntityEvent event, uint32 priority) override;
	virtual bool SendEvent(const SEntityEvent& event) override;

	virtual IEntityComponent *GetComponentWithRMIBase(const void *pBase) const override;

	virtual void SetComponentUpdatePolicy(IEntityComponent &component, unsigned int eUpdatePolicy) override;
	virtual unsigned int GetLastConditionalUpdateFlags() override { return m_lastConditionalUpdateFlags; }

	virtual void SendComponentEvent(uint32 eventId, void *pUserData = nullptr) override;

	//////////////////////////////////////////////////////////////////////////
	// Physics.
	//////////////////////////////////////////////////////////////////////////
	virtual void             Physicalize(SEntityPhysicalizeParams& params) override;
	virtual void             EnablePhysics(bool enable) override;

	virtual IPhysicalEntity* GetPhysics() const override;

	virtual int              PhysicalizeSlot(int slot, SEntityPhysicalizeParams& params) override;
	virtual void             UnphysicalizeSlot(int slot) override;
	virtual void             UpdateSlotPhysics(int slot) override;

	virtual void             SetPhysicsState(XmlNodeRef& physicsState) override;

	//////////////////////////////////////////////////////////////////////////
	// Custom entity material.
	//////////////////////////////////////////////////////////////////////////
	virtual void       SetMaterial(IMaterial* pMaterial) override;
	virtual IMaterial* GetMaterial() override;

	//////////////////////////////////////////////////////////////////////////
	// Entity Slots interface.
	//////////////////////////////////////////////////////////////////////////
	virtual bool                  IsSlotValid(int nSlot) const override;
	virtual void                  FreeSlot(int nSlot) override;
	virtual int                   GetSlotCount() const override;
	virtual bool                  GetSlotInfo(int nSlot, SEntitySlotInfo& slotInfo) const override;
	virtual const Matrix34&       GetSlotWorldTM(int nIndex) const override;
	virtual const Matrix34&       GetSlotLocalTM(int nIndex, bool bRelativeToParent) const override;
	virtual void                  SetSlotLocalTM(int nIndex, const Matrix34& localTM, int nWhyFlags = 0) override;
	virtual void                  SetSlotCameraSpacePos(int nIndex, const Vec3& cameraSpacePos) override;
	virtual void                  GetSlotCameraSpacePos(int nSlot, Vec3& cameraSpacePos) const override;
	virtual bool                  SetParentSlot(int nParentSlot, int nChildSlot) override;
	virtual void                  SetSlotMaterial(int nSlot, IMaterial* pMaterial) override;
	virtual void                  SetSlotFlags(int nSlot, uint32 nFlags) override;
	virtual uint32                GetSlotFlags(int nSlot) const override;
	virtual bool                  ShouldUpdateCharacter(int nSlot) const override;
	virtual ICharacterInstance*   GetCharacter(int nSlot) override;
	virtual int                   SetCharacter(ICharacterInstance* pCharacter, int nSlot) override;
	virtual IStatObj*             GetStatObj(int nSlot) override;
	virtual int                   SetStatObj(IStatObj* pStatObj, int nSlot, bool bUpdatePhysics, float mass = -1.0f) override;
	virtual IParticleEmitter*     GetParticleEmitter(int nSlot) override;
	virtual IGeomCacheRenderNode* GetGeomCacheRenderNode(int nSlot) override;

	virtual void                  MoveSlot(IEntity* targetIEnt, int nSlot) override;

	virtual int                   LoadGeometry(int nSlot, const char* sFilename, const char* sGeomName = NULL, int nLoadFlags = 0) override;
	virtual int                   LoadCharacter(int nSlot, const char* sFilename, int nLoadFlags = 0) override;
	#if defined(USE_GEOM_CACHES)
	virtual int                   LoadGeomCache(int nSlot, const char* sFilename) override;
	#endif
	virtual int                   SetParticleEmitter(int nSlot, IParticleEmitter* pEmitter, bool bSerialize = false) override;
	virtual int                   LoadParticleEmitter(int nSlot, IParticleEffect* pEffect, SpawnParams const* params = NULL, bool bPrime = false, bool bSerialize = false) override;
	virtual int                   LoadLight(int nSlot, CDLight* pLight) override;
	int                           LoadLightImpl(int nSlot, CDLight* pLight);

	virtual bool                  UpdateLightClipBounds(CDLight& light);
	int                           LoadCloud(int nSlot, const char* sFilename);
	int                           SetCloudMovementProperties(int nSlot, const SCloudMovementProperties& properties);
	int                           LoadFogVolume(int nSlot, const SFogVolumeProperties& properties);

	int                           FadeGlobalDensity(int nSlot, float fadeTime, float newGlobalDensity);
	int                           LoadVolumeObject(int nSlot, const char* sFilename);
	int                           SetVolumeObjectMovementProperties(int nSlot, const SVolumeObjectMovementProperties& properties);

	#if !defined(EXCLUDE_DOCUMENTATION_PURPOSE)
	virtual int LoadPrismObject(int nSlot);
	#endif // EXCLUDE_DOCUMENTATION_PURPOSE

	virtual void InvalidateTM(int nWhyFlags = 0, bool bRecalcPhyBounds = false) override;

	// Load/Save entity parameters in XML node.
	virtual void         SerializeXML(XmlNodeRef& node, bool bLoading, bool bFromInit) override;

	virtual IEntityLink* GetEntityLinks() override;
	virtual IEntityLink* AddEntityLink(const char* sLinkName, EntityId entityId, EntityGUID entityGuid = 0) override;
	virtual void         RemoveEntityLink(IEntityLink* pLink) override;
	virtual void         RemoveAllEntityLinks() override;
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	virtual IEntity* UnmapAttachedChild(int& partId) override;

	virtual void     GetMemoryUsage(ICrySizer* pSizer) const override;

	//////////////////////////////////////////////////////////////////////////
	// Local methods.
	//////////////////////////////////////////////////////////////////////////

	// Returns true if entity was already fully initialized by this point.
	virtual bool IsInitialized() const override { return m_bInitialized; }

	virtual void DebugDraw(const SGeometryDebugDrawInfo& info) override;
	//////////////////////////////////////////////////////////////////////////

	// Get fast access to the slot, only used internally by entity components.
	class CEntityObject* GetSlot(int nSlot) const;

	// For internal use.
	CEntitySystem* GetCEntitySystem() const { return g_pIEntitySystem; }

	//////////////////////////////////////////////////////////////////////////
	bool ReloadEntity(SEntityLoadParams& loadParams);

	// Check if the entity should update, and adds / removes from update list
	void CheckShouldUpdate();
	// Get status if entity need to be update every frame or not.
	bool ShouldUpdate() const 
	{ 
		return (m_numUpdatedComponents || m_scheduledRemovalType) && (!m_bHidden || CheckFlags(ENTITY_FLAG_UPDATE_HIDDEN));
	}

	//////////////////////////////////////////////////////////////////////////
	// Description:
	//   Invalidates entity and childs cached world transformation.
	void CalcLocalTM(Matrix34& tm) const;

	void Hide(bool bHide, EEntityEvent eEvent1, EEntityEvent eEvent2);

	// for ProximityTriggerSystem
	SProximityElement* GetProximityElement()   { return m_pProximityEntity; }

	virtual void       IncKeepAliveCounter() override   { m_nKeepAliveCounter++; }
	virtual void       DecKeepAliveCounter() override   { assert(m_nKeepAliveCounter > 0); m_nKeepAliveCounter--; }
	virtual void       ResetKeepAliveCounter() override { m_nKeepAliveCounter = 0; }
	virtual bool       IsKeptAlive() const override     { return m_nKeepAliveCounter > 0; }

	// LiveCreate entity interface
	virtual bool HandleVariableChange(const char* szVarName, const void* pVarData) override;

	#ifdef SEG_WORLD
	virtual unsigned int GetSwObjDebugFlag() const             { return m_eSwObjDebugFlag; };
	virtual void         SetSwObjDebugFlag(unsigned int uiVal) { m_eSwObjDebugFlag = uiVal; };

	virtual void         SetLocalSeg(bool bLocalSeg)           { m_bLocalSeg = bLocalSeg; }
	virtual bool         IsLocalSeg() const                    { return m_bLocalSeg; }
	#endif //SEG_WORLD

	void        CheckMaterialFlags();

	void        SetCloneLayerId(int cloneLayerId) { m_cloneLayerId = cloneLayerId; }
	int         GetCloneLayerId() const           { return m_cloneLayerId; }

protected:

	//////////////////////////////////////////////////////////////////////////
	// Attachment.
	//////////////////////////////////////////////////////////////////////////
	void AllocBindings();
	void DeallocBindings();
	void OnRellocate(int nWhyFlags);
	void LogEvent(const SEntityEvent& event, CTimeValue dt);
	//////////////////////////////////////////////////////////////////////////

private:
	void ComputeForwardDir() const;
	bool IsScaled(float threshold = 0.0003f) const
	{
		return (fabsf(m_vScale.x - 1.0f) + fabsf(m_vScale.y - 1.0f) + fabsf(m_vScale.z - 1.0f)) >= threshold;
	}
	// Fetch the IA object from the AIObjectID, if any
	IAIObject* GetAIObject();

private:
	//////////////////////////////////////////////////////////////////////////
	// VARIABLES.
	//////////////////////////////////////////////////////////////////////////
	friend class CEntitySystem;
	friend class CPhysicsEventListener; // For faster access to internals.
	friend class CEntityObject;         // For faster access to internals.
	friend class CRenderComponent;          // For faster access to internals.
	friend class CTriggerComponent;
	friend class CPhysicsComponent;

	// Childs structure, only allocated when any child entity is attached.
	struct SBindings
	{
		enum EBindingType
		{
			eBT_Pivot,
			eBT_GeomCacheNode,
			eBT_CharacterBone,
		};

		SBindings() : pParent(NULL), parentBindingType(eBT_Pivot), attachId(-1), childrenAttachIds(0) {}

		std::vector<CEntity*> childs;
		CEntity*              pParent;
		EBindingType          parentBindingType;
		int                   attachId;
		attachMask            childrenAttachIds; // bitmask of used attachIds of the children
	};

	//////////////////////////////////////////////////////////////////////////
	// Internal entity flags (must be first member var of CEntity) (Reduce cache misses on access to entity data).
	//////////////////////////////////////////////////////////////////////////
	unsigned int         m_bInActiveList       : 1; // Added to entity system active list.
	mutable unsigned int m_bBoundsValid        : 1; // Set when the entity bounding box is valid.
	unsigned int         m_bInitialized        : 1; // Set if this entity already Initialized.
	unsigned int         m_bHidden             : 1; // Set if this entity is hidden.
	unsigned int         m_bGarbage            : 1; // Set if this entity is garbage and will be removed soon.
	unsigned int         m_bHaveEventListeners : 1; // Set if entity have an event listeners associated in entity system.
	unsigned int         m_bTrigger            : 1; // Set if entity is proximity trigger itself.
	unsigned int         m_bWasRelocated       : 1; // Set if entity was relocated at least once.
	unsigned int m_lastConditionalUpdateFlags;

	unsigned int m_bInvisible           : 1;        // Set if this entity is invisible.
	unsigned int m_bNotInheritXform     : 1;        // Inherit or not transformation from parent.
	unsigned int m_bInShutDown          : 1;        // Entity is being shut down.

	mutable bool m_bDirtyForwardDir     : 1;    // Cached world transformed forward vector
	unsigned int m_bLoadedFromLevelFile : 1;    // Entity was loaded from level file
	#ifdef SEG_WORLD
	unsigned int m_eSwObjDebugFlag      : 2;
	bool         m_bLocalSeg            : 1;
	#endif //SEG_WORLD

	// Unique ID of the entity.
	EntityId m_nID;

	// Optional entity guid.
	EntityGUID m_guid;

	// Entity flags. any combination of EEntityFlags values.
	uint32 m_flags;

	// Entity extended flags. any combination of EEntityFlagsExtended values.
	uint32 m_flagsExtended;

	// Pointer to the class that describe this entity.
	IEntityClass* m_pClass;

	// Pointer to the entity archetype.
	IEntityArchetype* m_pArchetype;

	// Position of the entity in local space.
	Vec3             m_vPos;
	// Rotation of the entity in local space.
	Quat             m_qRotation;
	// Scale of the entity in local space.
	Vec3             m_vScale;
	// World transformation matrix of this entity.
	mutable Matrix34 m_worldTM;

	mutable Vec3     m_vForwardDir;

	// Pointer to hierarchical binding structure.
	// It contains array of child entities and pointer to the parent entity.
	// Because entity attachments are not used very often most entities do not need it,
	// and space is preserved by keeping it separate structure.
	SBindings* m_pBinds;

	// The representation of this entity in the AI system.
	tAIObjectID m_aiObjectID;

	// Custom entity material.
	_smart_ptr<IMaterial> m_pMaterial;

	struct SComponentInfo
	{
		SComponentInfo(std::shared_ptr<IEntityComponent> pEntComponent)
			: pComponent(pEntComponent)
			, updatePolicy(EEntityUpdatePolicy_Never)
		{
		}

		std::shared_ptr<IEntityComponent> pComponent;
		unsigned int updatePolicy;
		bool bShouldPostUpdate;
	};

	typedef std::unordered_map<const CryInterfaceID, SComponentInfo, stl::hash_guid> TEntityComponentMap;
	// Store components in map for lookup by type id
	TEntityComponentMap m_entityComponentMap;

	// Map indicating which components are listening to which events
	typedef std::unordered_map<EEntityEvent, SEventComponents> TEventComponentMap;
	TEventComponentMap m_eventComponentListenerMap;

	// Number of components that have set their update policy to something other than 0
	uint m_numUpdatedComponents;

	// Entity Links.
	IEntityLink* m_pEntityLinks;

	// For tracking entity in the partition grid.
	SGridLocation*     m_pGridLocation;
	// For tracking entity inside proximity trigger system.
	SProximityElement* m_pProximityEntity;

	// counter to prevent deletion if entity is processed deferred by for example physics events
	uint32 m_nKeepAliveCounter;

	// Name of the entity.
	string m_szName;

	// If this entity is part of a layer that was cloned at runtime, this is the cloned layer
	// id (not related to the layer id)
	int m_cloneLayerId;

	// Time in which the entity should be removed
	float m_scheduledRemovalTime;
	EScheduledRemovalType m_scheduledRemovalType;
};

//////////////////////////////////////////////////////////////////////////
void ILINE CEntity::ComputeForwardDir() const
{
	if (m_bDirtyForwardDir)
	{
		if (IsScaled())
		{
			Matrix34 auxTM = m_worldTM;
			auxTM.OrthonormalizeFast();

			// assuming (0, 1, 0) as the local forward direction
			m_vForwardDir = auxTM.GetColumn1();
		}
		else
		{
			// assuming (0, 1, 0) as the local forward direction
			m_vForwardDir = m_worldTM.GetColumn1();
		}

		m_bDirtyForwardDir = false;
	}
}

#endif // __Entity_h__
