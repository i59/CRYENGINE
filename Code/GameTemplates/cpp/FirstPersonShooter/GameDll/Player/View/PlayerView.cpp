#include "StdAfx.h"
#include "PlayerView.h"

#include "Player/Player.h"

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

	viewParams.rotation = Quat(m_pPlayer->GetInput()->GetLookOrientation());
	viewParams.position = entity.GetWorldPos();

	// Offset camera up two units
	viewParams.position += entity.GetWorldRotation().GetColumn2() * m_pPlayer->GetCVars().m_playerEyeHeight;
}

void CPlayerView::Release()
{
	ISimpleExtension::Release();
}