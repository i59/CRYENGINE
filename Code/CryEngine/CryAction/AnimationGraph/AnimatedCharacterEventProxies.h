// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#ifndef __AnimatedCharacterComponents_h__
	#define __AnimatedCharacterComponents_h__

class CAnimatedCharacter;
class CAnimatedCharacterComponent_Base : public IEntityComponent
{
public:
	void SetAnimatedCharacter(CAnimatedCharacter *pAnimCharacter);

protected:

	typedef CAnimatedCharacterComponent_Base SuperClass;

	explicit CAnimatedCharacterComponent_Base();

	virtual void ProcessEvent(const SEntityEvent& event) override;
	virtual void OnPrePhysicsUpdate(float elapseTime) = 0;

	CAnimatedCharacter* m_pAnimCharacter;
};

class CAnimatedCharacterComponent_PrepareAnimatedCharacterForUpdate : public CAnimatedCharacterComponent_Base
{
public:
	DECLARE_COMPONENT("CAnimatedCharacterComponent_PrepareAnimatedCharacterForUpdate", 0x8EBC751B53D34DE4, 0x94CA9644D6D851A5)

	CAnimatedCharacterComponent_PrepareAnimatedCharacterForUpdate();

	ILINE void QueueRotation(const Quat& rotation) { m_queuedRotation = rotation; m_hasQueuedRotation = true; }
	ILINE void ClearQueuedRotation()               { m_hasQueuedRotation = false; }

private:
	Quat m_queuedRotation;
	bool m_hasQueuedRotation;
	virtual void OnPrePhysicsUpdate(float elapsedTime);
};

class CAnimatedCharacterComponent_StartAnimProc : public CAnimatedCharacterComponent_Base
{
public:
	DECLARE_COMPONENT("CAnimatedCharacterComponent_StartAnimProc", 0x24978C604D744CAE, 0xBCE1836773423372)

protected:

	virtual void                   OnPrePhysicsUpdate(float elapsedTime);
};

class CAnimatedCharacterComponent_GenerateMoveRequest : public CAnimatedCharacterComponent_Base
{
public:
	DECLARE_COMPONENT("CAnimatedCharacterComponent_GenerateMoveRequest", 0x7619F10FD914487E, 0x90F3C423D46D6AA5)

protected:

	virtual void                               OnPrePhysicsUpdate(float elapsedTime);
};

#endif // __AnimatedCharacterComponents_h__
