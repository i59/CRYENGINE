#include "StdAfx.h"
#include "PlayerAnimations.h"

#include "Player/Player.h"

#include "IdleAction.h"

#include <CryAnimation/ICryAnimation.h>
#include <ICryMannequin.h>

CPlayerAnimations::CPlayerAnimations()
	: m_pActionController(nullptr)
	, m_pAnimationContext(nullptr)
{
}

CPlayerAnimations::~CPlayerAnimations()
{
	SAFE_RELEASE(m_pActionController);
	SAFE_DELETE(m_pAnimationContext);
}

bool CPlayerAnimations::Init(IGameObject *pGameObject)
{
	SetGameObject(pGameObject);

	return pGameObject->BindToNetwork();
}

void CPlayerAnimations::PostInit(IGameObject *pGameObject)
{
	m_pPlayer = static_cast<CPlayer *>(pGameObject->QueryExtension("Player"));

	// Make sure that this extension is updated regularly via the Update function below
	pGameObject->EnableUpdateSlot(this, 0);
}

void CPlayerAnimations::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	if (m_pActionController != nullptr)
	{
		m_pActionController->Update(ctx.fFrameTime);
	}
}

void CPlayerAnimations::ProcessEvent(SEntityEvent& event)
{
	switch (event.event)
	{
		case ENTITY_EVENT_ANIM_EVENT:
		{
			const AnimEventInstance *pAnimEvent = reinterpret_cast<const AnimEventInstance *>(event.nParam[0]);
			ICharacterInstance *pCharacter = reinterpret_cast<ICharacterInstance *>(event.nParam[1]);

			m_pActionController->OnAnimationEvent(pCharacter, *pAnimEvent);
		}
		break;
	}
}

void CPlayerAnimations::OnPlayerModelChanged()
{
	// Release previous controller and context, if we are respawning
	SAFE_RELEASE(m_pActionController);
	SAFE_DELETE(m_pAnimationContext);

	// Now start loading the Mannequin data
	IMannequin &mannequinInterface = gEnv->pGame->GetIGameFramework()->GetMannequinInterface();
	IAnimationDatabaseManager &animationDatabaseManager = mannequinInterface.GetAnimationDatabaseManager();

	const char *mannequinControllerDefinition = m_pPlayer->GetCVars().m_pFirstPersonControllerDefinition->GetString();

	// Load the Mannequin controller definition.
	// This is owned by the animation database manager
	const SControllerDef *pControllerDefinition = animationDatabaseManager.LoadControllerDef(mannequinControllerDefinition);
	if (pControllerDefinition == nullptr)
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed to load controller definition.");
		return;
	}

	const char *mannequinContextName = m_pPlayer->GetCVars().m_pFirstPersonMannequinContext->GetString();
	const char *animationDatabasePath = m_pPlayer->GetCVars().m_pFirstPersonAnimationDatabase->GetString();

	// Load the animation database
	auto *pAnimationDatabase = animationDatabaseManager.Load(animationDatabasePath);
	if (pAnimationDatabase == nullptr)
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed to load animation database %s!", animationDatabasePath);
		return;
	}

	// Create a new animation context for the controller definition we loaded above
	m_pAnimationContext = new SAnimationContext(*pControllerDefinition);

	// Now create the controller that will be handling animation playback
	m_pActionController = mannequinInterface.CreateActionController(GetEntity(), *m_pAnimationContext);
	CRY_ASSERT(m_pActionController != nullptr);

	// Activate the Main context we'll be playing our animations in
	ActivateMannequinContext(mannequinContextName, *pControllerDefinition, *pAnimationDatabase);

	// Create this idle fragment
	// This implementation handles switching to various sub-fragments by itself, based on input and physics data
	int priority = 0;
	auto idleFragmentId = pControllerDefinition->m_fragmentIDs.Find("Idle");

	m_pIdleFragment = new CIdleAction(priority, idleFragmentId, TAG_STATE_EMPTY, IAction::Interruptable);

	// Queue the idle fragment to start playing immediately on next update
	m_pActionController->Queue(*m_pIdleFragment.get());
}

void CPlayerAnimations::ActivateMannequinContext(const char *contextName, const SControllerDef &controllerDefinition, const IAnimationDatabase &animationDatabase)
{
	IEntity &entity = *GetEntity();

	const TagID scopeContextId = controllerDefinition.m_scopeContexts.Find(contextName);
	if (scopeContextId == TAG_ID_INVALID)
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed to find %s scope context id in controller definition.", contextName);
		return;
	}

	ICharacterInstance *pCharacterInstance = entity.GetCharacter(CPlayer::eGeometry_FirstPerson);
	CRY_ASSERT(pCharacterInstance != nullptr);

	// Setting Scope contexts can happen at any time, and what entity or character instance we have bound to a particular scope context
	// can change during the lifetime of an action controller.
	m_pActionController->SetScopeContext(scopeContextId, entity, pCharacterInstance, &animationDatabase);
}

void CPlayerAnimations::Release()
{
	ISimpleExtension::Release();
}