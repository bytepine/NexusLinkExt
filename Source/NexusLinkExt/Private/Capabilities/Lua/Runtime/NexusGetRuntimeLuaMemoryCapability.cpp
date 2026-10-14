// Copyright byteyang. All Rights Reserved.

#include "Capabilities/Lua/Runtime/NexusGetRuntimeLuaMemoryCapability.h"

#if WITH_UNLUA

#include "NexusCapabilityRegistry.h"
#include "NexusMcpSchemaBuilder.h"
#include "Utils/NexusLuaUtils.h"
#include "NexusMcpTool.h"

void FNexusGetRuntimeLuaMemoryCapability::BuildDefinition(FNexusCapabilityDefinition& Out) const
{
	Out.Name = TEXT("GetRuntimeLuaMemory");
	Out.Description = TEXT("Report Lua VM heap usage (KB/bytes). Use with GcRuntimeLua.");
	Out.InputSchema = FNexusSchema::EmptyObject();
	Out.Tags = {FNexusMcpTags::Readonly, FNexusMcpTags::Runtime };
	Out.ExtraSearchKeywords = { TEXT("heap"), TEXT("Usage"), TEXT("stats"), TEXT("ram"), TEXT("kb") };
	Out.RelatedCapabilities = { TEXT("GcRuntimeLua") };
	Out.Prerequisites = { TEXT("unlua"), TEXT("PIE") };
	Out.WhenToUse = TEXT("Check heap size before/after gc collect");
}

FCapabilityResult FNexusGetRuntimeLuaMemoryCapability::Execute(const TSharedPtr<FJsonObject>& Arguments) const
{
	FNexusMcpToolResult Tmp;
	lua_State* L = FNexusLuaUtils::GetMainLuaState(Tmp);
	if (!L)
		return FCapabilityResult::MakeFatal(Tmp.ErrorText);

	const int KBCount      = lua_gc(L, LUA_GCCOUNT,  0);
	const int ByteRemainder = lua_gc(L, LUA_GCCOUNTB, 0);

	FCapabilityResult R;
	TSharedPtr<FJsonObject> Entry = MakeShared<FJsonObject>();
	Entry->SetNumberField(TEXT("MemoryKB"),    KBCount);
	Entry->SetNumberField(TEXT("MemoryBytes"), KBCount * 1024 + ByteRemainder);
	R.Entries.Add(MakeShared<FJsonValueObject>(Entry));
	return R;
}

REGISTER_MCP_CAPABILITY(FNexusGetRuntimeLuaMemoryCapability)

#endif // WITH_UNLUA
