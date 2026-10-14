// Copyright byteyang. All Rights Reserved.

#include "Capabilities/Lua/Runtime/NexusGetRuntimeLuaObjectCapability.h"
#include "Utils/NexusArgs.h"

#if WITH_UNLUA

#include "NexusCapabilityRegistry.h"
#include "NexusMcpSchemaBuilder.h"
#include "Utils/NexusLuaUtils.h"
#include "Utils/NexusRuntimeUtils.h"
#include "GameFramework/Actor.h"
#include "NexusMcpTool.h"

// UnLua 在 LUA_REGISTRYINDEX 中以 lightuserdata(UObject*) 为 key 关联 userdata wrapper；
// 此处尝试取出其 uservalue（per-instance Lua 表）并压栈。
static bool TryPushObjectLuaTable(lua_State* L, UObject* Object)
{
	const int32 StackTop = lua_gettop(L);

	lua_pushlightuserdata(L, static_cast<void*>(Object));
	lua_rawget(L, LUA_REGISTRYINDEX);

	if (lua_isuserdata(L, -1))
	{
#if LUA_VERSION_NUM >= 502
		lua_getuservalue(L, -1);
#else
		lua_getfenv(L, -1);
#endif
		if (lua_istable(L, -1))
		{
			lua_remove(L, -2);
			return true;
		}
	}

	lua_settop(L, StackTop);
	return false;
}

void FNexusGetRuntimeLuaObjectCapability::BuildDefinition(FNexusCapabilityDefinition& Out) const
{
	Out.Name = TEXT("GetRuntimeLuaObject");
	Out.Description = TEXT("Read UnLua-bound Actor/UObject instance Lua table.");
	Out.InputSchema = FNexusSchema::Object()
		.Required(TEXT("ActorName"), FNexusSchema::Str(TEXT("Runtime Actor name")))
		.Prop(TEXT("LuaPath"),       FNexusSchema::Str(TEXT("Dot-separated sub-path in Lua table")))
		.Prop(TEXT("NameFilter"),    FNexusSchema::Str(TEXT("Key filter; /regex/, ^prefix, suffix$")))
		.Prop(TEXT("Limit"),         FNexusSchema::Int(TEXT("Max returned keys"), 100, 1, 500))
		.Build();
	Out.Tags = {FNexusMcpTags::Readonly, FNexusMcpTags::Runtime };
	Out.ExtraSearchKeywords = { TEXT("instance"), TEXT("Actor"), TEXT("Fields"), TEXT("self"), TEXT("Data") };
	Out.RelatedCapabilities = { TEXT("GetRuntimeLuaEnv"), TEXT("GetAssetLuaBinding") };
	Out.Prerequisites = { TEXT("unlua"), TEXT("PIE") };
}

FCapabilityResult FNexusGetRuntimeLuaObjectCapability::Execute(const TSharedPtr<FJsonObject>& Arguments) const
{
	const FNexusArgs A(Arguments);
	FNexusMcpToolResult Tmp;
	lua_State* L = FNexusLuaUtils::GetMainLuaState(Tmp);
	if (!L)
		return FCapabilityResult::MakeFatal(Tmp.ErrorText);

	FCapabilityResult R;

	FString ActorName;
	if (!RequireString(Arguments, TEXT("ActorName"), ActorName, R.Entries))
		return R;

	TSharedPtr<FJsonObject> Entry = MakeShared<FJsonObject>();

	UWorld* World = FNexusRuntimeUtils::GetActiveWorld();
	if (!World) { EmitError(R.Entries, {{TEXT("ActorName"), ActorName}}, TEXT("No active World")); return R; }
	AActor* Actor = FNexusRuntimeUtils::FindActorByName(World, ActorName);
	if (!Actor) { EmitError(R.Entries, {{TEXT("ActorName"), ActorName}}, TEXT("Actor not found")); return R; }

	FString ModuleName = FNexusLuaUtils::GetUnLuaModuleName(Actor->GetClass());
	if (ModuleName.IsEmpty())
	{
		EmitError(R.Entries, {{TEXT("ActorName"), ActorName}}, TEXT("Actor has no UnLua binding"));
		return R;
	}

	Entry->SetStringField(TEXT("ActorName"),  ActorName);
	Entry->SetStringField(TEXT("LuaModule"),  ModuleName);

	FString Path, NameFilter;
	int32 Limit = 100;
	Path = FNexusArgs(Arguments).Str(TEXT("LuaPath"), Path);
	NameFilter = FNexusArgs(Arguments).Str(TEXT("NameFilter"), NameFilter);
	if (Arguments->HasField(TEXT("Limit")))
		Limit = FMath::Clamp(static_cast<int32>(A.Num(TEXT("Limit"))), 1, 500);

	const int32 StackTop = lua_gettop(L);

	const bool bUsingInstanceTable = TryPushObjectLuaTable(L, Actor);
	if (!bUsingInstanceTable)
	{
		if (!FNexusLuaUtils::PushLoadedModule(L, ModuleName))
		{
			lua_settop(L, StackTop);
			EmitError(R.Entries, {{TEXT("ActorName"), ActorName}},
				TEXT("Unable to resolve object Lua table (instance and class tables unavailable)"));
			return R;
		}
		Entry->SetStringField(TEXT("TableSource"), TEXT("classTable"));
	}
	else
	{
		Entry->SetStringField(TEXT("TableSource"), TEXT("instanceTable"));
	}

	if (!Path.IsEmpty())
	{
		TArray<FString> Parts;
		Path.ParseIntoArray(Parts, TEXT("."));
		FString FailedPart;
		bool bResolved = true;
		for (const FString& Part : Parts)
		{
			if (!lua_istable(L, -1)) { FailedPart = Part; bResolved = false; break; }
			lua_getfield(L, -1, TCHAR_TO_UTF8(*Part));
			if (lua_isnil(L, -1)) { FailedPart = Part; bResolved = false; break; }
		}

		if (bResolved)
		{
			Entry->SetStringField(TEXT("type"), UTF8_TO_TCHAR(lua_typename(L, lua_type(L, -1))));
			TSharedPtr<FJsonValue> Val = FNexusLuaUtils::LuaValueToJson(L, -1, 0);
			Entry->SetField(TEXT("Value"), Val.IsValid() ? Val : MakeShared<FJsonValueNull>());
		}
		else
		{
			Entry->SetBoolField(TEXT("bNotFound"), true);
			Entry->SetStringField(TEXT("FailedAt"), FailedPart);
		}
	}
	else
	{
		TArray<TSharedPtr<FJsonValue>> Keys;
		int32 TotalCount = 0;
		FNexusLuaUtils::CollectTableKeys(L, -1, NameFilter, Limit, Keys, TotalCount);
		Entry->SetNumberField(TEXT("KeyCount"), TotalCount);
		Entry->SetArrayField(TEXT("Keys"), Keys);
	}

	lua_settop(L, StackTop);
	R.Entries.Add(MakeShared<FJsonValueObject>(Entry));
	return R;
}

REGISTER_MCP_CAPABILITY(FNexusGetRuntimeLuaObjectCapability)

#endif // WITH_UNLUA
