#include "StdAfx.h"
#include "Bullet.h"

#include "Game/GameFactory.h"

class CBulletRegistrator
	: public IEntityRegistrator
{
	virtual void Register() override
	{
		CGameFactory::RegisterGameObject<CBullet>("Bullet", "", 0);

		RegisterCVars();
	}

	void RegisterCVars()
	{
		m_pGeometry = gEnv->pConsole->RegisterString("w_bulletGeometry", "Objects/Default/primitive_sphere.cgf", VF_CHEAT, "Sets the path to the geometry assigned to every weapon bullet");

		REGISTER_CVAR2("w_bulletMass", &m_mass, 5000.f, VF_CHEAT, "Sets the mass of each individual bullet fired by weapons");
		REGISTER_CVAR2("w_bulletInitialVelocity", &m_initialVelocity, 10.f, VF_CHEAT, "Sets the initial velocity of each individual bullet fired by weapons");

		REGISTER_CVAR2("w_bulletPostCollisionLifetime", &m_postCollisionLifetime, 1.f, VF_CHEAT, "Number of seconds that the bullet will remain for after colliding with another object");
	}

public:
	ICVar *m_pGeometry;

	float m_mass;
	float m_initialVelocity;

	float m_postCollisionLifetime;
};

CBulletRegistrator g_bulletRegistrator;

CBullet::CBullet()
	: m_removalTime(-1)
{
}

void CBullet::PostInit(IGameObject *pGameObject)
{
	// Make sure that this extension is updated regularly via the Update function below
	pGameObject->EnableUpdateSlot(this, 0);

	// Make sure we get logged collision events
	// Note the difference between immediate (directly on the physics thread) and logged (delayed to run on main thread) physics events.
	pGameObject->EnablePhysicsEvent(true, eEPE_OnCollisionLogged);

	const int requiredEvents[] = { eGFE_OnCollision };
	pGameObject->UnRegisterExtForEvents(this, NULL, 0);
	pGameObject->RegisterExtForEvents(this, requiredEvents, sizeof(requiredEvents) / sizeof(int));

	// Set the model
	GetEntity()->LoadGeometry(0, g_bulletRegistrator.m_pGeometry->GetString());

	// Now create the physical representation of the entity
	Physicalize();

	// Apply an impulse so that the bullet flies forward
	if (auto *pPhysics = GetEntity()->GetPhysics())
	{
		pe_action_impulse impulseAction;

		// Set the actual impulse, in this cause the value of the initial velocity CVar in bullet's forward direction
		impulseAction.impulse = GetEntity()->GetWorldRotation().GetColumn1() * g_bulletRegistrator.m_initialVelocity;

		// Send to the physical entity
		pPhysics->Action(&impulseAction);
	}
}

void CBullet::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	// Check if a removal time is currently scheduled
	// If so, check if the bullet has expired
	if (!isneg(m_removalTime) && gEnv->pTimer->GetFrameStartTime().GetSeconds() - m_removalTime > g_bulletRegistrator.m_postCollisionLifetime)
	{
		// Ask that the entity system removes this entity
		// This operation is normally queued, so will happen within a frame from now
		gEnv->pEntitySystem->RemoveEntity(GetEntityId());

		m_removalTime = -1;
	}
}

void CBullet::HandleEvent(const SGameObjectEvent &event)
{
	// Handle the OnCollision event, in order to have the entity removed on collision
	if (event.event == eGFE_OnCollision)
	{
		// Collision info can be retrieved using the event pointer
		//EventPhysCollision *physCollision = reinterpret_cast<EventPhysCollision *>(event.ptr);

		// Queue removal of this entity, unless it has already been done
		if (isneg(m_removalTime))
		{
			m_removalTime = gEnv->pTimer->GetFrameStartTime().GetSeconds() + g_bulletRegistrator.m_postCollisionLifetime;
		}
	}
}

void CBullet::Physicalize()
{
	SEntityPhysicalizeParams physParams;
	physParams.type = PE_RIGID;

	physParams.mass = g_bulletRegistrator.m_mass;

	GetEntity()->Physicalize(physParams);
}

void CBullet::Release()
{
	ISimpleExtension::Release();
}