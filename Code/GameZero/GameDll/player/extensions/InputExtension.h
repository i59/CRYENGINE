// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <IActionMapManager.h>

#include <CryAction/IGameObject.h>

class CInputExtension 
	: public IEntityComponent
	, IActionListener
	, ISystemEventListener
{
public:
	DECLARE_COMPONENT("Input", 0x170A915CF2BC42E6, 0x8C04F77DC0AF97D6)

	// IEntityComponent
	virtual void PostInitialize() override;

	virtual bool NeedSerialize() { return true; }
	// ~IEntityComponent

	// IActionListener
	virtual void OnAction(const ActionId& action, int activationMode, float value) override;
	// ~IActionListener

	// ISystemEventListener
	void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
	// ~ISystemEventListener

	CInputExtension();
	virtual ~CInputExtension();

	Quat GetViewRotation();
	Vec3 GetDeltaMovement() { return m_vDeltaMovement; }
	bool IsBoosting() { return m_bBoost; }

private:
	bool OnActionMoveForward(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionMoveBackward(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionMoveLeft(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionMoveRight(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionMoveUp(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionMoveDown(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionMouseRotateYaw(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionMouseRotatePitch(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionControllerMoveX(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionControllerMoveY(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionControllerRotateYaw(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionControllerRotatePitch(EntityId entityId, const ActionId& actionId, int activationMode, float value);
	bool OnActionBoost(EntityId entityId, const ActionId& actionId, int activationMode, float value);

	void RegisterInputHandler();

	TActionHandler<CInputExtension> m_playerActionHandler;
	Vec3                            m_vDeltaMovement;
	Ang3                            m_rotation;
	float                           m_mouseSensitivity;
	float                           m_controllerSensitivity;
	bool                            m_bUseControllerRotation;
	bool                            m_bBoost;
};
