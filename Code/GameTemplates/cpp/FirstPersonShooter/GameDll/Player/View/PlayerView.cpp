#include "StdAfx.h"
#include "PlayerView.h"

#include "Player/Player.h"
#include "Player/Input/PlayerInput.h"
#include "Player/Movement/PlayerMovement.h"

#include <IViewSystem.h>

CPlayerView::CPlayerView()
{
}

CPlayerView::~CPlayerView()
{
	GetGameObject()->ReleaseView(this);
}

bool CPlayerView::Init(IGameObject *pGameObject)
{
	SetGameObject(pGameObject);

	return pGameObject->BindToNetwork();
}

void CPlayerView::PostInit(IGameObject *pGameObject)
{
	m_pPlayer = static_cast<CPlayer *>(pGameObject->QueryExtension("Player"));

	const int requiredEvents[] = { eGFE_BecomeLocalPlayer };
	pGameObject->RegisterExtForEvents(this, requiredEvents, sizeof(requiredEvents) / sizeof(int));
}

void CPlayerView::HandleEvent(const SGameObjectEvent &event)
{
	if (event.event == eGFE_BecomeLocalPlayer)
	{
		// Register for UpdateView callbacks
		GetGameObject()->CaptureView(this);
	}
}

void CPlayerView::UpdateView(SViewParams &viewParams)
{
	IEntity &entity = *GetEntity();

	// Start with changing view rotation to the requested mouse look orientation
	viewParams.rotation = Quat(m_pPlayer->GetInput()->GetLookOrientation());

#ifdef TEMPLATE_DEBUG
	auto inputFlags = m_pPlayer->GetInput()->GetInputFlags();

	if((inputFlags & CPlayerInput::eInputFlag_DetachCamera) != 0)
	{
		viewParams.position += viewParams.rotation * m_pPlayer->GetMovement()->GetLocalMoveDirection();

		return;
	}
#endif


	viewParams.position = entity.GetWorldPos();

	// The player's origin point is at its feet
	// Offset camera upwards to meet its eyes
	viewParams.position += entity.GetWorldRotation().GetColumn2() * m_pPlayer->GetCVars().m_playerEyeHeight;
}

void CPlayerView::Release()
{
	ISimpleExtension::Release();
}