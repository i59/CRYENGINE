// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:   IEntityProxy.h
//  Version:     v1.00
//  Created:     28/9/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: Definition of all proxy interfaces used by an Entity.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __IEntityProxy_h__
#define __IEntityProxy_h__
#pragma once

#include <CryNetwork/SerializeFwd.h>
#include <CryAudio/IAudioSystem.h>

#include <CryEntitySystem/IComponent.h>
#include <CryAction/ILipSyncProvider.h>

#include <CryCore/BitMask.h>

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

//! Entity proxies that can be hosted by the entity.
enum EEntityProxy
{
	ENTITY_PROXY_USER,

	//! Always the last entry of the enum.
	ENTITY_PROXY_LAST
};

//! Base interface to access to various entity proxy objects.
struct IEntityProxy : public IComponent
{
	// <interfuscator:shuffle>
	virtual ~IEntityProxy(){}

	virtual ComponentEventPriority GetEventPriority(const int eventID) const { return ENTITY_PROXY_LAST - const_cast<IEntityProxy*>(this)->GetType(); }

	virtual void                   GetMemoryUsage(ICrySizer* pSizer) const   {};

	virtual EEntityProxy           GetType() = 0;

	//! Called when the subsystem initialize.
	virtual bool Init(IEntity* pEntity, SEntitySpawnParams& params) = 0;

	//! Called when the subsystem is reloaded.
	virtual void Reload(IEntity* pEntity, SEntitySpawnParams& params) = 0;

	//! Called when the entity is destroyed.
	//! At this point, all proxies are valid.
	//! \note No memory should be deleted here!
	virtual void Done() = 0;

	//! When host entity is destroyed every proxy will be called with the Release method to delete itself.
	virtual void Release() = 0;

	//! Update will be called every time the host Entity is updated.
	//! \param ctx Update context of the host entity, provides all the information needed to update the entity.
	virtual void Update(SEntityUpdateContext& ctx) = 0;

	// By overriding this function proxy will be able to handle events sent from the host Entity.
	// \param event Event structure, contains event id and parameters.
	//virtual	void ProcessEvent( SEntityEvent &event ) = 0;

	//////////////////////////////////////////////////////////////////////////
	//! Serialize proxy to/from XML.
	virtual void SerializeXML(XmlNodeRef& entityNode, bool bLoading) = 0;

	//! Serialize proxy with a TSerialize
	//! \param ser The object wioth which to serialize.
	virtual void Serialize(TSerialize ser) = 0;

	//! Returns true if this proxy need to be saved during serialization.
	//! \return true If proxy needs to be serialized
	virtual bool NeedSerialize() = 0;

	//! Builds a signature to describe the dynamic hierarchy of the parent Entity container.
	//! It is the responsibility of the proxy to identify its internal state which may complicate the hierarchy
	//! of the parent Entity i.e., sub-proxies and which actually exist for this instantiation.
	//! \param ser the object to serialize with, forming the signature.
	//! \return true If the signature is thus far valid
	virtual bool GetSignature(TSerialize signature) = 0;
	// </interfuscator:shuffle>
};

DECLARE_COMPONENT_POINTERS(IEntityProxy);

#endif // __IEntityProxy_h__
