#include "StdAfx.h"
#include "PlayerMovement.h"

#include "Player/Player.h"
#include "Player/Input/PlayerInput.h"

CPlayerMovement::CPlayerMovement()
	: m_bOnGround(false)
{
}

CPlayerMovement::~CPlayerMovement()
{
}

bool CPlayerMovement::Init(IGameObject *pGameObject)
{
	SetGameObject(pGameObject);

	return pGameObject->BindToNetwork();
}

void CPlayerMovement::PostInit(IGameObject *pGameObject)
{
	m_pPlayer = static_cast<CPlayer *>(pGameObject->QueryExtension("Player"));

	// Make sure that this extension is updated regularly via the Update function below
	pGameObject->EnableUpdateSlot(this, 0);
}

void CPlayerMovement::Physicalize()
{
	SEntityPhysicalizeParams physParams;
	physParams.type = PE_LIVING;

	physParams.mass = 90;

	pe_player_dimensions playerDimensions;
	playerDimensions.sizeCollider = Vec3(0.45f, 0.45f, m_pPlayer->GetCVars().m_playerEyeHeight * 0.5f);
	playerDimensions.heightPivot = 0;
	playerDimensions.heightEye = 0.f;
	playerDimensions.heightCollider = 1.f;
	playerDimensions.bUseCapsule = 0;
	playerDimensions.groundContactEps = 0.004f;

	physParams.pPlayerDimensions = &playerDimensions;
	
	pe_player_dynamics playerDynamics;
	//playerDynamics.kInertia = 0.f;
	//playerDynamics.kInertiaAccel = 0.f;
	playerDynamics.kAirControl = 0.f;
	playerDynamics.mass = physParams.mass;

	physParams.pPlayerDynamics = &playerDynamics;

	GetEntity()->Physicalize(physParams);

	m_bRequestedJump = false;
}

void CPlayerMovement::Ragdollize()
{
	// No character is loaded for player yet, just remove physics
	Dephysicalize();
}

void CPlayerMovement::Dephysicalize()
{
	SEntityPhysicalizeParams physParams;
	physParams.type = PE_NONE;

	GetEntity()->Physicalize(physParams);
}

void CPlayerMovement::Release()
{
	ISimpleExtension::Release();
}

void CPlayerMovement::Update(SEntityUpdateContext &ctx, int updateSlot)
{
	if(m_pPlayer->IsDead())
		return;

	IEntity &entity = *GetEntity();
	IPhysicalEntity *pPhysicalEntity = entity.GetPhysics();
	if(pPhysicalEntity == nullptr)
		return;

	GetLatestPhysicsStats(*pPhysicalEntity);

	UpdateMovementRequest(ctx.fFrameTime, *pPhysicalEntity);
}

void CPlayerMovement::GetLatestPhysicsStats(IPhysicalEntity &physicalEntity)
{
	pe_status_living livingStatus;
	if(physicalEntity.GetStatus(&livingStatus) != 0)
	{
		m_bOnGround = !livingStatus.bFlying;
	}
}

void CPlayerMovement::UpdateMovementRequest(float frameTime, IPhysicalEntity &physicalEntity)
{
	if(m_bOnGround)
	{
		pe_action_move moveAction;
		moveAction.iJump = 2;

		moveAction.dir = ZERO;

		const float moveSpeed = 20.5f;
		moveAction.dir = m_pPlayer->GetInput()->GetLookOrientation() * GetLocalMoveDirection() * moveSpeed * frameTime;

		if(m_pPlayer->GetInput()->GetInputFlags() & CPlayerInput::eInputFlag_Jump)
		{
			m_bRequestedJump = true;
		}
		else if(m_bRequestedJump)
		{
			moveAction.dir.z += 3.5f;

			m_bRequestedJump = false;
		}

		physicalEntity.Action(&moveAction);
	}
}

Vec3 CPlayerMovement::GetLocalMoveDirection() const
{
	Vec3 moveDirection = ZERO;

	uint32 inputFlags = m_pPlayer->GetInput()->GetInputFlags();

	if (inputFlags & CPlayerInput::eInputFlag_MoveLeft)
	{
		moveDirection.x -= 1;
	}
	if (inputFlags & CPlayerInput::eInputFlag_MoveRight)
	{
		moveDirection.x += 1;
	}
	if (inputFlags & CPlayerInput::eInputFlag_MoveForward)
	{
		moveDirection.y += 1;
	}
	if (inputFlags & CPlayerInput::eInputFlag_MoveBack)
	{
		moveDirection.y -= 1;
	}

	return moveDirection;
}