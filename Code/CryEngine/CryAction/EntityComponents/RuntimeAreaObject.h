// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#ifndef RUNTIMEAREAOBJECT_H_INCLUDED
#define RUNTIMEAREAOBJECT_H_INCLUDED

class CRuntimeAreaObject : public IEntityComponent
{
public:
	DECLARE_COMPONENT("RuntimeAreaObject", 0x1D0AEE3A9D6049B9, 0x90B768BEA12DE3B5)

	typedef unsigned int TSurfaceCRC;

	struct SAudioControls
	{
		AudioControlId audioTriggerId;
		AudioControlId audioRtpcId;

		explicit SAudioControls(
		  AudioControlId _audioTriggerId = INVALID_AUDIO_CONTROL_ID,
		  AudioControlId _audioRtpcId = INVALID_AUDIO_CONTROL_ID)
			: audioTriggerId(_audioTriggerId)
			, audioRtpcId(_audioRtpcId)
		{}
	};

	typedef std::map<TSurfaceCRC, SAudioControls> TAudioControlMap;

	static TAudioControlMap m_audioControls;

	virtual ~CRuntimeAreaObject() override;

	// IEntityComponent
	virtual void ProcessEvent(const SEntityEvent& event) override;

	virtual void GetMemoryUsage(ICrySizer* pSizer) const override;
	// ~IEntityComponent

private:

	struct SAreaSoundInfo
	{
		SAudioControls audioControls;
		float          parameter;

		explicit SAreaSoundInfo(SAudioControls const& _audioControls, float const _parameter = 0.0f)
			: audioControls(_audioControls)
			, parameter(_parameter)
		{}

		~SAreaSoundInfo()
		{}
	};

	typedef std::map<TSurfaceCRC, SAreaSoundInfo>  TAudioParameterMap;
	typedef std::map<EntityId, TAudioParameterMap> TEntitySoundsMap;

	void UpdateParameterValues(IEntity* const pEntity, TAudioParameterMap& paramMap);
	void StopEntitySounds(EntityId const entityId, TAudioParameterMap& paramMap);

	TEntitySoundsMap m_activeEntitySounds;
};

#endif // RUNTIMEAREAOBJECT_H_INCLUDED
