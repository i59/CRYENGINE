// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "EntityComponentRMIDispatch.h"
#include "Network/GameContext.h"

CEntityComponentRMIDispatch::CEntityComponentRMIDispatch() : m_bSafety(false)
{
}

CEntityComponentRMIDispatch::~CEntityComponentRMIDispatch()
{
}

void CEntityComponentRMIDispatch::RegisterInterface(SRemoteComponentFunction* pMessages, size_t nCount)
{
	if (nCount == 0)
		return;

	// watch out for duplicate registrations...
	for (size_t i = 0; i < m_messages.size(); i++)
		if (m_messages[i]->pBase == pMessages)
			return;
	// messages already registered

	if (m_bSafety)
	{
		CryFatalError("CEntityComponentRMIDispatch::RegisterInterface: occurs too late");
		return;
	}

	// actually build protocol definitions for the messages
	for (size_t i = 0; i < nCount; i++)
	{
		std::vector<SNetMessageDef>* pMsgDef = pMessages[i].isServerCall ? &m_serverCalls : &m_clientCalls;

		SNetMessageDef md;
		md.description = pMessages[i].description;
		md.handler = Trampoline;
		md.nUser = m_messages.size();
		switch (pMessages[i].attach)
		{
		case eRAT_NoAttach:     // Intentional fall-through
		case eRAT_Urgent:       // Intentional fall-through
		case eRAT_Independent:
			// NoAttach, Urgent and Independent get the reliability specified at RMI declaration
			md.reliability = pMessages[i].reliability;
			break;
		default:
			// Everything else's reliability/orderedness is based on the object they're attached to
			md.reliability = eNRT_UnreliableOrdered;
		}
		md.parallelFlags = eMPF_BlocksStateChange | eMPF_DiscardIfNoEntity;
		if (pMessages[i].lowDelay)
			md.parallelFlags |= eMPF_NoSendDelay;

		pMsgDef->push_back(md);
		m_messages.push_back(pMessages + i);
	}
}

TNetMessageCallbackResult CEntityComponentRMIDispatch::Trampoline(
  uint32 userId,
  INetMessageSink* handler,
  TSerialize serialize,
  uint32 curSeq,
  uint32 oldSeq,
  uint32 timeFraction32,
  EntityId* pEntityId,
  INetChannel* pNetChannel)
{
	const SRemoteComponentFunction* pRMI = static_cast<CEntityComponentRMIDispatch *>(CCryAction::GetCryAction()->GetRMIDispatch())->m_messages[userId];
	IRMIAtSyncItem* pItem = (IRMIAtSyncItem*) pRMI->decoder(serialize, pEntityId, pNetChannel);
	if (pItem)
	{
		CCryAction* pCryAction = CCryAction::GetCryAction();
		CGameContext* pGameContext = pCryAction ? pCryAction->GetGameContext() : NULL;
		INetContext* pNetContext = pGameContext ? pGameContext->GetNetContext() : NULL;

		if (pNetContext)
			pNetContext->LogCppRMI(*pEntityId, pItem);
	}
	return TNetMessageCallbackResult(pItem != 0, pItem);
}

void CEntityComponentRMIDispatch::LockSafety()
{
	if (m_bSafety)
		return;

	for (size_t i = 0; i < m_serverCalls.size(); i++)
		m_messages[m_serverCalls[i].nUser]->pMsgDef = &m_serverCalls[i];
	for (size_t i = 0; i < m_clientCalls.size(); i++)
		m_messages[m_clientCalls[i].nUser]->pMsgDef = &m_clientCalls[i];

	m_bSafety = true;
}

bool CEntityComponentRMIDispatch::CProtocolDef::IsServer()
{
	return this == &static_cast<CEntityComponentRMIDispatch *>(CCryAction::GetCryAction()->GetRMIDispatch())->m_serverDef;
}

void CEntityComponentRMIDispatch::CProtocolDef::DefineProtocol(IProtocolBuilder* pBuilder)
{
	auto *pDispatch = static_cast<CEntityComponentRMIDispatch *>(CCryAction::GetCryAction()->GetRMIDispatch());

	pDispatch->LockSafety();

	std::vector<SNetMessageDef>* pSending = IsServer() ? &pDispatch->m_clientCalls : &pDispatch->m_serverCalls;
	std::vector<SNetMessageDef>* pReceiving = IsServer() ? &pDispatch->m_serverCalls : &pDispatch->m_clientCalls;

	SNetProtocolDef protoSending;
	protoSending.nMessages = pSending->size();
	protoSending.vMessages = protoSending.nMessages ? &*pSending->begin() : 0;
	SNetProtocolDef protoReceiving;
	protoReceiving.nMessages = pReceiving->size();
	protoReceiving.vMessages = protoReceiving.nMessages ? &*pReceiving->begin() : 0;

	pBuilder->AddMessageSink(this, protoSending, protoReceiving);
}

bool CEntityComponentRMIDispatch::CProtocolDef::HasDef(const SNetMessageDef* pDef)
{
	auto *pDispatch = static_cast<CEntityComponentRMIDispatch *>(CCryAction::GetCryAction()->GetRMIDispatch());

	return
	  pDef >= &*pDispatch->m_serverCalls.begin() && pDef < &*pDispatch->m_serverCalls.end() ||
	  pDef >= &*pDispatch->m_clientCalls.begin() && pDef < &*pDispatch->m_clientCalls.end();
}

void CEntityComponentRMIDispatch::GetMemoryUsage(ICrySizer* s) const
{
	//s->AddObject(m_messages);
	//s->AddObject(m_clientCalls);
	//s->AddObject(m_serverCalls);
}
