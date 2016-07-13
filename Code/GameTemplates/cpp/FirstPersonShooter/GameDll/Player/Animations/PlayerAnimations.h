#pragma once

#include "Entities/Helpers/ISimpleExtension.h"

class CPlayer;

class IActionController;
struct SAnimationContext;
class IAnimationDatabase;
struct SControllerDef;
class IAction;

////////////////////////////////////////////////////////
// Player extension to manage animation handling and playback via Mannequin
////////////////////////////////////////////////////////
class CPlayerAnimations
	: public CGameObjectExtensionHelper<CPlayerAnimations, ISimpleExtension>
{
public:
	CPlayerAnimations();
	virtual ~CPlayerAnimations();

	// ISimpleExtension
	virtual bool Init(IGameObject* pGameObject) override;
	virtual void PostInit(IGameObject* pGameObject) override;

	virtual void Update(SEntityUpdateContext& ctx, int updateSlot) override;

	virtual void ProcessEvent(SEntityEvent& event) override;

	virtual void Release() override;
	// ~ISimpleExtension

	void OnPlayerModelChanged();

protected:
	void ActivateMannequinContext(const char *contextName, const SControllerDef &controllerDefinition, const IAnimationDatabase &animationDatabase);

protected:
	CPlayer *m_pPlayer;

	IActionController *m_pActionController;
	SAnimationContext *m_pAnimationContext;

	_smart_ptr<IAction> m_pIdleFragment;
};
