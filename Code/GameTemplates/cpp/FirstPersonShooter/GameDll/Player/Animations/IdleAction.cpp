#include "StdAfx.h"
#include "IdleAction.h"

CIdleAction::CIdleAction(int priority, FragmentID fragmentID, const TagState fragTags, uint32 flags, ActionScopes scopeMask, uint32 userToken)
	: TBase(priority, fragmentID, fragTags, flags, scopeMask, userToken)
	, m_defaultFlags(flags)
{
}

IAction::EStatus CIdleAction::Update(float timePassed)
{
	QueryFragmentChange();

	return TBase::Update(timePassed);
}


IAction::EStatus CIdleAction::UpdatePending(float timePassed)
{
	EStatus ret = TBase::UpdatePending(timePassed);

	QueryFragmentChange();

	return ret;
}

void CIdleAction::QueryFragmentChange()
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_GAME);

	if (m_rootScope == nullptr)
		return;

	string newFragment = "";
	
	// Always fallback to the Idle fragment if nothing else was requested
	if (newFragment.size() == 0)
	{
		newFragment = "Idle";
	}

	RequestFragment(newFragment);
}

bool CIdleAction::RequestFragment(string fragmentName)
{
	FragmentID fragmentID = m_context->controllerDef.m_fragmentIDs.Find(fragmentName);
	if(fragmentID == FRAGMENT_ID_INVALID)
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed to find fragment %s!", fragmentName.c_str());
		return false;
	}

	bool bShouldChangeFragment = m_fragmentID == FRAGMENT_ID_INVALID || fragmentID != m_fragmentID;
	
	IActionPtr pCurrentAction = m_rootScope->GetAction();
	if(pCurrentAction && pCurrentAction != this)
	{
		if(pCurrentAction->GetPriority() < GetPriority())
		{
			pCurrentAction->Stop();
			bShouldChangeFragment = true;
		}
	}

	if (bShouldChangeFragment)
	{
		SetFragment(fragmentID, m_fragTags, GetRandomOptionIdx(fragmentID), 0, false);
		return true;
	}

	return false;
}

uint32 CIdleAction::GetRandomOptionIdx(FragmentID fragmentId)
{
	SFragmentQuery fragmentQuery;
	fragmentQuery.fragID = fragmentId;
	fragmentQuery.optionIdx = GetOptionIdx();
	fragmentQuery.tagState = m_rootScope->GetLastTagState();

	int optionsCount = m_rootScope->GetDatabase().FindBestMatchingTag(fragmentQuery);
	
	return (int)floor(cry_random(0, optionsCount));
}