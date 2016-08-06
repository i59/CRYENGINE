// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: Grenade that delivers multiple EMP pulses over a given time period

*************************************************************************/
#ifndef __EMP_GRENADE_H__
#define __EMP_GRENADE_H__

#include "Grenade.h"


class CEMPGrenade : public CGrenade
{
private:
	typedef CGrenade BaseClass;

public:
	DECLARE_COMPONENT("EMPGrenade", 0x0B944D89A67C4BD9, 0x89BC9D8840035EC0)

	CEMPGrenade();
	virtual ~CEMPGrenade();

	virtual void Update( SEntityUpdateContext &ctx);
	virtual void HandleEvent(const SGameObjectEvent &event);

private:
	
	Vec3 m_pulsePos;

	float m_postExplosionLifetime;
	bool	m_bActive;
};

#endif //__EMP_GRENADE_H__
