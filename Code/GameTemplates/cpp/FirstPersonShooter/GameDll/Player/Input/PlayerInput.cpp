#include "StdAfx.h"
#include "PlayerInput.h"

#include "Player/Player.h"

CPlayerInput::CPlayerInput()
{
}

CPlayerInput::~CPlayerInput()
{
}

bool CPlayerInput::Init(IGameObject *pGameObject)
{
	SetGameObject(pGameObject);

	return pGameObject->BindToNetwork();
}

void CPlayerInput::PostInit(IGameObject *pGameObject)
{
	const int requiredEvents[] = { eGFE_BecomeLocalPlayer };
	pGameObject->UnRegisterExtForEvents(this, NULL, 0);
	pGameObject->RegisterExtForEvents(this, requiredEvents, sizeof(requiredEvents) / sizeof(int));

	m_pPlayer = static_cast<CPlayer *>(pGameObject->QueryExtension("Player"));
}

void CPlayerInput::HandleEvent(const SGameObjectEvent &event)
{
	if (event.event == eGFE_BecomeLocalPlayer)
	{
		IActionMapManager *pActionMapManager = gEnv->pGame->GetIGameFramework()->GetIActionMapManager();

		pActionMapManager->InitActionMaps("Libs/config/defaultprofile.xml");
		pActionMapManager->Enable(true);

		pActionMapManager->EnableActionMap("player", true);

		if(IActionMap *pActionMap = pActionMapManager->GetActionMap("player"))
		{
			pActionMap->SetActionListener(GetEntityId());
		}

		GetGameObject()->CaptureActions(this);

		// Make sure that this extension is updated regularly via the Update function below
		GetGameObject()->EnableUpdateSlot(this, 0);
	}
}

void CPlayerInput::Release()
{
	ISimpleExtension::Release();
}

void CPlayerInput::Update(SEntityUpdateContext &ctx, int updateSlot)
{
	// Start by updating look dir
	Ang3 ypr = CCamera::CreateAnglesYPR(Matrix33(m_lookOrientation));
	
	ypr.x += m_mouseDeltaRotation.x * m_pPlayer->GetCVars().m_rotationSpeedYaw * ctx.fFrameTime;

	// TODO: Perform soft clamp here instead of hard wall, should reduce rot speed in this direction when close to limit.
	ypr.y = CLAMP(ypr.y + m_mouseDeltaRotation.y * m_pPlayer->GetCVars().m_rotationSpeedPitch * ctx.fFrameTime, m_pPlayer->GetCVars().m_rotationLimitsMinPitch, m_pPlayer->GetCVars().m_rotationLimitsMaxPitch);

	ypr.z = 0;

	m_lookOrientation = Quat(CCamera::CreateOrientationYPR(ypr));

	// Reset every frame
	m_mouseDeltaRotation = ZERO;
}

void CPlayerInput::OnPlayerRespawn()
{
	m_inputFlags = 0;
	m_mouseDeltaRotation = ZERO;
	m_lookOrientation = IDENTITY;
}

void CPlayerInput::OnAction(const ActionId &action, int activationMode, float value)
{
	// This function is called when inputs trigger action map events
	// Cross-reference 'action.c_str()' with Libs/config/defaultprofile.xml actions
	if(!strcmp(action.c_str(), "moveleft"))
	{
		HandleInputFlagChange(eInputFlag_MoveLeft, activationMode);
	}
	else if(!strcmp(action.c_str(), "moveright"))
	{
		HandleInputFlagChange(eInputFlag_MoveRight, activationMode);
	}
	else if(!strcmp(action.c_str(), "moveforward"))
	{
		HandleInputFlagChange(eInputFlag_MoveForward, activationMode);
	}
	else if(!strcmp(action.c_str(), "moveback"))
	{
		HandleInputFlagChange(eInputFlag_MoveBack, activationMode);
	}
	else if(!strcmp(action.c_str(), "jump"))
	{
		HandleInputFlagChange(eInputFlag_Jump, activationMode);
	}
	else if(!strcmp(action.c_str(), "mouse_rotateyaw"))
	{
		m_mouseDeltaRotation.x -= value;
	}
	else if(!strcmp(action.c_str(), "mouse_rotatepitch"))
	{
		m_mouseDeltaRotation.y -= value;
	}
}

void CPlayerInput::HandleInputFlagChange(EInputFlags flags, int activationMode)
{
	if(activationMode == eIS_Released)
	{
		m_inputFlags &= ~flags;
	}
	else
	{
		m_inputFlags |= flags;
	}
}