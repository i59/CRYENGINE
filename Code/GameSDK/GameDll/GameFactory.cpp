// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Description:  Register the factory templates used to create classes from names
e.g. REGISTER_FACTORY(pFramework, "Player", CPlayer, false);
or   REGISTER_FACTORY(pFramework, "Player", CPlayerG4, false);

Since overriding this function creates template based linker errors,
it's been replaced by a standalone function in its own cpp file.

-------------------------------------------------------------------------
History:
- 17:8:2005   Created by Nick Hesketh - Refactor'd from Game.cpp/h

*************************************************************************/

#include "StdAfx.h"
#include "Game.h"
#include "Player.h"

//
#include "Item.h"
#include "Weapon.h"
#include "VehicleWeapon.h"
#include "VehicleWeaponGuided.h"
#include "VehicleWeaponControlled.h"
#include "VehicleMountedWeapon.h"
#include "Binocular.h"
#include "C4.h"
#include "DebugGun.h"
#include "GunTurret.h"
#include "JAW.h"
#include "AIGrenade.h"
#include "Accessory.h"
#include "HandGrenades.h"
#include "EnvironmentalWeapon.h"
#include "Laser.h"
#include "flashlight.h"
#include "DoubleMagazine.h"
#include "LTAG.h"
#include "HeavyMountedWeapon.h"
#include "HeavyWeapon.h"
#include "PickAndThrowWeapon.h"
#include "NoWeapon.h"
#include "WeaponMelee.h"
#include "UseableTurret.h"
#include "CinematicWeapon.h"

#include "DummyPlayer.h"

#include "ReplayObject.h"
#include "ReplayActor.h"

#include "MultiplayerEntities/CarryEntity.h"

#include "VehicleMovementBase.h"
#include "Vehicle/VehicleMovementDummy.h"
#include "VehicleActionAutomaticDoor.h"
#include "VehicleActionDeployRope.h"
#include "VehicleActionEntityAttachment.h"
#include "VehicleActionLandingGears.h"
#include "VehicleActionAutoTarget.h"
#include "VehicleDamageBehaviorBurn.h"
#include "VehicleDamageBehaviorCameraShake.h"
#include "VehicleDamageBehaviorExplosion.h"
#include "VehicleDamageBehaviorTire.h"
#include "VehicleDamageBehaviorAudioFeedback.h"
#include "VehicleMovementStdWheeled.h"
#include "VehicleMovementStdTank.h"
#include "VehicleMovementArcadeWheeled.h"
//#include "VehicleMovementHelicopterArcade.h"
#include "VehicleMovementHelicopter.h"
#include "VehicleMovementStdBoat.h"
#include "VehicleMovementTank.h"
#include "VehicleMovementMPVTOL.h"

#include "ScriptControlledPhysics.h"

#include "GameRules.h"
#include "GameRulesModules/GameRulesModulesManager.h"
#include "GameRulesModules/IGameRulesTeamsModule.h"
#include "GameRulesModules/GameRulesStandardTwoTeams.h"
#include "GameRulesModules/GameRulesGladiatorTeams.h"
#include "GameRulesModules/IGameRulesStateModule.h"
#include "GameRulesModules/GameRulesStandardState.h"
#include "GameRulesModules/GameRulesStandardVictoryConditionsTeam.h"
#include "GameRulesModules/GameRulesStandardVictoryConditionsPlayer.h"
#include "GameRulesModules/GameRulesObjectiveVictoryConditionsTeam.h"
#include "GameRulesModules/GameRulesObjectiveVictoryConditionsIndividualScore.h"
#include "GameRulesModules/GameRulesExtractionVictoryConditions.h"
#include "GameRulesModules/GameRulesSurvivorOneVictoryConditions.h"
#include "GameRulesModules/GameRulesStandardSetup.h"
#include "GameRulesModules/GameRulesStandardScoring.h"
#include "GameRulesModules/GameRulesAssistScoring.h"
#include "GameRulesModules/GameRulesStandardPlayerStats.h"
#include "GameRulesModules/IGameRulesSpawningModule.h"
#include "GameRulesModules/GameRulesSpawningBase.h"
#include "GameRulesModules/GameRulesMPSpawning.h"
#include "GameRulesModules/GameRulesMPSpawningWithLives.h"
#include "GameRulesModules/GameRulesMPWaveSpawning.h"
#include "GameRulesModules/GameRulesMPDamageHandling.h"
#include "GameRulesModules/GameRulesMPActorAction.h"
#include "GameRulesModules/GameRulesMPSpectator.h"
#include "GameRulesModules/GameRulesSPDamageHandling.h"
#include "GameRulesModules/GameRulesObjective_Predator.h"
#include "GameRulesModules/GameRulesStandardRounds.h"
#include "GameRulesModules/GameRulesStatsRecording.h"
#include "GameRulesModules/GameRulesObjective_PowerStruggle.h"
#include "GameRulesModules/GameRulesObjective_Extraction.h"
#include "GameRulesModules/GameRulesSimpleEntityBasedObjective.h"
#include "GameRulesModules/GameRulesObjective_CTF.h"

