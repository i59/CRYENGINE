#pragma once

#include <CryEntitySystem/IEntityClass.h>
#include <CryEntitySystem/IEntitySystem.h>

// IEntityPropertyHandler handler implementation that keeps track of all property values in all instances of an entity class
class CNativeEntityPropertyHandler
	: public IEntityPropertyHandler
	, public IEntitySystemSink
{
public:
	typedef std::unordered_map<int, string> TPropertyStorage;

	CNativeEntityPropertyHandler(SNativeEntityPropertyInfo *pProperties, int numProperties, uint32 scriptFlags);
	~CNativeEntityPropertyHandler();

	// IEntityPropertyHandler
	virtual void GetMemoryUsage(ICrySizer *pSizer) const { pSizer->Add(m_pProperties); }
	virtual void RefreshProperties() {}
	virtual void LoadEntityXMLProperties(IEntity* entity, const XmlNodeRef& xml);
	virtual void LoadArchetypeXMLProperties(const char* archetypeName, const XmlNodeRef& xml) {}
	virtual void InitArchetypeEntity(IEntity* entity, const char* archetypeName, const SEntitySpawnParams& spawnParams) {}

	virtual int GetPropertyCount() const { return m_numProperties; }

	virtual bool GetPropertyInfo(int index, SPropertyInfo &info) const
	{
		if (index >= m_numProperties)
			return false;

		info = m_pProperties[index].info;
		return true;
	}

	virtual void SetProperty(IEntity* entity, int index, const char* value);

	virtual const char* GetProperty(IEntity* entity, int index) const;

	virtual const char* GetDefaultProperty(int index) const
	{
		if (index >= m_numProperties)
			return "";

		return m_pProperties[index].defaultValue;
	}

	virtual void PropertiesChanged(IEntity *entity);

	virtual uint32 GetScriptFlags() const { return m_scriptFlags; }
	// -IEntityPropertyHandler

	// IEntitySystemSink
	virtual bool OnBeforeSpawn(SEntitySpawnParams &params) { return false; }
	virtual void OnSpawn(IEntity *pEntity, SEntitySpawnParams &params) {}
	virtual void OnReused(IEntity *pEntity, SEntitySpawnParams &params) {}
	virtual void OnEvent(IEntity *pEntity, SEntityEvent &event) {}
	virtual bool OnRemove(IEntity *pEntity);
	// ~IEntitySystemSink

	void LoadEntityXMLGroupProperties(IEntity *pEntity, const XmlNodeRef &groupNode, bool bRootNode);

protected:
	SNativeEntityPropertyInfo *m_pProperties;
	int m_numProperties;

	int m_scriptFlags;

	std::unordered_map<IEntity *, TPropertyStorage> m_entityPropertyMap;
};