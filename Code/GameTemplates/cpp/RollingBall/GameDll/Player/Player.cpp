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
		CGameFactory::RegisterGameObject<CPlayer>("Player");

		CGameFactory::RegisterGameObjectExtension<CPlayerMovement>("PlayerMovement");
		CGameFactory::RegisterGameObjectExtension<CPlayerInput>("PlayerInput");
		CGameFactory::RegisterGameObjectExtension<CPlayerView>("PlayerView");
		
		RegisterCVars();
	}

	void RegisterCVars()
	{
		REGISTER_CVAR2("pl_mass", &m_mass, 90.f, VF_CHEAT, "Mass of the player entity in kg");
		REGISTER_CVAR2("pl_moveImpulseStrength", &m_moveImpulseStrength, 800.f, VF_CHEAT, "Strength of the per-frame impulse when holding inputs");

		REGISTER_CVAR2("pl_rotationSpeedYaw", &m_rotationSpeedYaw, 0.05f, VF_CHEAT, "Speed at which the player rotates entity yaw");
		REGISTER_CVAR2("pl_rotationSpeedPitch", &m_rotationSpeedPitch, 0.05f, VF_CHEAT, "Speed at which the player rotates entity pitch");

		REGISTER_CVAR2("pl_rotationLimitsMinPitch", &m_rotationLimitsMinPitch, -0.84f, VF_CHEAT, "Minimum entity pitch limit");
		REGISTER_CVAR2("pl_rotationLimitsMaxPitch", &m_rotationLimitsMaxPitch, 1.5f, VF_CHEAT, "Maximum entity pitch limit");

		REGISTER_CVAR2("pl_viewDistance", &m_viewDistance, 10.f, VF_CHEAT, "Determiens the distance between player and camera");
		
		m_pGeometry = REGISTER_STRING("pl_geometry", "Objects/Default/primitive_sphere.cgf", VF_CHEAT, "Sets the third person geometry to load");
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
			if (event.nParam[0] == 1)
			{
				// Make sure to revive player when respawning in Editor
				SetHealth(GetMaxHealth());
			}
		}
		break;
	}
}

void CPlayer::SetHealth(float health)
{
	// Find a spawn point and move the entity there
	SelectSpawnPoint();

	// Note that this implementation does not handle the concept of death, SetHealth(0) will still revive the player.
	if (m_bAlive)
		return;

	m_bAlive = true;

	// Unhide the entity in case hidden by the Editor
	GetEntity()->Hide(false);

	// Set the player geometry, this also triggers physics proxy creation
	SetPlayerModel();

	// Notify input that the player respawned
	m_pInput->OnPlayerRespawn();
}

void CPlayer::SelectSpawnPoint()
{
	// We only handle default spawning below for the Launcher
	// Editor has special logic in CEditorGame
	if (gEnv->IsEditor())
		return;

	// Spawn at first default spawner
	auto *pEntityIterator = gEnv->pEntitySystem->GetEntityIterator();
	pEntityIterator->MoveFirst();

	auto *pSpawnerClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("SpawnPoint");
	auto extensionId = gEnv->pGame->GetIGameFramework()->GetIGameObjectSystem()->GetID("SpawnPoint");

	while (!pEntityIterator->IsEnd())
	{
		IEntity *pEntity = pEntityIterator->Next();

		if (pEntity->GetClass() != pSpawnerClass)
			continue;

		auto *pGameObject = gEnv->pGame->GetIGameFramework()->GetGameObject(pEntity->GetId());
		if (pGameObject == nullptr)
			continue;

		auto *pSpawner = static_cast<CSpawnPoint *>(pGameObject->QueryExtension(extensionId));
		if (pSpawner == nullptr)
			continue;

		pSpawner->SpawnEntity(*GetEntity());

		break;
	}
}

void CPlayer::SetPlayerModel()
{
	// Load the third person model
	GetEntity()->LoadGeometry(eGeometry_ThirdPerson, GetCVars().m_pGeometry->GetString());
	
	// Override material so that we can see the ball rolling more easily
	GetEntity()->SetMaterial(gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("EngineAssets/TextureMsg/DefaultSolids"));

	// Now create the physical representation of the entity
	m_pMovement->Physicalize();
}