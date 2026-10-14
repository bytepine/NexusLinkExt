// Copyright byteyang. All Rights Reserved.

#include "Capabilities/Lua/Runtime/NexusGcRuntimeLuaCapability.h"
#include "Utils/NexusArgs.h"

#if WITH_UNLUA

#include "NexusCapabilityRegistry.h"
#include "NexusMcpSchemaBuilder.h"
#include "Utils/NexusLuaUtils.h"
#include "NexusMcpTool.h"

void FNexusGcRuntimeLuaCapability::BuildDefinition(FNexusCapabilityDefinition& Out) const
{
	Out.Name = TEXT("GcRuntimeLua");
	Out.Description = TEXT("Control Lua GC in PIE. Mode=collect|stop|restart|count.");
	Out.InputSchema = FNexusSchema::Object()
		.Prop(TEXT("Mode"), FNexusSchema::Enum(TEXT("GC mode"),
			{ TEXT("Collect"), TEXT("Stop"), TEXT("Restart"), TEXT("Count") }, TEXT("Collect")))
		.Build();
	Out.Tags = {FNexusMcpTags::Write, FNexusMcpTags::Runtime };
	Out.ExtraSearchKeywords = { TEXT("Collect"), TEXT("garbage"), TEXT("memory"), TEXT("cleanup"), TEXT("reclaim") };
	Out.RelatedCapabilities = { TEXT("GetRuntimeLuaMemory") };
	Out.Prerequisites = { TEXT("unlua"), TEXT("PIE") };
}

FCapabilityResult FNexusGcRuntimeLuaCapability::Execute(const TSharedPtr<FJsonObject>& Arguments) const
{
	const FNexusArgs A(Arguments);
	FNexusMcpToolResult Tmp;
	lua_State* L = FNexusLuaUtils::GetMainLuaState(Tmp);
	if (!L)
		return FCapabilityResult::MakeFatal(Tmp.ErrorText);

	FString Mode = TEXT("Collect");
	if (Arguments->HasField(TEXT("Mode")))
		Mode = A.Str(TEXT("Mode")).ToLower();

	FCapabilityResult R;
	TSharedPtr<FJsonObject> Entry = MakeShared<FJsonObject>();

	const int32 MemBeforeKB = lua_gc(L, LUA_GCCOUNT,  0);
	const int32 MemBeforeB  = lua_gc(L, LUA_GCCOUNTB, 0);
	const int32 BeforeBytes = MemBeforeKB * 1024 + MemBeforeB;

	if (Mode == TEXT("Collect"))
	{
		lua_gc(L, LUA_GCCOLLECT, 0);
		const int32 MemAfterKB = lua_gc(L, LUA_GCCOUNT,  0);
		const int32 MemAfterB  = lua_gc(L, LUA_GCCOUNTB, 0);
		Entry->SetStringField(TEXT("Mode"),          TEXT("Collect"));
		Entry->SetNumberField(TEXT("MemoryBeforeKB"), MemBeforeKB);
		Entry->SetNumberField(TEXT("MemoryAfterKB"),  MemAfterKB);
		Entry->SetNumberField(TEXT("FreedBytes"),     BeforeBytes - (MemAfterKB * 1024 + MemAfterB));
	}
	else if (Mode == TEXT("Stop"))
	{
		lua_gc(L, LUA_GCSTOP, 0);
		Entry->SetStringField(TEXT("Mode"),      TEXT("Stop"));
		Entry->SetNumberField(TEXT("MemoryKB"),  MemBeforeKB);
	}
	else if (Mode == TEXT("Restart"))
	{
		lua_gc(L, LUA_GCRESTART, 0);
		Entry->SetStringField(TEXT("Mode"),      TEXT("Restart"));
		Entry->SetNumberField(TEXT("MemoryKB"),  MemBeforeKB);
	}
	else if (Mode == TEXT("Count"))
	{
		Entry->SetStringField(TEXT("Mode"),        TEXT("Count"));
		Entry->SetNumberField(TEXT("MemoryKB"),    MemBeforeKB);
		Entry->SetNumberField(TEXT("MemoryBytes"), BeforeBytes);
	}
	else
	{
		EmitError(R.Entries, {}, FString::Printf(
			TEXT("Unsupported mode: %s (expected collect/stop/restart/count)"), *Mode));
		return R;
	}

	R.Entries.Add(MakeShared<FJsonValueObject>(Entry));
	return R;
}

REGISTER_MCP_CAPABILITY(FNexusGcRuntimeLuaCapability)

#endif // WITH_UNLUA
