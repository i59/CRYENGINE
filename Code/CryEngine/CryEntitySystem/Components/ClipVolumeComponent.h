// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#ifndef __CLIPVOLUMEPROXY_H__
#define __CLIPVOLUMEPROXY_H__

#pragma once

#include "EntitySystem.h"
#include <CryEntitySystem/IEntityClass.h>
#include <CryNetwork/ISerialize.h>
#include <CryRenderer/IRenderMesh.h>

//////////////////////////////////////////////////////////////////////////
// Description:
//    Proxy for storage of entity attributes.
//////////////////////////////////////////////////////////////////////////
class CClipVolumeComponent : public IClipVolumeComponent
{
public:
	CClipVolumeComponent();
	virtual ~CClipVolumeComponent();

	// IEntityComponent
	virtual void Initialize(IEntity &entity) override;
	virtual void Reload(SEntitySpawnParams& params, XmlNodeRef entityNode) override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	virtual void Serialize(TSerialize ser) override {}
	virtual void SerializeXML(XmlNodeRef& entityNode, bool bLoading, bool bFromInit) override;

	virtual void GetMemoryUsage(ICrySizer* pSizer) const override;
	// ~IEntityComponent

	virtual void         UpdateRenderMesh(IRenderMesh* pRenderMesh, const DynArray<Vec3>& meshFaces) override;
	virtual IClipVolume* GetClipVolume() const override { return m_pClipVolume; }
	virtual IBSPTree3D*  GetBspTree() const override    { return m_pBspTree; }
	virtual void         SetProperties(bool bIgnoresOutdoorAO) override;

private:
	bool LoadFromFile(const char* szFilePath);

	void Reset();

private:
	// Engine clip volume
	IClipVolume* m_pClipVolume;

	// Render Mesh
	_smart_ptr<IRenderMesh> m_pRenderMesh;

	// BSP tree
	IBSPTree3D* m_pBspTree;

	// In-game stat obj
	string m_GeometryFileName;

	// Clip volume flags
	uint32 m_nFlags;
};

DECLARE_SHARED_POINTERS(CClipVolumeComponent)

#endif //__CLIPVOLUMEPROXY_H__
