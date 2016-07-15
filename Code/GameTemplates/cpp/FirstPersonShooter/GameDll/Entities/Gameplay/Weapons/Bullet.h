#pragma once

#include "Entities/Helpers/NativeEntityBase.h"

////////////////////////////////////////////////////////
// Physicalized bullet shot from weaponry, expires on collision with another object
////////////////////////////////////////////////////////
class CBullet 
	: public CGameObjectExtensionHelper<CBullet, CNativeEntityBase>
{
public:
	CBullet();
	virtual ~CBullet() {}

	// ISimpleExtension	
	virtual void PostInit(IGameObject *pGameObject) override;
	virtual void HandleEvent(const SGameObjectEvent &event) override;

	virtual void Update(SEntityUpdateContext& ctx, int updateSlot) override;

	virtual void Release() override;
	// ~ISimpleExtension

protected:
	void Physicalize();

protected:
	// Time at which this entity should be removed, or negative if not scheduled for removal
	float m_removalTime;
};