// Copyright byteyang. All Rights Reserved.

#include "Capabilities/Lua/Runtime/NexusEvalRuntimeLuaCapability.h"

#if WITH_UNLUA

#include "NexusCapabilityRegistry.h"
#include "NexusMcpSchemaBuilder.h"
#include "Utils/NexusLuaUtils.h"
#include "NexusMcpTool.h"

void FNexusEvalRuntimeLuaCapability::BuildDefinition(FNexusCapabilityDefinition& Out) const
{
	Out.Name = TEXT("EvalRuntimeLua");
	Out.Description = TEXT("Execute Lua snippet in PIE/Game; returns stacked values.");
	Out.InputSchema = FNexusSchema::Object()
		.Prop(TEXT("Code"), FNexusSchema::Str(TEXT("Lua expression or code block")))
		.Prop(TEXT("Reason"), FNexusSchema::Str(TEXT("Purpose for the editor confirm dialog. Required in Confirm mode.")))
		.Required({ TEXT("Code") })
		.Build();
	Out.Tags = {FNexusMcpTags::Write, FNexusMcpTags::Runtime, FNexusMcpTags::Dangerous };
	Out.ExtraSearchKeywords = { TEXT("script"), TEXT("Code"), TEXT("expression"), TEXT("repl"), TEXT("snippet") };
	Out.RelatedCapabilities = { TEXT("SetRuntimeLua"), TEXT("GetRuntimeLuaValue") };
	Out.Prerequisites = { TEXT("unlua"), TEXT("PIE") };
}

FCapabilityResult FNexusEvalRuntimeLuaCapability::Execute(const TSharedPtr<FJsonObject>& Arguments) const
{
	FNexusMcpToolResult Tmp;
	lua_State* L = FNexusLuaUtils::GetMainLuaState(Tmp);
	if (!L)
		return FCapabilityResult::MakeFatal(Tmp.ErrorText);

	FCapabilityResult R;

	FString Code;
	if (!RequireString(Arguments, TEXT("Code"), Code, R.Entries))
		return R;

	TSharedPtr<FJsonObject> Entry = MakeShared<FJsonObject>();
	const int32 StackTop = lua_gettop(L);

	// Best-effort：预注入 UE 全局，允许 AI 在裸 Lua 全局下直接写 UE.* 一行诊断。
	static const char* Bootstrap =
		"if rawget(_G, 'UE') == nil then\n"
		"  local ok, M = pcall(require, 'UnLua')\n"
		"  if ok and type(M) == 'table' and M.UE then _G.UE = M.UE end\n"
		"end";
	luaL_dostring(L, Bootstrap);
	lua_settop(L, StackTop);

	// 先尝试 "return <code>" 以获取返回值；失败则直接执行原始代码
	FString ExprCode = FString::Printf(TEXT("return %s"), *Code);
	int32 LoadResult = luaL_loadstring(L, TCHAR_TO_UTF8(*ExprCode));
	if (LoadResult != 0)
	{
		lua_pop(L, 1);
		LoadResult = luaL_loadstring(L, TCHAR_TO_UTF8(*Code));
		if (LoadResult != 0)
		{
			FString ErrMsg = UTF8_TO_TCHAR(lua_tostring(L, -1));
			lua_settop(L, StackTop);
			EmitError(R.Entries, {}, FString::Printf(TEXT("Lua parse error: %s"), *ErrMsg));
			return R;
		}
	}

	if (lua_pcall(L, 0, LUA_MULTRET, 0) != 0)
	{
		FString ErrMsg = UTF8_TO_TCHAR(lua_tostring(L, -1));
		lua_settop(L, StackTop);
		EmitError(R.Entries, {}, FString::Printf(TEXT("Lua eval error: %s"), *ErrMsg));
		return R;
	}

	const int32 NumResults = lua_gettop(L) - StackTop;

	if (NumResults == 1)
	{
		Entry->SetStringField(TEXT("type"), UTF8_TO_TCHAR(lua_typename(L, lua_type(L, -1))));
		TSharedPtr<FJsonValue> Val = FNexusLuaUtils::LuaValueToJson(L, -1, 0);
		Entry->SetField(TEXT("Value"), Val.IsValid() ? Val : MakeShared<FJsonValueNull>());
	}
	else if (NumResults > 1)
	{
		TArray<TSharedPtr<FJsonValue>> Values;
		for (int32 i = StackTop + 1; i <= lua_gettop(L); ++i)
		{
			TSharedPtr<FJsonValue> Val = FNexusLuaUtils::LuaValueToJson(L, i, 0);
			Values.Add(Val.IsValid() ? Val : MakeShared<FJsonValueNull>());
		}
		Entry->SetArrayField(TEXT("Values"), Values);
	}

	lua_settop(L, StackTop);
	R.Entries.Add(MakeShared<FJsonValueObject>(Entry));
	return R;
}

REGISTER_MCP_CAPABILITY(FNexusEvalRuntimeLuaCapability)

#endif // WITH_UNLUA
