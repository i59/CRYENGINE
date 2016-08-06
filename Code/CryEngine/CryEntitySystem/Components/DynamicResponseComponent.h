// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#ifndef DYNAMICRESPONSESYSTEM_PROXY_H_
#define DYNAMICRESPONSESYSTEM_PROXY_H_

#include <CryDynamicResponseSystem/IDynamicResponseSystem.h>

//////////////////////////////////////////////////////////////////////////
// Description:
//    Implements dynamic response proxy class for entity.
//////////////////////////////////////////////////////////////////////////
class CDynamicResponseComponent final : public IEntityDynamicResponseComponent
{
public:
	CDynamicResponseComponent();
	virtual ~CDynamicResponseComponent();

	// IEntityComponent
	virtual void Initialize(IEntity &entity) override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	virtual void Release() override { delete this; }

	virtual void Serialize(TSerialize ser) override {}

	virtual bool NeedSerialize() override { return true; }

	virtual void GetMemoryUsage(ICrySizer* pSizer) const override
	{
		pSizer->AddObject(this, sizeof(*this));
	}
	// ~IEntityComponent

	//////////////////////////////////////////////////////////////////////////
	// IEntityDynamicResponseComponent interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual DRS::IVariableCollection* GetLocalVariableCollection() const override;
	virtual DRS::IResponseActor*      GetResponseActor() const override;
	//////////////////////////////////////////////////////////////////////////

private:
	DRS::IResponseActor* m_pResponseActor;
};

#endif // DYNAMICRESPONSESYSTEM_PROXY_H_