#include "Environment/Tornado.h"
#include "Environment/Shake.h"
#include "Environment/Rain.h"
#include "Environment/Snow.h"
#include "Environment/InteractiveObject.h"
#include "Environment/DeflectorShield.h"
#include "Environment/DangerousRigidBody.h"
#include "Environment/Ledge.h"
#include "Environment/WaterPuddle.h"
#include "Environment/SmartMine.h"
#include "Environment/MineField.h"
#include "Environment/DoorPanel.h"
#include "Environment/VicinityDependentObjectMover.h"
#include "Environment/WaterRipplesGenerator.h"
#include "Environment/LightningArc.h"

#include "AI/AICorpse.h"

#include "Turret/Turret/Turret.h"
#include "MPPath.h"

#include <IItemSystem.h>
#include <IVehicleSystem.h>
#include <IGameRulesSystem.h>
#include <CryGame/IGameVolumes.h>

#include "GameCVars.h"

#define HIDE_FROM_EDITOR(className)																																				\
  { IEntityClass *pItemClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(className);\
  pItemClass->SetFlags(pItemClass->GetFlags() | ECLF_INVISIBLE); }																				\

#define REGISTER_EDITOR_VOLUME_CLASS(frameWork, className)                                          \
{	                                                                                                  \
	IGameVolumes* pGameVolumes = frameWork->GetIGameVolumesManager();                                 \
	IGameVolumesEdit* pGameVolumesEdit = pGameVolumes ? pGameVolumes->GetEditorInterface() : NULL;    \
	if (pGameVolumesEdit != NULL)                                                                     \
	{                                                                                                 \
		pGameVolumesEdit->RegisterEntityClass( className );                                             \
	}                                                                                                 \
} 

#define REGISTER_ITEM(name) IItemSystem::RegisterItemClass<C##name>(#name);

