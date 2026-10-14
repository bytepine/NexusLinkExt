// Copyright byteyang. All Rights Reserved.

#include "Capabilities/Lua/Runtime/NexusSetRuntimeLuaCapability.h"

#if WITH_UNLUA

#include "NexusCapabilityRegistry.h"
#include "NexusMcpSchemaBuilder.h"
#include "Utils/NexusLuaUtils.h"
#include "NexusMcpTool.h"

void FNexusSetRuntimeLuaCapability::BuildDefinition(FNexusCapabilityDefinition& Out) const
{
	Out.Name = TEXT("SetRuntimeLua");
	Out.Description = TEXT("Assign Lua global or nested field. Dot path; string/number/bool/null.");
	Out.InputSchema = FNexusSchema::Object()
		.Required(TEXT("LuaPath"), FNexusSchema::Str(TEXT("Dot path target for set")))
		.Required(TEXT("Value"),   FNexusSchema::AnyScalar(TEXT("Value (string/number/boolean/null)")))
		.Build();
	Out.Tags = {FNexusMcpTags::Write, FNexusMcpTags::Runtime };
	Out.ExtraSearchKeywords = { TEXT("Value"), TEXT("variable"), TEXT("field"), TEXT("Path"), TEXT("assign") };
	Out.RelatedCapabilities = { TEXT("GetRuntimeLuaValue"), TEXT("EvalRuntimeLua") };
	Out.Prerequisites = { TEXT("unlua"), TEXT("PIE") };
}

FCapabilityResult FNexusSetRuntimeLuaCapability::Execute(const TSharedPtr<FJsonObject>& Arguments) const
{
	FNexusMcpToolResult Tmp;
	lua_State* L = FNexusLuaUtils::GetMainLuaState(Tmp);
	if (!L)
		return FCapabilityResult::MakeFatal(Tmp.ErrorText);

	FCapabilityResult R;

	FString Path;
	if (!RequireString(Arguments, TEXT("LuaPath"), Path, R.Entries))
		return R;
	if (!Arguments->HasField(TEXT("Value")))
	{
		EmitError(R.Entries, {{TEXT("LuaPath"), Path}}, TEXT("Missing value"));
		return R;
	}

	TSharedPtr<FJsonObject> Entry = MakeShared<FJsonObject>();
	Entry->SetStringField(TEXT("LuaPath"), Path);
	const int32 StackTop = lua_gettop(L);

	// 将 JSON value 压入 Lua 栈
	TSharedPtr<FJsonValue> JsonVal = Arguments->TryGetField(TEXT("Value"));
	if (!JsonVal.IsValid() || JsonVal->IsNull())
	{
		lua_pushnil(L);
	}
	else if (JsonVal->Type == EJson::Boolean)
	{
		lua_pushboolean(L, JsonVal->AsBool() ? 1 : 0);
	}
	else if (JsonVal->Type == EJson::Number)
	{
		lua_pushnumber(L, JsonVal->AsNumber());
	}
	else if (JsonVal->Type == EJson::String)
	{
		lua_pushstring(L, TCHAR_TO_UTF8(*JsonVal->AsString()));
	}
	else
	{
		lua_settop(L, StackTop);
		EmitError(R.Entries, {{TEXT("LuaPath"), Path}}, TEXT("value supports string/number/boolean/null only"));
		return R;
	}

	TArray<FString> Parts;
	Path.ParseIntoArray(Parts, TEXT("."));

	if (Parts.Num() == 1)
	{
		lua_setglobal(L, TCHAR_TO_UTF8(*Parts[0]));
	}
	else
	{
		lua_getglobal(L, TCHAR_TO_UTF8(*Parts[0]));
		if (!lua_istable(L, -1))
		{
			lua_settop(L, StackTop);
			EmitError(R.Entries, {{TEXT("LuaPath"), Path}},
				FString::Printf(TEXT("'%s' is not a table; cannot set sub-field"), *Parts[0]));
			return R;
		}

		for (int32 i = 1; i < Parts.Num() - 1; ++i)
		{
			lua_getfield(L, -1, TCHAR_TO_UTF8(*Parts[i]));
			if (!lua_istable(L, -1))
			{
				lua_settop(L, StackTop);
				EmitError(R.Entries, {{TEXT("LuaPath"), Path}},
					FString::Printf(TEXT("Intermediate path node '%s' is not a table"), *Parts[i]));
				return R;
			}
			lua_remove(L, -2);
		}

		// 栈顶是 parentTable，下面是先前压入的 value；复制 value 到栈顶后写回 parent
		const int32 ParentIdx = lua_gettop(L);
		lua_pushvalue(L, ParentIdx - 1);
		lua_setfield(L, ParentIdx, TCHAR_TO_UTF8(*Parts.Last()));
		lua_settop(L, StackTop);
	}

	R.Entries.Add(MakeShared<FJsonValueObject>(Entry));
	return R;
}

REGISTER_MCP_CAPABILITY(FNexusSetRuntimeLuaCapability)

#endif // WITH_UNLUA
