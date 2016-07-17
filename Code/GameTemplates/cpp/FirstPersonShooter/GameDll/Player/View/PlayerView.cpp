#include "StdAfx.h"
#include "PlayerView.h"

#include "Player/Player.h"
#include "Player/Input/PlayerInput.h"
#include "Player/Movement/PlayerMovement.h"

#include <IViewSystem.h>
#include <CryAnimation/ICryAnimation.h>

CPlayerView::CPlayerView()
	: m_cameraJointId(0)
{
}

CPlayerView::~CPlayerView()
{
	GetGameObject()->ReleaseView(this);
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

void CPlayerView::OnPlayerModelChanged()
{
	if (auto *pCharacter = GetEntity()->GetCharacter(CPlayer::eGeometry_FirstPerson))
	{
		// Cache the camera joint id so that we don't need to look it up every frame in UpdateView
		const char *cameraJointName = m_pPlayer->GetCVars().m_pCameraJointName->GetString();

		m_cameraJointId = pCharacter->GetIDefaultSkeleton().GetJointIDByName(cameraJointName);
	}
}

void CPlayerView::UpdateView(SViewParams &viewParams)
{
	IEntity &entity = *GetEntity();

	// Start with changing view rotation to the requested mouse look orientation
	viewParams.rotation = Quat(m_pPlayer->GetInput()->GetLookOrientation());

	if (auto *pCharacter = entity.GetCharacter(CPlayer::eGeometry_FirstPerson))
	{
		const QuatT &cameraOrientation = pCharacter->GetISkeletonPose()->GetAbsJointByID(m_cameraJointId);

		viewParams.position = entity.GetWorldTM().TransformPoint(cameraOrientation.t);
	}
	else
	{
		// Fallback in case the player character failed to load
		// Should not happen in production
		viewParams.position = entity.GetWorldPos();

		// The player's origin point is at its feet
		// Offset camera upwards to meet its eyes
		viewParams.position += entity.GetWorldRotation().GetColumn2() * m_pPlayer->GetCVars().m_playerEyeHeight;
	}
}