// Register the factory templates used to create classes from names. Called via CGame::Init()
void InitGameFactory(IGameFramework *pFramework)
{
	assert(pFramework);

	IEntityComponent::RegisterEntityWithComponent<CPlayer>("Player", VF_INVISIBLE, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/Actor/Player.lua");
	IEntityComponent::RegisterEntityWithComponent<CPlayer>("PlayerHeavy", VF_INVISIBLE, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/Actor/PlayerHeavy.lua");

	IEntityComponent::RegisterEntityWithComponent<CPlayer>("DamageTestEnt", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/AI/DamageTestEnt.lua");

#if (USE_DEDICATED_INPUT)
	IEntityComponent::RegisterEntityWithComponent<CDummyPlayer>("DummyPlayer", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/AI/DummyPlayer.lua");
#endif

	//REGISTER_FACTORY(pFramework, "Civilian", CPlayer, true);

	// Null AI for AI pool
	IEntityComponent::RegisterEntityWithComponent<CPlayer>("NullAI", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/AI/NullAI.lua");

	// Characters	
	IEntityComponent::RegisterEntityWithComponent<CPlayer>("Human", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/AI/Characters/Human.lua");
	
	class CBasicItem : public CItem
	{
	public:
		DECLARE_COMPONENT("Item", 0x0F8F9D3D11C54403, 0x928E34D7CD4439E1)
	};

	// Items
	IItemSystem::RegisterItemClass<CBasicItem>("Item");
	REGISTER_ITEM(Accessory);
	REGISTER_ITEM(Laser);
	REGISTER_ITEM(FlashLight);
	REGISTER_ITEM(DoubleMagazine);
	REGISTER_ITEM(HandGrenades);

	class CBasicWeapon : public CWeapon
	{
	public:
		DECLARE_COMPONENT("Weapon", 0x028F9D7D01C5448F, 0x9213B189E52BC565)
	};

	// Weapons
	IItemSystem::RegisterItemClass<CBasicWeapon>("Weapon");
	REGISTER_ITEM(VehicleWeapon);
	REGISTER_ITEM(VehicleWeaponGuided);
	REGISTER_ITEM(VehicleWeaponControlled);
	REGISTER_ITEM(VehicleWeaponPulseC);
	REGISTER_ITEM(VehicleMountedWeapon);
	REGISTER_ITEM(Binocular);
	REGISTER_ITEM(C4);
	REGISTER_ITEM(DebugGun);
	REGISTER_ITEM(GunTurret);
	IItemSystem::RegisterItemClass<CJaw>("JAW");
	REGISTER_ITEM(AIGrenade);
	IItemSystem::RegisterItemClass<CAIGrenade>("AISmokeGrenades");
	IItemSystem::RegisterItemClass<CAIGrenade>("AIEMPGrenade");
	IItemSystem::RegisterItemClass<CLTag>("LTAG");
	REGISTER_ITEM(PickAndThrowWeapon);
	REGISTER_ITEM(NoWeapon);
	REGISTER_ITEM(HeavyMountedWeapon);
	REGISTER_ITEM(HeavyWeapon);
	REGISTER_ITEM(WeaponMelee);
	REGISTER_ITEM(UseableTurret);
	REGISTER_ITEM(CinematicWeapon);
	
	// vehicle objects
	IVehicleSystem* pVehicleSystem = pFramework->GetIVehicleSystem();

#define REGISTER_VEHICLEOBJECT(name, obj) \
	REGISTER_FACTORY(pVehicleSystem, name, obj, false); \
	obj::m_objectId = pVehicleSystem->AssignVehicleObjectId(name);

	// damage behaviours
	REGISTER_VEHICLEOBJECT("Burn", CVehicleDamageBehaviorBurn);
	REGISTER_VEHICLEOBJECT("CameraShake", CVehicleDamageBehaviorCameraShake);
	REGISTER_VEHICLEOBJECT("Explosion", CVehicleDamageBehaviorExplosion);
	REGISTER_VEHICLEOBJECT("BlowTire", CVehicleDamageBehaviorBlowTire);
	REGISTER_VEHICLEOBJECT("AudioFeedback", CVehicleDamageBehaviorAudioFeedback);

	// actions
	REGISTER_VEHICLEOBJECT("AutomaticDoor", CVehicleActionAutomaticDoor);
	REGISTER_VEHICLEOBJECT("EntityAttachment", CVehicleActionEntityAttachment);
	REGISTER_VEHICLEOBJECT("LandingGears", CVehicleActionLandingGears);
	REGISTER_VEHICLEOBJECT("AutoAimTarget", CVehicleActionAutoTarget);

	//seat actions
	REGISTER_VEHICLEOBJECT("DeployRope", CVehicleActionDeployRope);

	// vehicle movements
	REGISTER_FACTORY(pVehicleSystem, "DummyMovement", CVehicleMovementDummy, false);
	//REGISTER_FACTORY(pVehicleSystem, "HelicopterArcade", CVehicleMovementHelicopterArcade, false);
	REGISTER_FACTORY(pVehicleSystem, "Helicopter", CVehicleMovementHelicopter, false);
	REGISTER_FACTORY(pVehicleSystem, "StdBoat", CVehicleMovementStdBoat, false);
	REGISTER_FACTORY(pVehicleSystem, "StdWheeled", CVehicleMovementStdWheeled, false);
	REGISTER_FACTORY(pVehicleSystem, "StdTank", CVehicleMovementStdTank, false);
	REGISTER_FACTORY(pVehicleSystem, "ArcadeWheeled", CVehicleMovementArcadeWheeled, false);
	REGISTER_FACTORY(pVehicleSystem, "Tank", CVehicleMovementTank, false);
	REGISTER_FACTORY(pVehicleSystem, "MPVTOL", CVehicleMovementMPVTOL, false);


	// Custom GameObjects
	IEntityComponent::RegisterEntityWithComponent<CTornado>("Tornado", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/Environment/Tornado.lua");
	IEntityComponent::RegisterEntityWithComponent<CShake>("Shake", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/Environment/Shake.lua");
	IEntityComponent::RegisterEntityWithComponent<CRain>("Rain", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/Environment/Rain.lua");
	IEntityComponent::RegisterEntityWithComponent<CSnow>("Snow", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/Environment/Snow.lua");
	IEntityComponent::RegisterEntityWithComponent<CInteractiveObjectEx>("InteractiveObjectEx", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/PlayerInteractive/InteractiveObjectEx.lua");
	IEntityComponent::RegisterEntityWithComponent<CDeployableBarrier>("DeployableBarrier", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/PlayerInteractive/DeployableBarrier.lua");
	IEntityComponent::RegisterEntityWithComponent<CReplayObject>("ReplayObject");
	IEntityComponent::RegisterEntityWithComponent<CReplayActor>("ReplayActor");
	IEntityComponent::RegisterEntityWithComponent<CDeflectorShield>("DeflectorShield", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/Others/DeflectorShield.lua");
	HIDE_FROM_EDITOR("DeflectorShield");
	IEntityComponent::RegisterEntityWithComponent<CEnvironmentalWeapon>("EnvironmentalWeapon", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/Multiplayer/EnvironmentWeapon_Rooted.lua");
	IEntityComponent::RegisterEntityWithComponent<CDangerousRigidBody>("DangerousRigidBody", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/Multiplayer/DangerousRigidBody.lua");
	IEntityComponent::RegisterEntityWithComponent<CAICorpse>("AICorpse");
	HIDE_FROM_EDITOR("ReplayObject");
	HIDE_FROM_EDITOR("ReplayActor");
	HIDE_FROM_EDITOR("AICorpse");
	HIDE_FROM_EDITOR("NullAI");

	//////////////////////////////////////////////////////////////////////////
	/// Shape/Volume objects
	IEntityComponent::RegisterEntityWithComponent<CMPPath>("MPPath", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/Multiplayer/MPPath.lua");
	HIDE_FROM_EDITOR("MPPath");
	REGISTER_EDITOR_VOLUME_CLASS( pFramework, "MPPath" );

	IEntityComponent::RegisterEntityWithComponent<CLedgeObject>("LedgeObject", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/ContextualNavigation/LedgeObject.lua");
	HIDE_FROM_EDITOR("LedgeObject");
	REGISTER_EDITOR_VOLUME_CLASS( pFramework, "LedgeObject" );
	IEntityComponent::RegisterEntityWithComponent<CLedgeObjectStatic>("LedgeObjectStatic", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/ContextualNavigation/LedgeObjectStatic.lua");
	HIDE_FROM_EDITOR("LedgeObjectStatic");
	REGISTER_EDITOR_VOLUME_CLASS( pFramework, "LedgeObjectStatic" );

	IEntityComponent::RegisterEntityWithComponent<CWaterPuddle>("WaterPuddle", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/Environment/WaterPuddle.lua");
	HIDE_FROM_EDITOR("WaterPuddle");
	REGISTER_EDITOR_VOLUME_CLASS(pFramework, "WaterPuddle");
	//////////////////////////////////////////////////////////////////////////


	IEntityComponent::RegisterEntityWithComponent<CSmartMine>("SmartMine", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/Environment/SmartMine.lua");
	IEntityComponent::RegisterEntityWithComponent<CMineField>("MineField", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/Environment/MineField.lua");
	IEntityComponent::RegisterEntityWithComponent<CDoorPanel>("DoorPanel", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/Environment/DoorPanel.lua");
	IEntityComponent::RegisterEntityWithComponent<CVicinityDependentObjectMover>("VicinityDependentObjectMover", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/Environment/VicinityDependentObjectMover.lua");
	IEntityComponent::RegisterEntityWithComponent<CWaterRipplesGenerator>("WaterRipplesGenerator", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/Environment/WaterRipplesGenerator.lua");
	IEntityComponent::RegisterEntityWithComponent<CLightningArc>("LightningArc", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/Environment/LightningArc.lua");

	IEntityComponent::RegisterEntityWithComponent<CCarryEntity>("CTFFlag", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/Multiplayer/CTFFlag.lua");
	IEntityComponent::RegisterEntityWithComponent<CTurret>("Turret", 0, SEditorClassInfo(), nullptr, 0, "Scripts/Entities/Turret/Turret.lua");
	
	HIDE_FROM_EDITOR("CTFFlag");
	IEntityClassRegistry::SEntityClassDesc stdClass;
	stdClass.flags |= ECLF_INVISIBLE|ECLF_DEFAULT;
	stdClass.sName = "Corpse";
	gEnv->pEntitySystem->GetClassRegistry()->RegisterStdClass(stdClass);

	// Register the game mode as an external component to be instantiated by CryAction
	IEntityComponent::RegisterExternalComponent<CGameRules>();

	IGameRulesModulesManager *pGameRulesModulesManager = CGameRulesModulesManager::GetInstance();

	REGISTER_FACTORY(pGameRulesModulesManager, "StandardTwoTeams", CGameRulesStandardTwoTeams, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "GladiatorTeams", CGameRulesGladiatorTeams, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "StandardState", CGameRulesStandardState, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "StandardVictoryConditionsTeam", CGameRulesStandardVictoryConditionsTeam, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "StandardVictoryConditionsPlayer", CGameRulesStandardVictoryConditionsPlayer, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "ObjectiveVictoryConditionsTeam", CGameRulesObjectiveVictoryConditionsTeam, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "ObjectiveVictoryConditionsIndiv", CGameRulesObjectiveVictoryConditionsIndividualScore, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "ExtractionVictoryConditions", CGameRulesExtractionVictoryConditions, false);
#if SURVIVOR_ONE_ENABLED
	REGISTER_FACTORY(pGameRulesModulesManager, "SurvivorOneVictoryConditions", CGameRulesSurvivorOneVictoryConditions, false);
#endif
	REGISTER_FACTORY(pGameRulesModulesManager, "StandardSetup", CGameRulesStandardSetup, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "StandardScoring", CGameRulesStandardScoring, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "AssistScoring", CGameRulesAssistScoring, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "StandardPlayerStats", CGameRulesStandardPlayerStats, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "SpawningBase", CGameRulesSpawningBase, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "MPRSSpawning", CGameRulesRSSpawning, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "StandardStatsRecording", CGameRulesStatsRecording, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "MPSpawningWithLives", CGameRulesMPSpawningWithLives, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "MPWaveSpawning", CGameRulesMPWaveSpawning, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "MPDamageHandling", CGameRulesMPDamageHandling, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "MPActorAction", CGameRulesMPActorAction, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "MPSpectator", CGameRulesMPSpectator, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "SPDamageHandling", CGameRulesSPDamageHandling, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "Objective_Predator", CGameRulesObjective_Predator, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "StandardRounds", CGameRulesStandardRounds, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "Objective_PowerStruggle", CGameRulesObjective_PowerStruggle, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "Objective_Extraction", CGameRulesObjective_Extraction, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "Objective_SimpleEntityBased", CGameRulesSimpleEntityBasedObjective, false);
	REGISTER_FACTORY(pGameRulesModulesManager, "Objective_CTF", CGameRulesObjective_CTF, false);

	pGameRulesModulesManager->Init();
}
