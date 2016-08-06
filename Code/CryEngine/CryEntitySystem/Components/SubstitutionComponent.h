// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:   SubstitutionProxy.h
//  Version:     v1.00
//  Created:     7/6/2005 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description:
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __SubstitutionProxy_h__
#define __SubstitutionProxy_h__
#pragma once

//////////////////////////////////////////////////////////////////////////
// Description:
//    Implements base substitution proxy class for entity.
//////////////////////////////////////////////////////////////////////////
struct CSubstitutionComponent : IEntitySubstitutionComponent
{
public:
	CSubstitutionComponent() { m_pSubstitute = 0; }
	virtual ~CSubstitutionComponent();

	// IEntityComponent
	virtual void OnEntityReload(SEntitySpawnParams& params, XmlNodeRef entityNode);

	virtual void Serialize(TSerialize ser) override;
	virtual bool NeedSerialize() override;

	virtual void GetMemoryUsage(ICrySizer* pSizer) const override
	{
		pSizer->AddObject(this, sizeof(*this));
	}
	// ~IEntityComponent

	//////////////////////////////////////////////////////////////////////////
	// IEntitySubstitutionProxy interface.
	//////////////////////////////////////////////////////////////////////////
	virtual void         SetSubstitute(IRenderNode* pSubstitute);
	virtual IRenderNode* GetSubstitute() { return m_pSubstitute; }
	//////////////////////////////////////////////////////////////////////////

protected:
	IRenderNode* m_pSubstitute;
};

#endif // __SubstitutionProxy_h__
