// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "Player.h"

#include "extensions/InputExtension.h"
#include "extensions/MovementExtension.h"
#include "extensions/ViewExtension.h"

CPlayer::CPlayer()
{
}

CPlayer::~CPlayer()
{
}

void CPlayer::Initialize(IEntity &entity)
{
	IEntityComponent::Initialize(entity);

	auto &gameObject = GetEntity()->AcquireExternalComponent<IGameObject>();
	gameObject.BindToNetwork();

	GetEntity()->AcquireComponent<CViewExtension>();
	GetEntity()->AcquireComponent<CMovementExtension>();
	GetEntity()->AcquireComponent<CInputExtension>();
}