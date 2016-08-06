// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#ifndef __FLOWTORNADO_H__
#define __FLOWTORNADO_H__

#include "../Game.h"
#include "Tornado.h"
#include "Nodes/G2FlowBaseNode.h"

class CFlowTornadoWander : public CFlowBaseNode<eNCT_Instanced>
{
public:
	CFlowTornadoWander( SActivationInfo * pActInfo )
	{
	}

	IFlowNodePtr Clone( SActivationInfo * pActInfo )
	{
		return new CFlowTornadoWander(pActInfo);
	}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] = {
			InputPortConfig_Void("Activate", _HELP("Tell the tornado to actually wander to the new [Target]")),
				InputPortConfig<EntityId>("Target", _HELP("Set a new wandering target for the tornado")),
			{0}
		};
		static const SOutputPortConfig outputs[] = {
			OutputPortConfig_Void("Done", _HELP("Triggered when target has been reached")),
			{0}
		};
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.sDescription = _HELP("Tells a tornado entity to wander in the direction of the [Target] entity");
		config.SetCategory(EFLN_ADVANCED);
	}

	virtual void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		switch (event)
		{
		case eFE_Initialize:
			m_actInfo = *pActInfo;
			break;
		case eFE_Activate:
			if (IsPortActive(pActInfo, 0) && pActInfo->pEntity)
			{
				CTornado* pTornado = (CTornado*)gEnv->pEntitySystem->QueryComponent<CTornado>(pActInfo->pEntity->GetId());

				if (pTornado)
				{ 
					IEntity* pTarget = gEnv->pEntitySystem->GetEntity( GetPortEntityId(pActInfo, 1) );
					if (pTarget)
					{
						pTornado->SetTarget(pTarget, this);
					}
				}        
			}
			break;
		}
	}

	void Done()
	{
		if (m_actInfo.pGraph)
			ActivateOutput( &m_actInfo, 0, true);
	}
private:
	SActivationInfo m_actInfo;
};

#endif //__FLOWTORNADO_H__
