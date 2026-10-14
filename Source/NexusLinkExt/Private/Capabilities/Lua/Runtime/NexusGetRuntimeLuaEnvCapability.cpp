// Copyright byteyang. All Rights Reserved.

#include "Capabilities/Lua/Runtime/NexusGetRuntimeLuaEnvCapability.h"
#include "Utils/NexusArgs.h"

#if WITH_UNLUA

#include "NexusCapabilityRegistry.h"
#include "NexusMcpSchemaBuilder.h"
#include "Utils/NexusLuaUtils.h"
#include "Utils/NexusStringMatchUtils.h"

void FNexusGetRuntimeLuaEnvCapability::BuildDefinition(FNexusCapabilityDefinition& Out) const
{
	Out.Name = TEXT("GetRuntimeLuaEnv");
	Out.Description = TEXT("List _G or nested table keys. Supports NameFilter+limit.");
	Out.InputSchema = FNexusSchema::Object()
		.Prop(TEXT("LuaPath"),    FNexusSchema::Str(TEXT("Dot-separated table path; omit for _G")))
		.Prop(TEXT("NameFilter"), FNexusSchema::Str(TEXT("Key filter; /regex/, ^prefix, suffix$")))
		.Prop(TEXT("Limit"),      FNexusSchema::Int(TEXT("Max return count"), 100, 1, 500))
		.Build();
	Out.Tags = {FNexusMcpTags::Readonly, FNexusMcpTags::Runtime };
	Out.ExtraSearchKeywords = { TEXT("environment"), TEXT("global"), TEXT("table"), TEXT("Keys"), TEXT("scope") };
	Out.RelatedCapabilities = { TEXT("GetRuntimeLuaValue"), TEXT("GetRuntimeLuaObject") };
	Out.Prerequisites = { TEXT("unlua"), TEXT("PIE") };
	Out.WhenToUse = TEXT("Browse all keys (env) or read single key (value)");
}

FCapabilityResult FNexusGetRuntimeLuaEnvCapability::Execute(const TSharedPtr<FJsonObject>& Arguments) const
{
	const FNexusArgs A(Arguments);
	FNexusMcpToolResult Tmp;
	lua_State* L = FNexusLuaUtils::GetMainLuaState(Tmp);
	if (!L)
		return FCapabilityResult::MakeFatal(Tmp.ErrorText);

	FCapabilityResult R;
	const int32 StackTop = lua_gettop(L);

	FString TablePath;
	TablePath = FNexusArgs(Arguments).Str(TEXT("LuaPath"), TablePath);

	if (TablePath.IsEmpty())
	{
		lua_pushglobaltable(L);
	}
	else if (!FNexusLuaUtils::ResolveLuaPath(L, TablePath))
	{
		lua_settop(L, StackTop);
		EmitError(R.Entries, {{TEXT("LuaPath"), TablePath}}, FString::Printf(TEXT("Path '%s' not found"), *TablePath));
		return R;
	}

	if (!lua_istable(L, -1))
	{
		lua_settop(L, StackTop);
		EmitError(R.Entries, {{TEXT("LuaPath"), TablePath}}, FString::Printf(TEXT("'%s' is not a table"), *TablePath));
		return R;
	}

	FString NameFilter;
	int32 Limit = 100;
	NameFilter = FNexusArgs(Arguments).Str(TEXT("NameFilter"), NameFilter);
	if (Arguments->HasField(TEXT("Limit")))
		Limit = FMath::Clamp(static_cast<int32>(A.Num(TEXT("Limit"))), 1, 500);

	const int32 AbsIdx = lua_gettop(L);
	TArray<TSharedPtr<FJsonValue>> Keys;
	int32 TotalCount = 0;

	lua_pushnil(L);
	while (FNexusLuaUtils::SafeLuaNext(L, AbsIdx) != 0)
	{
		FString Key;
		const int KT = lua_type(L, -2);
		if (KT == LUA_TSTRING)      Key = UTF8_TO_TCHAR(lua_tostring(L, -2));
		else if (KT == LUA_TNUMBER) Key = FString::Printf(TEXT("[%d]"), static_cast<int>(lua_tonumber(L, -2)));
		else                        Key = FString::Printf(TEXT("<%s>"), UTF8_TO_TCHAR(lua_typename(L, KT)));

		if (!NameFilter.IsEmpty() && !FNexusStringMatchUtils::Matches(Key, NameFilter))
		{
			lua_pop(L, 1);
			continue;
		}

		++TotalCount;
		if (Keys.Num() < Limit)
		{
			TSharedRef<FJsonObject> Entry = MakeShared<FJsonObject>();
			Entry->SetStringField(TEXT("Key"),  Key);
			Entry->SetStringField(TEXT("type"), UTF8_TO_TCHAR(lua_typename(L, lua_type(L, -1))));
			Keys.Add(MakeShared<FJsonValueObject>(Entry));
		}
		lua_pop(L, 1);
	}

	lua_settop(L, StackTop);

	TSharedPtr<FJsonObject> OutEntry = MakeShared<FJsonObject>();
	OutEntry->SetNumberField(TEXT("TotalCount"), TotalCount);
	OutEntry->SetArrayField(TEXT("Keys"), Keys);
	R.Entries.Add(MakeShared<FJsonValueObject>(OutEntry));
	return R;
}

REGISTER_MCP_CAPABILITY(FNexusGetRuntimeLuaEnvCapability)

#endif // WITH_UNLUA

