#include "StdAfx.h"
#include "Rifle.h"

#include "Game/GameFactory.h"

class CRifleRegistrator
	: public IEntityRegistrator
{
	virtual void Register() override
	{
		CGameFactory::RegisterGameObject<CRifle>("Rifle");

		RegisterCVars();
	}

	void RegisterCVars()
	{
		m_pGeometryPath = REGISTER_STRING("w_rifleGeometryPath", "Objects/Weapons/SampleWeapon/motusweapon.cgf", VF_CHEAT, "Path to the rifle geometry that we want to load");

		REGISTER_CVAR2("w_rifleBulletScale", &m_bulletScale, 0.05f, VF_CHEAT, "Determines the scale of the bullet geometry");
	}

public:
	ICVar *m_pGeometryPath;
	float m_bulletScale;
};

CRifleRegistrator g_rifleRegistrator;

void CRifle::PostInit(IGameObject *pGameObject)
{
	const int geometrySlot = 0;

	LoadMesh(geometrySlot, g_rifleRegistrator.m_pGeometryPath->GetString());
}

void CRifle::RequestFire(const Vec3 &initialBulletPosition, const Quat &initialBulletRotation)
{
	SEntitySpawnParams spawnParams;
	spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("Bullet");

	spawnParams.vPosition = initialBulletPosition;
	spawnParams.qRotation = initialBulletRotation;

	spawnParams.vScale = Vec3(g_rifleRegistrator.m_bulletScale);

	// Spawn the entity, bullet is propelled in CBullet based on the rotation and position here
	gEnv->pEntitySystem->SpawnEntity(spawnParams);
}