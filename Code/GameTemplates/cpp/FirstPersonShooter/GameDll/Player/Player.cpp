#include "StdAfx.h"
#include "Player.h"

#include "Movement/PlayerMovement.h"
#include "Input/PlayerInput.h"
#include "View/PlayerView.h"

#include "Game/GameFactory.h"
#include "Game/GameRules.h"

#include "FlowNodes/Helpers/FlowGameEntityNode.h"

#include "Entities/Gameplay/SpawnPoint.h"

#include <CryRenderer/IRenderAuxGeom.h>

class CPlayerRegistrator
	: public IEntityRegistrator
	, public CPlayer::SExternalCVars
{
	virtual void Register() override
	{
		CGameFactory::RegisterGameObject<CPlayer>("Player", "", 0);

		CGameFactory::RegisterGameObjectExtension<CPlayerMovement>("PlayerMovement");
		CGameFactory::RegisterGameObjectExtension<CPlayerInput>("PlayerInput");
		CGameFactory::RegisterGameObjectExtension<CPlayerView>("PlayerView");
		
		// Create flownode
		CGameEntityNodeFactory &nodeFactory = CGameFactory::RegisterEntityFlowNode("Player");

		// Define output ports
		std::vector<SOutputPortConfig> outputs;
		outputs.push_back(OutputPortConfig_Void("OnRespawn", "Sent when the player is respawned"));
		outputs.push_back(OutputPortConfig_Void("OnDeath", "Sent when the player is killed"));
		nodeFactory.AddOutputs(outputs);

		// Mark the factory as complete, indicating that there will be no additional ports
		nodeFactory.Close();

		RegisterCVars();
	}

	void RegisterCVars()
	{
		REGISTER_CVAR2("pl_rotationSpeedYaw", &m_rotationSpeedYaw, 0.05f, VF_CHEAT, "Speed at which the player rotates entity yaw");
		REGISTER_CVAR2("pl_rotationSpeedPitch", &m_rotationSpeedPitch, 0.05f, VF_CHEAT, "Speed at which the player rotates entity pitch");

		REGISTER_CVAR2("pl_rotationLimitsMinPitch", &m_rotationLimitsMinPitch, -1.f, VF_CHEAT, "Minimum entity pitch limit");
		REGISTER_CVAR2("pl_rotationLimitsMaxPitch", &m_rotationLimitsMaxPitch, 1.5f, VF_CHEAT, "Maximum entity pitch limit");

		REGISTER_CVAR2("pl_eyeHeight", &m_playerEyeHeight, 0.935f, VF_CHEAT, "Height of the player's eyes from ground");
	}
};

CPlayerRegistrator g_playerRegistrator;

CPlayer::CPlayer()
	: m_pInput(nullptr)
	, m_pMovement(nullptr)
	, m_pView(nullptr)
	, m_bAlive(false)
	, m_bIsLocalClient(false)
{
}

CPlayer::~CPlayer()
{
	gEnv->pGame->GetIGameFramework()->GetIActorSystem()->RemoveActor(GetEntityId());
}

const CPlayer::SExternalCVars &CPlayer::GetCVars() const
{
	return g_playerRegistrator;
}

bool CPlayer::Init(IGameObject *pGameObject)
{
	SetGameObject(pGameObject);

	return pGameObject->BindToNetwork();
}

void CPlayer::PostInit(IGameObject *pGameObject)
{
	const int requiredEvents[] = { eGFE_BecomeLocalPlayer };
	pGameObject->RegisterExtForEvents(this, requiredEvents, sizeof(requiredEvents) / sizeof(int));

	m_pMovement = static_cast<CPlayerMovement *>(GetGameObject()->AcquireExtension("PlayerMovement"));
	m_pInput = static_cast<CPlayerInput *>(GetGameObject()->AcquireExtension("PlayerInput"));

	m_pView = static_cast<CPlayerView *>(GetGameObject()->AcquireExtension("PlayerView"));

	// Register with the actor system
	gEnv->pGame->GetIGameFramework()->GetIActorSystem()->AddActor(GetEntityId(), this);
}

void CPlayer::HandleEvent(const SGameObjectEvent &event)
{
	if (event.event == eGFE_BecomeLocalPlayer)
	{
		m_bIsLocalClient = true;
	}
}

void CPlayer::ProcessEvent(SEntityEvent& event)
{
	switch (event.event)
	{
		case ENTITY_EVENT_RESET:
		{
			// Make sure to revive player when respawning in Editor
			if (event.nParam[0] == 1)
			{
				m_bAlive = false;

				SetHealth(GetMaxHealth());
			}
			else
			{
				// Trigger despawning
				SetHealth(0.f);
			}
		}
		break;
	}
}

void CPlayer::Release()
{
	ISimpleActor::Release();
}

void CPlayer::SetHealth(float health)
{
	bool bWasAlive = m_bAlive;
	m_bAlive = health > 0;

	if(m_bAlive && !bWasAlive)
	{
		TFlowInputData inputData;
		SEntityEvent evnt;

		evnt.event     = ENTITY_EVENT_ACTIVATE_FLOW_NODE_OUTPUT;
		evnt.nParam[0] = eOutputPort_OnRespawn;

		evnt.nParam[1] = (INT_PTR)&inputData;
		GetEntity()->SendEvent(evnt);

		SetPlayerModel();

		m_pInput->OnPlayerRespawn();
	}
	else if(!m_bAlive && bWasAlive)
	{
		TFlowInputData inputData;
		SEntityEvent evnt;

		evnt.event     = ENTITY_EVENT_ACTIVATE_FLOW_NODE_OUTPUT;
		evnt.nParam[0] = eOutputPort_OnDeath;

		evnt.nParam[1] = (INT_PTR)&inputData;
		GetEntity()->SendEvent(evnt);

		m_pMovement->Ragdollize();
	}
}

void CPlayer::OnSpawn(EntityId spawnerId)
{
	SetHealth(GetMaxHealth());
}

void CPlayer::SetPlayerModel()
{
	// Load the third person model
	GetEntity()->LoadGeometry(eGeometry_ThirdPerson, "Objects/primitive_sphere.cgf");
	
	// Disable player model rendering for the local client
	if (IsLocalClient())
	{
		GetEntity()->SetSlotFlags(eGeometry_ThirdPerson, GetEntity()->GetSlotFlags(0) & ~ENTITY_SLOT_RENDER);

		// Load the first person character
		GetEntity()->LoadGeometry(eGeometry_FirstPerson, "Objects/primitive_pyramid.cgf");
		Matrix34 tm = Matrix34::Create(Vec3(1, 1, 1), IDENTITY, Vec3(0, 0, 1));

		GetEntity()->SetSlotLocalTM(eGeometry_FirstPerson, tm);
	}

	m_pMovement->Physicalize();
}

void CPlayer::Despawn()
{
	m_pMovement->Dephysicalize();
}