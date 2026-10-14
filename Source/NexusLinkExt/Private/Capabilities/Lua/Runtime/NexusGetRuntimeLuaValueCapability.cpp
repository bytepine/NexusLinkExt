// Copyright byteyang. All Rights Reserved.

#include "Capabilities/Lua/Runtime/NexusGetRuntimeLuaValueCapability.h"

#if WITH_UNLUA

#include "NexusCapabilityRegistry.h"
#include "NexusMcpSchemaBuilder.h"
#include "Utils/NexusLuaUtils.h"
#include "NexusMcpTool.h"

void FNexusGetRuntimeLuaValueCapability::BuildDefinition(FNexusCapabilityDefinition& Out) const
{
	Out.Name = TEXT("GetRuntimeLuaValue");
	Out.Description = TEXT("Read Lua global or nested field by dot path. Returns type and value.");
	Out.InputSchema = FNexusSchema::Object()
		.Required(TEXT("LuaPath"), FNexusSchema::Str(TEXT("Lua dot-separated path")))
		.Build();
	Out.Tags = {FNexusMcpTags::Readonly, FNexusMcpTags::Runtime };
	Out.ExtraSearchKeywords = { TEXT("variable"), TEXT("field"), TEXT("Path"), TEXT("global"), TEXT("dot") };
	Out.RelatedCapabilities = { TEXT("SetRuntimeLua"), TEXT("GetRuntimeLuaEnv") };
	Out.Prerequisites = { TEXT("unlua"), TEXT("PIE") };
	Out.WhenToUse = TEXT("Read single key here; browse keys with env");
}

FCapabilityResult FNexusGetRuntimeLuaValueCapability::Execute(const TSharedPtr<FJsonObject>& Arguments) const
{
	FNexusMcpToolResult Tmp;
	lua_State* L = FNexusLuaUtils::GetMainLuaState(Tmp);
	if (!L)
		return FCapabilityResult::MakeFatal(Tmp.ErrorText);

	FCapabilityResult R;

	FString Path;
	if (!RequireString(Arguments, TEXT("LuaPath"), Path, R.Entries))
		return R;

	const int32 StackTop = lua_gettop(L);
	TSharedPtr<FJsonObject> Entry = MakeShared<FJsonObject>();
	Entry->SetStringField(TEXT("LuaPath"), Path);

	if (!FNexusLuaUtils::ResolveLuaPath(L, Path))
	{
		lua_settop(L, StackTop);
		EmitError(R.Entries, {{TEXT("LuaPath"), Path}},
			FString::Printf(TEXT("Path '%s' not found (nil or non-table intermediate)"), *Path));
		return R;
	}

	Entry->SetStringField(TEXT("type"), UTF8_TO_TCHAR(lua_typename(L, lua_type(L, -1))));
	TSharedPtr<FJsonValue> Val = FNexusLuaUtils::LuaValueToJson(L, -1, 0);
	Entry->SetField(TEXT("Value"), Val.IsValid() ? Val : MakeShared<FJsonValueNull>());

	lua_settop(L, StackTop);
	R.Entries.Add(MakeShared<FJsonValueObject>(Entry));
	return R;
}

REGISTER_MCP_CAPABILITY(FNexusGetRuntimeLuaValueCapability)

#endif // WITH_UNLUA
