// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

struct SEntityScriptProperties;
class CGameEntityNodeFactory;

#include <CryEntitySystem/IEntityComponent.h>

class CGeomEntity : public IEntityComponent
{
public:
	DECLARE_COMPONENT("GeomEntity", 0xD74C68AEF4FF4124, 0xA820AF368889238B)

	CGeomEntity();
	virtual ~CGeomEntity();

	// IEntityComponent
	virtual void Initialize(IEntity &entity) override;

	virtual void ProcessEvent(const SEntityEvent& event) override;
	// ~IEntityComponent

	static bool RegisterProperties(SEntityScriptProperties& tables, CGameEntityNodeFactory* pNodeFactory);

private:
	static void OnFlowgraphActivation(EntityId entityId, IFlowNode::SActivationInfo* pActInfo, const class CFlowGameEntityNode* pNode);
	void        Reset();

	enum EFlowgraphInputPorts
	{
		eInputPorts_LoadGeometry,
	};

	enum EFlowgraphOutputPorts
	{
		eOutputPorts_Done,
	};

	string m_geometryPath;
};
