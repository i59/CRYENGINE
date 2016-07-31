// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.
#pragma once

#include <CryNetwork/ISerialize.h>
#include <CryAudio/IAudioSystem.h>

#include <CryEntitySystem/IComponent.h>
#include <CryAction/ILipSyncProvider.h>

#include <CryCore/BitMask.h>

#include <CryExtension/ICryUnknown.h>

#include <CryMemory/CrySizer.h>

struct SEntitySpawnParams;
struct SEntityEvent;
struct SEntityUpdateContext;
struct IShaderPublicParams;
struct IFlowGraph;
struct IEntityEventListener;
struct IPhysicalEntity;
struct SSGHandle;
struct a2DPoint;
struct IRenderMesh;
struct IClipVolume;
struct IBSPTree3D;
struct IMaterial;
struct IScriptTable;
struct AABB;
typedef uint64 EntityGUID;  //!< Same as in IEntity.h.

struct IEntityComponent
{
	IEntityComponent()
		: m_pEntity(nullptr) {}

	virtual ~IEntityComponent() {}

	virtual void Initialize(IEntity &entity)
	{
		m_pEntity = &entity;
		m_entityId = entity.GetId();
	}

	virtual void PostInitialize() {}

	virtual void ProcessEvent(const SEntityEvent& event) {}

	virtual void Reload(SEntitySpawnParams& params, XmlNodeRef entityNode) {}
	virtual void Update(SEntityUpdateContext& ctx) {}
	virtual void PostUpdate(float frameTime) {}

	virtual void SerializeXML(XmlNodeRef& entityNode, bool bLoading, bool bFromInit) {}
	virtual void Serialize(TSerialize ser) {}

	virtual bool NeedSerialize() { return false; }
	
	virtual void GetMemoryUsage(ICrySizer* pSizer) const {}

	inline IEntity *GetEntity() const { return m_pEntity; }
	inline EntityId GetEntityId() const { return m_entityId; }

	// Template function used to query a component by entity id
	template <typename T>
	static T *QueryComponent(EntityId id)
	{
		if (IEntity *pEntity = gEnv->pEntitySystem->GetEntity(id))
		{
			return pEntity->QueryComponent<T>();
		}

		return nullptr;
	}

protected:
	IEntity *m_pEntity;
	EntityId m_entityId;
};

