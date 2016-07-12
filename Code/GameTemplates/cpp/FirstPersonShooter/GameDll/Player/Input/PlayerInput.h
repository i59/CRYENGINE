#pragma once

#include "Entities/Helpers/ISimpleExtension.h"

class CPlayer;

////////////////////////////////////////////////////////
// Player extension to manage input
////////////////////////////////////////////////////////
class CPlayerInput 
	: public CGameObjectExtensionHelper<CPlayerInput, ISimpleExtension>
	, public IActionListener
{
	enum EInputFlagType
	{
		eInputFlagType_Hold = 0,
		eInputFlagType_Toggle
	};

public:
	typedef uint8 TInputFlags;

	enum EInputFlags
		: TInputFlags
	{
		eInputFlag_MoveLeft = 1 << 0,
		eInputFlag_MoveRight = 1 << 1,
		eInputFlag_MoveForward = 1 << 2,
		eInputFlag_MoveBack = 1 << 3,

		eInputFlag_Jump = 1 << 4,

#ifdef TEMPLATE_DEBUG
		eInputFlag_DetachCamera = 1 << 5,
#endif
	};

public:
	CPlayerInput();
	virtual ~CPlayerInput();

	// ISimpleExtension
	virtual bool Init(IGameObject* pGameObject) override;
	virtual void PostInit(IGameObject* pGameObject) override;
	virtual void Release() override;

	virtual void HandleEvent(const SGameObjectEvent &event) override;

	virtual void Update(SEntityUpdateContext &ctx, int updateSlot) override;
	// ~ISimpleExtension

	// IActionListener
	virtual void OnAction(const ActionId &action, int activationMode, float value) override;
	// ~IActionListener

	void OnPlayerRespawn();

	const TInputFlags GetInputFlags() const { return m_inputFlags; }
	const Vec2 GetMouseDeltaRotation() const { return m_mouseDeltaRotation; }

	const Quat &GetLookOrientation() const { return m_lookOrientation; }

protected:
	void HandleInputFlagChange(EInputFlags flags, int activationMode, EInputFlagType type = eInputFlagType_Hold);

protected:
	CPlayer *m_pPlayer;

	TInputFlags m_inputFlags;

	Vec2 m_mouseDeltaRotation;

	// Should translate to head orientation in the future
	Quat m_lookOrientation;
};
