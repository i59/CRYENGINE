// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#ifndef __BREAK_REP_GAME_OBJECT__H__
#define __BREAK_REP_GAME_OBJECT__H__

#include "ActionGame.h"

#include <CryEntitySystem/INetworkedEntityComponent.h>

/*
   =====================================================================
   class: CBreakRepGameObject
   =====================================================================
 */
class CBreakRepGameObject : public CNetworkedEntityComponent<IEntityComponent>
{
public:
	DECLARE_COMPONENT("BreakRepGameObject", 0xB00AA92D13BE4B86, 0x9B2586787383B0C3)

	// IEntityComponent
	virtual void PostInitialize() override
	{
		m_removed = false;
		m_wasRemoved = false;
	}

	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) override
	{
		ser.Value("removed", m_removed);

		if (m_removed && m_wasRemoved == false)
		{
			m_wasRemoved = true;
			CActionGame::Get()->FreeBrokenMeshesForEntity(GetEntity()->GetPhysics());
		}
		return true;
	}
	// ~IEntityComponent

public:
	bool m_wasRemoved;
	bool m_removed;
};

#endif