#define DECLARE_COMPONENT(name, iidHigh, iidLow) \
	static const CryInterfaceID &IID() \
	{ \
		static const CryInterfaceID cid = { (uint64) iidHigh ## LL, (uint64) iidLow ## LL };   \
		return cid; \
	}

template <typename T>
void RegisterEntityWithComponent(const char *name, const char *luaScriptPath = "")
{
	IEntityClassRegistry::SEntityClassDesc runtimeObjectDesc;
	runtimeObjectDesc.sName = name;
	runtimeObjectDesc.sScriptFile = luaScriptPath;

	struct SSpawnCallback
	{
		static void CreateComponent(IEntity& entity, SEntitySpawnParams& params, void* pUserData)
		{
			entity.AcquireComponent<T>();
		}
	};

	runtimeObjectDesc.pEntitySpawnCallback = SSpawnCallback::CreateComponent;

	gEnv->pEntitySystem->GetClassRegistry()->RegisterStdClass(runtimeObjectDesc);
}

// TODO: Check if we should move out default components below to another file

struct IShaderParamCallback;
DECLARE_SHARED_POINTERS(IShaderParamCallback);

//! Interface to the entity Render component.
struct IEntityRenderComponent : public IEntityComponent
{
	// <interfuscator:shuffle>}
	DECLARE_COMPONENT("EntityRenderComponent", 0x8FA27348E662401E, 0x916AE2A00CEACC82)

	//! Get world bounds of render proxy.
	//!\param[out] bounds Bounding box in world space.
	virtual void GetWorldBounds(AABB& bounds) = 0;

	//! Get local space int the entity bounds of render proxy.
	//! \param[out] bounds Bounding box in local entity space.
	virtual void GetLocalBounds(AABB& bounds) = 0;

	//! Force local bounds.
	//! \param[out] bounds Bounding box in local space.
	//! \param bDoNotRecalculate When set to true entity will never try to recalculate local bounding box set by this call.
	virtual void SetLocalBounds(const AABB& bounds, bool bDoNotRecalculate) = 0;

	//! Invalidates local or world space bounding box.
	virtual void InvalidateLocalBounds() = 0;

	//! Retrieve an actual material used for rendering specified slot.
	//! Will return custom applied material or if custom material not set will return an actual material assigned to the slot geometry.
	//! \param nSlot Slot to query used material from, if -1 material will be taken from the first renderable slot.
	//! \return Material used for rendering, or NULL if slot is not rendered.
	virtual IMaterial* GetRenderMaterial(int nSlot = -1) = 0;

	//! Assign custom material to the slot.
	//! \param nSlot Slot to apply material to.
	virtual void SetSlotMaterial(int nSlot, IMaterial* pMaterial) = 0;

	//! Retrieve slot's custom material (This material Must have been applied before with the SetSlotMaterial).
	//! \param nSlot Slot to query custom material from.
	//! \return Custom material applied on the slot.
	virtual IMaterial* GetSlotMaterial(int nSlot) = 0;

	//! Retrieve engine render node, used to render this entity.
	virtual IRenderNode* GetRenderNode() = 0;

	//! Retrieve and optionally create a shaders public params for this render proxy.
	//! \param bCreate If Shader public params are not created for this entity, they will be created.
	virtual IShaderPublicParams* GetShaderPublicParams(bool bCreate = true) = 0;

	//! Add a render proxy callback.
	virtual void AddShaderParamCallback(IShaderParamCallbackPtr pCallback) = 0;

	//! Remove a render proxy callback.
	virtual bool RemoveShaderParamCallback(IShaderParamCallbackPtr pCallback) = 0;

	//! Check which shader param callback should be activated or disactived depending on the current entity state.
	virtual void CheckShaderParamCallbacks() = 0;

	//! Clears all the shader param callback from this entity.
	virtual void ClearShaderParamCallbacks() = 0;

	//! Assign sub-object hide mask to slot.
	//! \param nSlot Slot to apply hide mask to.
	virtual void     SetSubObjHideMask(int nSlot, hidemask nSubObjHideMask) = 0;
	virtual hidemask GetSubObjHideMask(int nSlot) const = 0;

	//! Updates indirect lighting for children.
	virtual void UpdateIndirLightForChildren() {};

	//! Set material layers masks.
	virtual void  SetMaterialLayersMask(uint8 nMtlLayersMask) = 0;
	virtual uint8 GetMaterialLayersMask() const = 0;

	//! Overrides material layers blend amount.
	virtual void   SetMaterialLayersBlend(uint32 nMtlLayersBlend) = 0;
	virtual uint32 GetMaterialLayersBlend() const = 0;

	//! Set cloak interference state.
	virtual void SetCloakInterferenceState(bool bHasCloakInterference) = 0;
	virtual bool GetCloakInterferenceState() const = 0;

	//! Set cloak highlight strength.
	virtual void SetCloakHighlightStrength(float highlightStrength) = 0;

	//! Set cloak color channel.
	virtual void  SetCloakColorChannel(uint8 nCloakColorChannel) = 0;
	virtual uint8 GetCloakColorChannel() const = 0;

	//! Set cloak fade by distance.
	virtual void SetCloakFadeByDistance(bool bCloakFadeByDistance) = 0;
	virtual bool DoesCloakFadeByDistance() const = 0;

	//! Set timescale for cloak blending (range 1 - 4).
	virtual void  SetCloakBlendTimeScale(float fCloakBlendTimeScale) = 0;
	virtual float GetCloakBlendTimeScale() const = 0;

	//! Set ignore cloak refraction color.
	virtual void SetIgnoreCloakRefractionColor(bool bIgnoreCloakRefractionColor) = 0;
	virtual bool DoesIgnoreCloakRefractionColor() const = 0;

	//! Set custom post effect.
	virtual void SetCustomPostEffect(const char* pPostEffectName) = 0;

	//! Set object to be rendered in post 3d pass.
	//! \param bPost3dRenderObject Flag object to render in post 3d render pass.
	//! \param groupId Assign group ID's to groups of objects that render in the same screen rect.
	//! \param groupScreenRect Screen rect that the grouped objects render in (0.0->1.0).
	virtual void SetAsPost3dRenderObject(bool bPost3dRenderObject, uint8 groupId, float groupScreenRect[4]) = 0;

	//! Set hud render proxy to ignore hud interference filter.
	virtual void SetIgnoreHudInterferenceFilter(const bool bIgnoreFiler) = 0;

	//! Set whether 3D HUD Objects require to be rendered at correct depth (i.e. behind weapon).
	virtual void SetHUDRequireDepthTest(const bool bRequire) = 0;

	//! Set whether 3D HUD Object should have bloom disabled (Required to avoid overglow and cutoff with alien hud ghosted planes).
	virtual void SetHUDDisableBloom(const bool bDisableBloom) = 0;

	//! Set render proxy to ignore heat value of object.
	virtual void SetIgnoreHeatAmount(bool bIgnoreHeat) = 0;

	//! S vision params (thermal/sonar/etc).
	virtual void   SetVisionParams(float r, float g, float b, float a) = 0;
	virtual uint32 GetVisionParams() const = 0;

	//! Set hud silhouetes params.
	virtual void   SetHUDSilhouettesParams(float r, float g, float b, float a) = 0;
	virtual uint32 GetHUDSilhouettesParams() const = 0;

	//! S shadow dissolving (fade out for phantom perk etc).
	virtual void SetShadowDissolve(bool enable) = 0;
	virtual bool GetShadowDissolve() const = 0;

	//! Set effect layer params (used for game related layer effects - eg: nanosuit effects).
	virtual void         SetEffectLayerParams(const Vec4& pParams) = 0;
	virtual void         SetEffectLayerParams(uint32 nEncodedParams) = 0;
	virtual const uint32 GetEffectLayerParams() const = 0;

	//! Set opacity.
	virtual void  SetOpacity(float fAmount) = 0;
	virtual float GetOpacity() const = 0;

	//! \return the last time (as set by the system timer) when the renderproxy was last seen.
	virtual float GetLastSeenTime() const = 0;

	//! \return true if entity visarea was visible during last frames.
	virtual bool IsRenderProxyVisAreaVisible() const = 0;

	//! Removes all slots from the render proxy.
	virtual void ClearSlots() = 0;

	//! Enables / Disables motion blur.
	virtual void SetMotionBlur(bool enable) = 0;

	//! Set the viewDistRatio on the render node.
	virtual void SetViewDistRatio(int nViewDistRatio) = 0;

	//! Sets the LodRatio on the render node.
	virtual void SetLodRatio(int nLodRatio) = 0;

	// </interfuscator:shuffle>
};

//! Physical proxy interface.
struct IEntityPhysicsComponent : public IEntityComponent
{
	// <interfuscator:shuffle>
	DECLARE_COMPONENT("EntityPhysicsComponent", 0x7583A7358303455B, 0x83306428E4C7373C)

	//! Assign a pre-created physical entity to this proxy.
	//! \param pPhysEntity The pre-created physical entity.
	//! \param nSlot Slot Index to which the new position will be taken from.
	virtual void AssignPhysicalEntity(IPhysicalEntity* pPhysEntity, int nSlot = -1) = 0;

	//! Get world bounds of physical object.
	//! \param[out] bounds Bounding box in world space.
	virtual void GetWorldBounds(AABB& bounds) = 0;

	//! Get local space physical bounding box.
	//! \param[out] bounds Bounding box in local entity space.
	virtual void             GetLocalBounds(AABB& bounds) = 0;

	virtual void             Physicalize(SEntityPhysicalizeParams& params) = 0;
	virtual void             ReattachSoftEntityVtx(IPhysicalEntity* pAttachToEntity, int nAttachToPart) = 0;
	virtual IPhysicalEntity* GetPhysicalEntity() const = 0;

	virtual void             SerializeTyped(TSerialize ser, int type, int flags) = 0;

	//! Enable or disable physical simulation.
	virtual void EnablePhysics(bool bEnable) = 0;

	//! Is physical simulation enabled?
	virtual bool IsPhysicsEnabled() const = 0;

	//! Add impulse to physical entity.
	virtual void AddImpulse(int ipart, const Vec3& pos, const Vec3& impulse, bool bPos, float fAuxScale, float fPushScale = 1.0f) = 0;

	//! Creates a trigger bounding box.
	//! When physics will detect collision with this bounding box it will send an events to the entity.
	//! If entity have script OnEnterArea and OnLeaveArea events will be called.
	//! \param bbox Axis aligned bounding box of the trigger in entity local space (Rotation and scale of the entity is ignored). Set empty bounding box to disable trgger.
	virtual void SetTriggerBounds(const AABB& bbox) = 0;

	//! Retrieve trigger bounding box in local space.
	//! \return Axis aligned bounding box of the trigger in the local space.
	virtual void GetTriggerBounds(AABB& bbox) = 0;

	//! Physicalizes the foliage of StatObj in slot iSlot.
	virtual bool PhysicalizeFoliage(int iSlot) = 0;

	//! Dephysicalizes the foliage in slot iSlot.
	virtual void DephysicalizeFoliage(int iSlot) = 0;

	//! returns foliage object in slot iSlot.
	virtual struct IFoliage* GetFoliage(int iSlot) = 0;

	//! retrieve starting partid for a given slot.
	virtual int GetPartId0(int islot = 0) = 0;

	//! Enable/disable network serialisation of the physics aspect.
	virtual void EnableNetworkSerialization(bool enable) = 0;

	//! Have the physics ignore the XForm event.
	virtual void IgnoreXFormEvent(bool ignore) = 0;
	// </interfuscator:shuffle>
};

struct IEntityScript;

//! Script proxy interface.
struct IEntityScriptComponent : public IEntityComponent
{
	// <interfuscator:shuffle>
	DECLARE_COMPONENT("EntityScriptComponent", 0xE4748A2B5D1F4AC7, 0x94AEE7E5435FDD17)

	virtual void          SetScriptUpdateRate(float fUpdateEveryNSeconds) = 0;
	virtual IScriptTable* GetScriptTable() = 0;
	virtual void          CallEvent(const char* sEvent) = 0;
	virtual void          CallEvent(const char* sEvent, float fValue) = 0;
	virtual void          CallEvent(const char* sEvent, bool bValue) = 0;
	virtual void          CallEvent(const char* sEvent, const char* sValue) = 0;
	virtual void          CallEvent(const char* sEvent, const Vec3& vValue) = 0;
	virtual void          CallEvent(const char* sEvent, EntityId nEntityId) = 0;

	//! Change current state of the entity script.
	//! \return If state was successfully set.
	virtual bool GotoState(const char* sStateName) = 0;

	//! Change current state of the entity script.
	//! \return If state was successfully set.
	virtual bool GotoStateId(int nStateId) = 0;

	//! Check if entity is in specified state.
	//! \param sStateName Name of state table within entity script (case sensetive).
	//! \return If entity script is in specified state.
	virtual bool IsInState(const char* sStateName) = 0;

	//! Retrieves name of the currently active entity script state.
	//! \return Name of current state.
	virtual const char* GetState() = 0;

	//! Retrieves the id of the currently active entity script state.
	//! \return Index of current state.
	virtual int GetStateId() = 0;

	//! Fires an event in the entity script.
	//! This will call OnEvent(id,param) Lua function in entity script, so that script can handle this event.
	virtual void SendScriptEvent(int Event, IScriptTable* pParamters, bool* pRet = NULL) = 0;
	virtual void SendScriptEvent(int Event, const char* str, bool* pRet = NULL) = 0;
	virtual void SendScriptEvent(int Event, int nParam, bool* pRet = NULL) = 0;

	//! Change the Entity Script used by the Script Proxy.
	//! Caller is responsible for making sure new script is initialised and script bound as required
	//! \param pScript an entity script object that has already been loaded with the new script.
	//! \param pScriptTable Used to set the properties table if required.
	virtual void ChangeScript(IEntityScript* pScript, IScriptTable *pScriptTable) = 0;
	// </interfuscator:shuffle>
};

inline IScriptTable* IEntity::GetScriptTable() const
{
	if (auto *pScriptComponent = QueryComponent<IEntityScriptComponent>())
		return pScriptComponent->GetScriptTable();

	return nullptr;
}

//! Proximity trigger proxy interface.
struct IEntityTriggerComponent : public IEntityComponent
{
	// <interfuscator:shuffle>
	DECLARE_COMPONENT("EntityTriggerComponent", 0x6A998A93CCB842B3, 0xBF6FEC6D87CE6259)

	//! Creates a trigger bounding box.
	//! When physics will detect collision with this bounding box it will send an events to the entity.
	//! If entity have script OnEnterArea and OnLeaveArea events will be called.
	//! \param bbox Axis aligned bounding box of the trigger in entity local space (Rotation and scale of the entity is ignored). Set empty bounding box to disable trigger.
	virtual void SetTriggerBounds(const AABB& bbox) = 0;

	//! Retrieve trigger bounding box in local space.
	//! \return Axis aligned bounding box of the trigger in the local space.
	virtual void GetTriggerBounds(AABB& bbox) = 0;

	//! Forward enter/leave events to this entity
	virtual void ForwardEventsTo(EntityId id) = 0;

	//! Invalidate the trigger, so it gets recalculated and catches things which are already inside when it gets enabled.
	virtual void InvalidateTrigger() = 0;
	// </interfuscator:shuffle>
};

//! Entity Audio Proxy interface.
struct IEntityAudioComponent : public IEntityComponent
{
	// <interfuscator:shuffle>
	DECLARE_COMPONENT("EntityAudioComponent", 0xCAC7D72F0C6F4DB9, 0x8E19D932CFA719F1)

	virtual void               SetFadeDistance(float const fadeDistance) = 0;
	virtual float              GetFadeDistance() const = 0;
	virtual void               SetEnvironmentFadeDistance(float const environmentFadeDistance) = 0;
	virtual float              GetEnvironmentFadeDistance() const = 0;
	virtual float              GetGreatestFadeDistance() const = 0;
	virtual void               SetEnvironmentId(AudioEnvironmentId const environmentId) = 0;
	virtual AudioEnvironmentId GetEnvironmentId() const = 0;
	virtual AudioProxyId       CreateAuxAudioProxy() = 0;
	virtual bool               RemoveAuxAudioProxy(AudioProxyId const audioProxyId) = 0;
	virtual void               SetAuxAudioProxyOffset(Matrix34 const& offset, AudioProxyId const audioProxyId = DEFAULT_AUDIO_PROXY_ID) = 0;
	virtual Matrix34 const&    GetAuxAudioProxyOffset(AudioProxyId const audioProxyId = DEFAULT_AUDIO_PROXY_ID) = 0;
	virtual bool               PlayFile(SAudioPlayFileInfo const& _playbackInfo, AudioProxyId const _audioProxyId = DEFAULT_AUDIO_PROXY_ID, SAudioCallBackInfo const& _callBackInfo = SAudioCallBackInfo::GetEmptyObject()) = 0;
	virtual void               StopFile(char const* const _szFile, AudioProxyId const _audioProxyId = DEFAULT_AUDIO_PROXY_ID) = 0;
	virtual bool               ExecuteTrigger(AudioControlId const audioTriggerId, AudioProxyId const audioProxyId = DEFAULT_AUDIO_PROXY_ID, SAudioCallBackInfo const& callBackInfo = SAudioCallBackInfo::GetEmptyObject()) = 0;
	virtual void               StopTrigger(AudioControlId const audioTriggerId, AudioProxyId const audioProxyId = DEFAULT_AUDIO_PROXY_ID) = 0;
	virtual void               SetSwitchState(AudioControlId const audioSwitchId, AudioSwitchStateId const audioStateId, AudioProxyId const audioProxyId = DEFAULT_AUDIO_PROXY_ID) = 0;
	virtual void               SetRtpcValue(AudioControlId const audioRtpcId, float const value, AudioProxyId const audioProxyId = DEFAULT_AUDIO_PROXY_ID) = 0;
	virtual void               SetObstructionCalcType(EAudioOcclusionType const occlusionType, AudioProxyId const audioProxyId = DEFAULT_AUDIO_PROXY_ID) = 0;
	virtual void               SetEnvironmentAmount(AudioEnvironmentId const audioEnvironmentId, float const amount, AudioProxyId const audioProxyId = DEFAULT_AUDIO_PROXY_ID) = 0;
	virtual void               SetCurrentEnvironments(AudioProxyId const audioProxyId = DEFAULT_AUDIO_PROXY_ID) = 0;
	virtual void               AuxAudioProxiesMoveWithEntity(bool const bCanMoveWithEntity) = 0;
	virtual void               AddAsListenerToAuxAudioProxy(AudioProxyId const audioProxyId, void(*func)(SAudioRequestInfo const* const), EAudioRequestType requestType = eAudioRequestType_AudioAllRequests, AudioEnumFlagsType specificRequestMask = ALL_AUDIO_REQUEST_SPECIFIC_TYPE_FLAGS) = 0;
	virtual void               RemoveAsListenerFromAuxAudioProxy(AudioProxyId const audioProxyId, void(*func)(SAudioRequestInfo const* const)) = 0;
	// </interfuscator:shuffle>
};

//! Type of an area managed by IEntityAreaComponent.
enum EEntityAreaType
{
	ENTITY_AREA_TYPE_SHAPE,          //!< Area type is a closed set of points forming shape.
	ENTITY_AREA_TYPE_BOX,            //!< Area type is a oriented bounding box.
	ENTITY_AREA_TYPE_SPHERE,         //!< Area type is a sphere.
	ENTITY_AREA_TYPE_GRAVITYVOLUME,  //!< Area type is a volume around a bezier curve.
	ENTITY_AREA_TYPE_SOLID           //!< Area type is a solid which can have any geometry figure.
};

//! Area proxy allow for entity to host an area trigger.
//! Area can be shape, box or sphere, when marked entities cross this area border,
//! it will send ENTITY_EVENT_ENTERAREA, ENTITY_EVENT_LEAVEAREA, and ENTITY_EVENT_AREAFADE
//! events to the target entities.
struct IEntityAreaComponent : public IEntityComponent
{
	enum EAreaProxyFlags
	{
		FLAG_NOT_UPDATE_AREA = BIT(1), //!< When set points in the area will not be updated.
		FLAG_NOT_SERIALIZE = BIT(2)  //!< Areas with this flag will not be serialized.
	};

	// <interfuscator:shuffle>
	DECLARE_COMPONENT("EntityAreaComponent", 0x45261B615EC744B2, 0xA52AF6716B4E319A)

	//! Area flags.
	virtual void SetFlags(int nAreaProxyFlags) = 0;

	//! Area flags.
	virtual int GetFlags() = 0;

	//! Retrieve area type.
	//! \return One of EEntityAreaType enumerated types.
	virtual EEntityAreaType GetAreaType() const = 0;

	//! Sets area to be a shape, and assign points to this shape.
	//! Points are specified in local entity space, shape will always be constructed in XY plane,
	//! lowest Z of specified points will be used as a base Z plane.
	//! If fHeight parameter is 0, shape will be considered 2D shape, and during intersection Z will be ignored
	//! If fHeight is not zero shape will be considered 3D and will accept intersection within vertical range from baseZ to baseZ+fHeight.
	//! \param pPoints                   Array of 3D vectors defining shape vertices.
	//! \param pSoundObstructionSegments Array of corresponding booleans that indicate sound obstruction.
	//! \param numLocalPoints            Number of vertices in vPoints array.
	//! \param height                    Height of the shape.
	virtual void SetPoints(Vec3 const* const pPoints, bool const* const pSoundObstructionSegments, size_t const numLocalPoints, float const height) = 0;

	//! Sets area to be a Box, min and max must be in local entity space.
	//! Host entity orientation will define the actual world position and orientation of area box.
	virtual void SetBox(const Vec3& min, const Vec3& max, const bool* const pabSoundObstructionSides, size_t const nSideCount) = 0;

	//! Sets area to be a Sphere, center and radius must be specified in local entity space.
	//! Host entity world position will define the actual world position of the area sphere.
	virtual void SetSphere(const Vec3& vCenter, float fRadius) = 0;

	//! This function need to be called before setting convex hulls for a AreaSolid.
	//! Then AddConvexHullSolid() function is called as the number of convexhulls consisting of a geometry.
	//! \see AddConvexHullToSolid, EndSettingSolid
	virtual void BeginSettingSolid(const Matrix34& worldTM) = 0;

	//! Add a convex hull to a solid. This function have to be called after calling BeginSettingSolid()
	//! \see BeginSettingSolid, EndSettingSolid
	virtual void AddConvexHullToSolid(const Vec3* verticesOfConvexHull, bool bObstruction, int numberOfVertices) = 0;

	//! Finish setting a solid geometry. Generally the BSPTree based on convex hulls which is set before is created in this function.
	//!\see BeginSettingSolid, AddConvexHullToSolid
	virtual void EndSettingSolid() = 0;

	//! Retrieve number of points for shape area, return 0 if not area type is not shape.
	virtual int GetPointsCount() = 0;

	//! Retrieve array of points for shape area, will return NULL for all other area types.
	virtual const Vec3* GetPoints() = 0;

	//! Set shape area height, if height is 0 area is 2D.
	virtual void SetHeight(float const value) = 0;

	//! Retrieve shape area height, if height is 0 area is 2D.
	virtual float GetHeight() const = 0;

	//! Retrieve min and max in local space of area box.
	virtual void GetBox(Vec3& min, Vec3& max) = 0;

	//! Retrieve center and radius of the sphere area in local space.
	virtual void GetSphere(Vec3& vCenter, float& fRadius) = 0;

	virtual void SetGravityVolume(const Vec3* pPoints, int nNumPoints, float fRadius, float fGravity, bool bDontDisableInvisible, float fFalloff, float fDamping) = 0;

	//! Set area ID, this id will be provided to the script callback OnEnterArea, OnLeaveArea.
	virtual void SetID(const int id) = 0;

	//! Retrieve area ID.
	virtual int GetID() const = 0;

	//! Set area group id, areas with same group id act as an exclusive areas.
	//! If 2 areas with same group id overlap, entity will be considered in the most internal area (closest to entity).
	virtual void SetGroup(const int id) = 0;

	//! Retrieve area group id.
	virtual int GetGroup() const = 0;

	//! Set priority defines the individual priority of an area,
	//! Area with same group id will depend on which has the higher priority
	virtual void SetPriority(const int nPriority) = 0;

	//! Retrieve area priority.
	virtual int GetPriority() const = 0;

	//! Sets sound obstruction depending on area type
	virtual void SetSoundObstructionOnAreaFace(size_t const index, bool const bObstructs) = 0;

	//! Add target entity to the area.
	//! When someone enters/leaves an area, it will send ENTERAREA, LEAVEAREA, AREAFADE, events to these target entities.
	virtual void AddEntity(EntityId id) = 0;

	//! Add target entity to the area.
	//! When someone enters/leaves an area, it will send ENTERAREA, LEAVEAREA, AREAFADE, events to these target entities.
	virtual void AddEntity(EntityGUID guid) = 0;

	//! Remove target entity from the area.
	//! When someone enters/leaves an area, it will send ENTERAREA, LEAVEAREA, AREAFADE, events to these target entities.
	virtual void RemoveEntity(EntityId const id) = 0;

	//! Remove target entity from the area.
	//! When someone enters/leaves an area, it will send ENTERAREA ,LEAVEAREA, AREAFADE, events to these target entities.
	virtual void RemoveEntity(EntityGUID const guid) = 0;

	//! Removes all added target entities.
	virtual void RemoveEntities() = 0;

	//! Set area proximity region near the border.
	//! When someone is moving within this proximity region from the area outside border
	//! Area will generate ENTITY_EVENT_AREAFADE event to the target entity, with a fade ratio from 0, to 1.
	//! Where 0 will be at the area outside border, and 1 inside the area in distance fProximity from the outside area border.
	virtual void SetProximity(float fProximity) = 0;

	//! Retrieves area proximity.
	virtual float GetProximity() = 0;

	//! Compute and return squared distance to a point which is outside
	//! OnHull3d is the closest point on the hull of the area
	virtual float CalcPointNearDistSq(EntityId const nEntityID, Vec3 const& Point3d, Vec3& OnHull3d) = 0;

	//! Computes and returns squared distance from a point to the hull of the area
	//! OnHull3d is the closest point on the hull of the area
	//! This function is not sensitive of if the point is inside or outside the area
	virtual float ClosestPointOnHullDistSq(EntityId const nEntityID, Vec3 const& Point3d, Vec3& OnHull3d) = 0;

	//! Checks if a given point is inside the area.
	//! \note Ignoring the height speeds up the check.
	virtual bool CalcPointWithin(EntityId const nEntityID, Vec3 const& Point3d, bool const bIgnoreHeight = false) const = 0;

	//! get number of entities in area
	virtual size_t GetNumberOfEntitiesInArea() const = 0;

	//! get entity in area by index
	virtual EntityId GetEntityInAreaByIdx(size_t const index) const = 0;

	//! Retrieve inner fade distance of this area.
	virtual float GetInnerFadeDistance() const = 0;

	//! Set this area's inner fade distance.
	virtual void SetInnerFadeDistance(float const distance) = 0;

	// </interfuscator:shuffle>
};

struct IClipVolumeComponent : public IEntityComponent
{
	DECLARE_COMPONENT("ClipVolumeComponent", 0xC4F90161B5424E2A, 0x8829B85A69EE0F10)

	virtual void         UpdateRenderMesh(IRenderMesh* pRenderMesh, const DynArray<Vec3>& meshFaces) = 0;
	virtual IClipVolume* GetClipVolume() const = 0;
	virtual IBSPTree3D*  GetBspTree() const = 0;
	virtual void         SetProperties(bool bIgnoresOutdoorAO) = 0;
};

//! Flow Graph proxy allows entity to host reference to the flow graph.
struct IEntityFlowGraphComponent : public IEntityComponent
{
	// <interfuscator:shuffle>
	DECLARE_COMPONENT("EntityFlowGraphComponent", 0x7512969C9251477E, 0x9BB892BF876FAD56)

	virtual void        SetFlowGraph(IFlowGraph* pFlowGraph) = 0;
	virtual IFlowGraph* GetFlowGraph() = 0;

	virtual void        AddEventListener(IEntityEventListener* pListener) = 0;
	virtual void        RemoveEventListener(IEntityEventListener* pListener) = 0;
	// </interfuscator:shuffle>
};

//! Substitution proxy remembers IRenderNode this entity substitutes and unhides it upon deletion
struct IEntitySubstitutionComponent : public IEntityComponent
{
	// <interfuscator:shuffle>
	DECLARE_COMPONENT("EntitySubstitutionComponent", 0x75AE31A76D584AAE, 0xBD6EB757380F224B)

	virtual void         SetSubstitute(IRenderNode* pSubstitute) = 0;
	virtual IRenderNode* GetSubstitute() = 0;
	// </interfuscator:shuffle>
};

//! Represents entity camera.
struct IEntityCameraComponent : public IEntityComponent
{
	// <interfuscator:shuffle>
	DECLARE_COMPONENT("EntityCameraComponent", 0x7952D8B269C64249, 0x879F9255694DFF4F)

	virtual void     SetCamera(CCamera& cam) = 0;
	virtual CCamera& GetCamera() = 0;
	// </interfuscator:shuffle>
};

//! Proxy for the entity rope.
struct IEntityRopeComponent : public IEntityComponent
{
	DECLARE_COMPONENT("EntityRopeComponent", 0x7BDA9235A6784030, 0x8A46ED8E08FA8537)

	virtual struct IRopeRenderNode* GetRopeRenderNode() = 0;
};

namespace DRS
{
	struct IResponseActor;
	struct IVariableCollection;
	typedef std::shared_ptr<IVariableCollection> IVariableCollectionSharedPtr;
}

//! Proxy for dynamic response system actors.
struct IEntityDynamicResponseComponent : public IEntityComponent
{
	DECLARE_COMPONENT("EntityDynamicResponseComponent", 0xE9250DD3A69F482C, 0xAAFF080D50E0F8B5)

	virtual DRS::IResponseActor*      GetResponseActor() const = 0;
	virtual DRS::IVariableCollection* GetLocalVariableCollection() const = 0;
};