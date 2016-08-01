// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

class CGameEntityNodeFactory;

struct SEntityScriptProperties
{
	SEntityScriptProperties()
		: pEntityTable(nullptr)
		, pEditorTable(nullptr)
		, pPropertiesTable(nullptr)
		, pInstancePropertiesTable(nullptr)
	{
	}

	IScriptTable* pEntityTable;
	IScriptTable* pEditorTable;
	IScriptTable* pPropertiesTable;
	IScriptTable* pInstancePropertiesTable;
};

class CGameFactory
{
public:
	static void Init();
	static void RegisterEntityFlowNodes();

private:
	template<class T>
	static void RegisterNoScriptGameObject(const string& name, const string& path);
	static void CreateScriptTables(SEntityScriptProperties& out);

	static std::map<string, CGameEntityNodeFactory*> s_flowNodeFactories;
};
