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

	// Make sure we get logged collision events
	pGameObject->EnablePhysicsEvent(true, eEPE_OnCollisionLogged);

	const int requiredEvents[] = { eGFE_OnCollision };
	pGameObject->UnRegisterExtForEvents(this, NULL, 0);
	pGameObject->RegisterExtForEvents(this, requiredEvents, sizeof(requiredEvents) / sizeof(int));
}

void CPlayerMovement::Physicalize()
{
	SEntityPhysicalizeParams physParams;
	physParams.type = PE_LIVING;

	physParams.mass = 90;

	pe_player_dimensions playerDimensions;
	playerDimensions.heightPivot = 0.f;
	playerDimensions.heightEye = 0.f;
	playerDimensions.sizeCollider = Vec3(0.45f, 0.45f, 2.f);
	playerDimensions.heightCollider = 0.f;
	playerDimensions.bUseCapsule = 1;
	
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

	Matrix34 entityTM = entity.GetWorldTM();

	GetLatestPhysicsStats(*pPhysicalEntity);

	UpdateMovementRequest(ctx.fFrameTime, *pPhysicalEntity, entityTM);

	// Update entity rotation
	Ang3 ypr = CCamera::CreateAnglesYPR(Matrix33(entityTM));
	
	// Use look orientation from view
	ypr.x = CCamera::CreateAnglesYPR(Matrix33(m_pPlayer->GetInput()->GetLookOrientation())).x;

	entityTM.SetRotation33(CCamera::CreateOrientationYPR(ypr));
	
	entity.SetWorldTM(entityTM);
}

void CPlayerMovement::HandleEvent(const SGameObjectEvent &event)
{
	if (event.event == eGFE_OnCollision && !m_pPlayer->IsDead())
	{
		EventPhysCollision *physCollision = reinterpret_cast<EventPhysCollision *>(event.ptr);
		Vec3 impactVelocity = physCollision->vloc[1] - physCollision->vloc[0];

		float hitEnergy = max(fabs_tpl(physCollision->vloc[0].x) + fabs_tpl(physCollision->vloc[0].y) + fabs_tpl(physCollision->vloc[0].z),
			                          physCollision->vloc[0].len2()) * physCollision->mass[0];

		const float maxHitEnergy = 2000.f;

		if(hitEnergy > maxHitEnergy)
		{
			// Kill the player
			m_pPlayer->SetHealth(0.f);
		}
	}
}

void CPlayerMovement::GetLatestPhysicsStats(IPhysicalEntity &physicalEntity)
{
	pe_status_living livingStatus;
	if(physicalEntity.GetStatus(&livingStatus) != 0)
	{
		m_bOnGround = !livingStatus.bFlying;
	}
}

void CPlayerMovement::UpdateMovementRequest(float frameTime, IPhysicalEntity &physicalEntity, const Matrix34 &entityTM)
{
	if(m_bOnGround)
	{
		pe_action_move moveAction;
		moveAction.iJump = 2;

		uint32 inputFlags = m_pPlayer->GetInput()->GetInputFlags();

		moveAction.dir = ZERO;

		const float moveSpeed = 20.5f;

		if(inputFlags & CPlayerInput::eInputFlag_MoveLeft)
		{
			moveAction.dir.x -= moveSpeed;
		}
		if(inputFlags & CPlayerInput::eInputFlag_MoveRight)
		{
			moveAction.dir.x += moveSpeed;
		}
		if(inputFlags & CPlayerInput::eInputFlag_MoveForward)
		{
			moveAction.dir.y += moveSpeed;
		}
		if(inputFlags & CPlayerInput::eInputFlag_MoveBack)
		{
			moveAction.dir.y -= moveSpeed;
		}	
	
		moveAction.dir = entityTM.TransformVector(moveAction.dir) * frameTime;

		if(inputFlags & CPlayerInput::eInputFlag_Jump)
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