#pragma once

#include "Player/ISimpleActor.h"

#include <CryMath/Cry_Camera.h>

class CPlayerInput;
class CPlayerMovement;
class CPlayerView;

class CPlayer;

class CSpawnPoint;

////////////////////////////////////////////////////////
// Represents a player participating in gameplay
////////////////////////////////////////////////////////
class CPlayer 
	: public CGameObjectExtensionHelper<CPlayer, ISimpleActor>
{
	enum EFlownodeOutputs
	{
		eOutputPort_OnRespawn = 0,
		eOutputPort_OnDeath,
	};

public:
	enum EGeometrySlots
	{
		eGeometry_ThirdPerson = 0,
		eGeometry_FirstPerson
	};

	struct SExternalCVars
	{
		float m_rotationSpeedYaw;
		float m_rotationSpeedPitch;

		float m_rotationLimitsMinPitch;
		float m_rotationLimitsMaxPitch;

		float m_playerEyeHeight;

		ICVar *m_pFirstPersonGeometry;
		ICVar *m_pCameraJointName;
	};

public:
	CPlayer();
	virtual ~CPlayer();

	// ISimpleActor
	virtual bool Init(IGameObject* pGameObject) override;
	virtual void PostInit(IGameObject* pGameObject) override;

	virtual void HandleEvent(const SGameObjectEvent &event) override;
	virtual void ProcessEvent(SEntityEvent& event) override;

	virtual void Release() override;

	virtual void SetHealth(float health) override;
	virtual float GetHealth() const override { return m_bAlive ? GetMaxHealth() : 0.f; }
	// ~ISimpleActor

	void OnSpawn(EntityId spawnerId);
	
	// Remove player from active play
	void Despawn();

	CPlayerInput *GetInput() const { return m_pInput; }
	CPlayerMovement *GetMovement() const { return m_pMovement; }

	const bool IsLocalClient() const { return m_bIsLocalClient; }

	const SExternalCVars &GetCVars() const;

protected:
	void SetPlayerModel();

protected:
	CPlayerInput *m_pInput;
	CPlayerMovement *m_pMovement;
	CPlayerView *m_pView;

	bool m_bAlive;
	bool m_bIsLocalClient;
};
