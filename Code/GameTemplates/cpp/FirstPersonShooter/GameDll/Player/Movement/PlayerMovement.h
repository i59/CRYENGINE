#pragma once

#include "Entities/Helpers/ISimpleExtension.h"

class CPlayer;

////////////////////////////////////////////////////////
// Player extension to manage movement
////////////////////////////////////////////////////////
class CPlayerMovement : public CGameObjectExtensionHelper<CPlayerMovement, ISimpleExtension>
{
public:
	CPlayerMovement();

	//ISimpleExtension
	virtual bool Init(IGameObject* pGameObject) override;
	virtual void PostInit(IGameObject* pGameObject) override;

	virtual void Release() override;

	virtual void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	//~ISimpleExtension

	void Physicalize();
	void Ragdollize();
	void Dephysicalize();

	// Gets the requested movement direction based on input data
	Vec3 GetLocalMoveDirection() const;

protected:
	// Get the stats from latest physics thread update
	void GetLatestPhysicsStats(IPhysicalEntity &physicalEntity);
	void UpdateMovementRequest(float frameTime, IPhysicalEntity &physicalEntity);

protected:
	CPlayer *m_pPlayer;

	bool m_bOnGround;
};
