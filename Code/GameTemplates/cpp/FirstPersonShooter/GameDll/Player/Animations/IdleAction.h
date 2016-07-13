#pragma once

#include <../CryAction/ICryMannequin.h>

class CIdleAction
	: public TAction<SAnimationContext>
{
	typedef TAction<SAnimationContext> TBase;

public:
	CIdleAction(int priority, FragmentID fragmentID = FRAGMENT_ID_INVALID, const TagState fragTags = TAG_STATE_EMPTY, uint32 flags = 0, ActionScopes scopeMask = 0, uint32 userToken = 0);
	
	// TBase
	virtual EStatus Update(float timePassed) override;
	virtual EStatus UpdatePending(float timePassed) override;

	virtual void OnInitialise() override { m_flags = m_defaultFlags; }
	// ~TBase

protected:
	void QueryFragmentChange();

	bool RequestFragment(string fragmentId);
	uint32 GetRandomOptionIdx(FragmentID fragmentId);

private:
	uint32 m_defaultFlags;
};