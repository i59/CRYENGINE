#pragma once

#include "ISimpleWeapon.h"

////////////////////////////////////////////////////////
// Rifle weapon entity that shoots entities as bullets
////////////////////////////////////////////////////////
class CRifle 
	: public CGameObjectExtensionHelper<CRifle, ISimpleWeapon>
{
public:
	// ISimpleExtension	
	virtual void PostInit(IGameObject *pGameObject) override;
	
	virtual void Release() override;
	// ~ISimpleExtension

	// IWeapon
	virtual void RequestFire() override;
	// ~IWeapon

protected:
	void LoadGeometry();
};