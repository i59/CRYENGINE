#include "StdAfx.h"
#include "Rifle.h"

#include "Game/GameFactory.h"

class CRifleRegistrator
	: public IEntityRegistrator
{
	virtual void Register() override
	{
		CGameFactory::RegisterGameObject<CRifle>("Rifle", "", 0);

		RegisterCVars();
	}

	void RegisterCVars()
	{
		m_pGeometryPath = REGISTER_STRING("w_rifleGeometryPath", "Objects/Weapons/Rifle/rifle_tp.cgf", VF_CHEAT, "The name of the FP context used in Mannequin");
	}

public:
	ICVar *m_pGeometryPath;
};

CRifleRegistrator g_rifleRegistrator;

void CRifle::PostInit(IGameObject *pGameObject)
{
	LoadGeometry();
}

void CRifle::LoadGeometry()
{
	const char *geometryPath = g_rifleRegistrator.m_pGeometryPath->GetString();

	// Load the rifle geometry into the first slot
	if (IsCharacterFile(geometryPath))
	{
		GetEntity()->LoadCharacter(0, geometryPath);
	}
	else
	{
		GetEntity()->LoadGeometry(0, geometryPath);
	}
}

void CRifle::RequestFire()
{
	SEntitySpawnParams spawnParams;
	spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("Bullet");

	spawnParams.vPosition = GetEntity()->GetWorldPos() + GetEntity()->GetWorldRotation() * Vec3(0, 1, 1);
	spawnParams.qRotation = GetEntity()->GetWorldRotation();
	spawnParams.vScale = Vec3(0.05f);

	// Spawn the entity, bullet is propelled in CBullet based on the rotation and position here
	gEnv->pEntitySystem->SpawnEntity(spawnParams);
}