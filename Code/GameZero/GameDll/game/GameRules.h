// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <IGameRulesSystem.h>

class CGameRules : public IGameRules
{
public:
	CGameRules();
	virtual ~CGameRules();

	DECLARE_COMPONENT("GameRules", 0xFDD4393DDA43468F, 0x87BD6AC352616E67)

	// IEntityComponent
	virtual void        PostInitialize() override;

	virtual void Release() override { delete this; }

	virtual void        GetMemoryUsage(ICrySizer* s) const override;
	// ~IEntityComponent

	//IGameRules
	virtual bool        ShouldKeepClient(int channelId, EDisconnectionCause cause, const char* desc) const override                                                                                                         { return true; }
	virtual void        PrecacheLevel() override                                                                                                                                                                            {}
	virtual void        PrecacheLevelResource(const char* resourceName, EGameResourceType resourceType) override                                                                                                            {}
	virtual XmlNodeRef  FindPrecachedXmlFile(const char* sFilename) override                                                                                                                                                { return 0; }
	virtual void        OnConnect(struct INetChannel* pNetChannel) override                                                                                                                                                 {}
	virtual void        OnDisconnect(EDisconnectionCause cause, const char* desc) override                                                                                                                                  {}
	virtual bool        OnClientConnect(int channelId, bool isReset) override;
	virtual void        OnClientDisconnect(int channelId, EDisconnectionCause cause, const char* desc, bool keepClient) override                                                                                            {}
	virtual bool        OnClientEnteredGame(int channelId, bool isReset) override                                                                                                                                           { return true; }
	virtual void        OnEntitySpawn(IEntity* pEntity) override                                                                                                                                                            {}
	virtual void        OnEntityRemoved(IEntity* pEntity) override                                                                                                                                                          {}
	virtual void        SendTextMessage(ETextMessageType type, const char* msg, uint32 to = eRMI_ToAllClients, int channelId = -1, const char* p0 = 0, const char* p1 = 0, const char* p2 = 0, const char* p3 = 0) override {}
	virtual void        SendChatMessage(EChatMessageType type, EntityId sourceId, EntityId targetId, const char* msg) override                                                                                              {}
	virtual void        ForbiddenAreaWarning(bool active, int timer, EntityId targetId)                                                                                                                                     {}
	virtual float       GetRemainingGameTime() const override                                                                                                                                                               { return 0.0f; }
	virtual void        SetRemainingGameTime(float seconds) override                                                                                                                                                        {}
	virtual void        ClearAllMigratingPlayers(void) override                                                                                                                                                             {}
	virtual EntityId    SetChannelForMigratingPlayer(const char* name, uint16 channelID) override                                                                                                                           { return INVALID_ENTITYID; }
	virtual void        StoreMigratingPlayer(IActor* pActor) override                                                                                                                                                       {}
	virtual bool        IsTimeLimited() const override                                                                                                                                                                      { return false; }
	virtual void        OnEntityReused(IEntity* pEntity, SEntitySpawnParams& params, EntityId prevId) override                                                                                                              {}
	virtual void        ClientHit(const HitInfo& hitInfo) override                                                                                                                                                          {}
	virtual void        ServerHit(const HitInfo& hitInfo) override                                                                                                                                                          {}
	virtual int         GetHitTypeId(const uint32 crc) const override                                                                                                                                                       { return 0; }
	virtual int         GetHitTypeId(const char* type) const override                                                                                                                                                       { return 0; }
	virtual const char* GetHitType(int id) const override                                                                                                                                                                   { return nullptr; }
	virtual void        OnVehicleDestroyed(EntityId id) override                                                                                                                                                            {}
	virtual void        OnVehicleSubmerged(EntityId id, float ratio) override                                                                                                                                               {}
	virtual void        CreateEntityRespawnData(EntityId entityId) override                                                                                                                                                 {}
	virtual bool        HasEntityRespawnData(EntityId entityId) const override                                                                                                                                              { return false; }
	virtual void        ScheduleEntityRespawn(EntityId entityId, bool unique, float timer) override                                                                                                                         {}
	virtual void        AbortEntityRespawn(EntityId entityId, bool destroyData) override                                                                                                                                    {}
	virtual void        ScheduleEntityRemoval(EntityId entityId, float timer, bool visibility) override                                                                                                                     {}
	virtual void        AbortEntityRemoval(EntityId entityId) override                                                                                                                                                      {}
	virtual void        AddHitListener(IHitListener* pHitListener) override                                                                                                                                                 {}
	virtual void        RemoveHitListener(IHitListener* pHitListener) override                                                                                                                                              {}
	virtual bool ApproveCollision(const EventPhysCollision& collision) override { return true; }
	virtual void        ShowStatus() override                                                                                                                                                                               {}
	virtual bool        CanEnterVehicle(EntityId playerId) override                                                                                                                                                         { return true; }
	virtual const char* GetTeamName(int teamId) const override                                                                                                                                                              { return nullptr; }
	//~IGameRules
};
