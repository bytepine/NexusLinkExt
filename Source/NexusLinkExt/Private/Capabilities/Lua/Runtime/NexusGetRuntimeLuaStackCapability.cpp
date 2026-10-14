// Copyright byteyang. All Rights Reserved.

#include "Capabilities/Lua/Runtime/NexusGetRuntimeLuaStackCapability.h"
#include "Utils/NexusArgs.h"

#if WITH_UNLUA

#include "NexusCapabilityRegistry.h"
#include "NexusMcpSchemaBuilder.h"
#include "Utils/NexusLuaUtils.h"
#include "Utils/NexusStringMatchUtils.h"
#include "NexusMcpTool.h"

// ── 调用栈遍历的局部 helpers ──

static void CollectLocals(lua_State* L, lua_Debug& Ar, TSharedRef<FJsonObject>& Frame)
{
	TArray<TSharedPtr<FJsonValue>> Locals;
	const char* LocalName;
	int32 LocalIdx = 1;
	while ((LocalName = lua_getlocal(L, &Ar, LocalIdx)) != nullptr)
	{
		if (LocalName[0] != '(')
		{
			TSharedRef<FJsonObject> Var = MakeShared<FJsonObject>();
			Var->SetStringField(TEXT("name"), UTF8_TO_TCHAR(LocalName));
		const int32 Type = lua_type(L, -1);
		Var->SetStringField(TEXT("type"), UTF8_TO_TCHAR(lua_typename(L, Type)));
		switch (Type)
		{
		case LUA_TSTRING:  Var->SetStringField(TEXT("Value"), UTF8_TO_TCHAR(lua_tostring(L, -1))); break;
		case LUA_TNUMBER:  Var->SetNumberField(TEXT("Value"), lua_tonumber(L, -1)); break;
		case LUA_TBOOLEAN: Var->SetBoolField(TEXT("Value"), lua_toboolean(L, -1) != 0); break;
		case LUA_TNIL:     Var->SetStringField(TEXT("Value"), TEXT("nil")); break;
		default:
			Var->SetStringField(TEXT("Value"),
				FString::Printf(TEXT("<%s: %p>"), UTF8_TO_TCHAR(lua_typename(L, Type)), lua_topointer(L, -1)));
			break;
		}
		Locals.Add(MakeShared<FJsonValueObject>(Var));
		}
		lua_pop(L, 1);
		++LocalIdx;
	}
	if (Locals.Num() > 0) { Frame->SetArrayField(TEXT("Locals"), Locals); }
}

static void CollectUpvalues(lua_State* L, lua_Debug& Ar, TSharedRef<FJsonObject>& Frame)
{
	if (!lua_getinfo(L, "f", &Ar)) { return; }

	TArray<TSharedPtr<FJsonValue>> Upvalues;
	int32 UpIdx = 1;
	const char* UpName;
	while ((UpName = lua_getupvalue(L, -1, UpIdx)) != nullptr)
	{
		TSharedRef<FJsonObject> Var = MakeShared<FJsonObject>();
		Var->SetStringField(TEXT("name"), UTF8_TO_TCHAR(UpName));
		const int32 Type = lua_type(L, -1);
		Var->SetStringField(TEXT("type"), UTF8_TO_TCHAR(lua_typename(L, Type)));
		switch (Type)
		{
		case LUA_TSTRING:  Var->SetStringField(TEXT("Value"), UTF8_TO_TCHAR(lua_tostring(L, -1))); break;
		case LUA_TNUMBER:  Var->SetNumberField(TEXT("Value"), lua_tonumber(L, -1)); break;
		case LUA_TBOOLEAN: Var->SetBoolField(TEXT("Value"), lua_toboolean(L, -1) != 0); break;
		case LUA_TNIL:     Var->SetStringField(TEXT("Value"), TEXT("nil")); break;
		default:
			Var->SetStringField(TEXT("Value"),
				FString::Printf(TEXT("<%s: %p>"), UTF8_TO_TCHAR(lua_typename(L, Type)), lua_topointer(L, -1)));
			break;
		}
		lua_pop(L, 1);
		Upvalues.Add(MakeShared<FJsonValueObject>(Var));
		++UpIdx;
	}
	lua_pop(L, 1);

	if (Upvalues.Num() > 0) { Frame->SetArrayField(TEXT("Upvalues"), Upvalues); }
}

// ── Capability 实现 ──

void FNexusGetRuntimeLuaStackCapability::BuildDefinition(FNexusCapabilityDefinition& Out) const
{
	Out.Name = TEXT("GetRuntimeLuaStack");
	Out.Description = TEXT("Dump Lua call stack and locals/upvalues. Detail=locals|upvalues|all.");
	Out.InputSchema = [this]() -> TSharedPtr<FJsonObject>
	{
		TSharedRef<FJsonObject> FrameIdxItem = MakeShared<FJsonObject>();
		FrameIdxItem->SetStringField(TEXT("type"), TEXT("number"));

		return FNexusSchema::Object()
		.Prop(TEXT("Detail"),       FNexusSchema::Enum(TEXT("Stack frame detail"),
		{ TEXT("Summary"), TEXT("Locals"), TEXT("Upvalues"), TEXT("All") }, TEXT("Summary")))
		.Prop(TEXT("FrameIndex"),   FNexusSchema::Int(TEXT("Single stack frame to drill into")))
		.Prop(TEXT("FrameIndices"), FNexusSchema::ArrayOf(TEXT("Multiple stack frames to drill into"), FrameIdxItem))
		.Prop(TEXT("SourceFilter"), FNexusSchema::Str(TEXT("Stack frame source path filter")))
		.Prop(TEXT("MaxDepth"),     FNexusSchema::Int(TEXT("Max stack frames"), 50, 1, 500))
		.Build();
	}();
	Out.Tags = {FNexusMcpTags::Readonly, FNexusMcpTags::Runtime };
	Out.ExtraSearchKeywords = { TEXT("traceback"), TEXT("trace"), TEXT("frame"), TEXT("debug"), TEXT("callstack") };
	Out.RelatedCapabilities = { TEXT("EvalRuntimeLua") };
	Out.Prerequisites = { TEXT("unlua"), TEXT("PIE") };
}

FCapabilityResult FNexusGetRuntimeLuaStackCapability::Execute(const TSharedPtr<FJsonObject>& Arguments) const
{
	const FNexusArgs A(Arguments);
	FNexusMcpToolResult Tmp;
	lua_State* L = FNexusLuaUtils::GetMainLuaState(Tmp);
	if (!L)
		return FCapabilityResult::MakeFatal(Tmp.ErrorText);

	FString Detail = TEXT("Summary");
	FString SourceFilter;
	int32 MaxDepth = 50;
	TSet<int32> DrillFrameSet;

	if (Arguments->HasField(TEXT("Detail")))
		Detail = A.Str(TEXT("Detail")).ToLower();
	if (Arguments->HasField(TEXT("SourceFilter")))
		SourceFilter = A.Str(TEXT("SourceFilter"));
	if (Arguments->HasField(TEXT("MaxDepth")))
		MaxDepth = FMath::Clamp(static_cast<int32>(A.Num(TEXT("MaxDepth"))), 1, 500);

	const TArray<TSharedPtr<FJsonValue>>* IndicesArr = nullptr;
	if (Arguments->TryGetArrayField(TEXT("FrameIndices"), IndicesArr))
	{
		for (const TSharedPtr<FJsonValue>& Val : *IndicesArr)
			DrillFrameSet.Add(static_cast<int32>(Val->AsNumber()));
	}
	else if (Arguments->HasField(TEXT("FrameIndex")))
	{
		DrillFrameSet.Add(static_cast<int32>(A.Num(TEXT("FrameIndex"))));
	}

	const bool bDrillMode    = DrillFrameSet.Num() > 0;
	const bool bWantLocals   = bDrillMode || Detail == TEXT("Locals")   || Detail == TEXT("All");
	const bool bWantUpvalues = bDrillMode || Detail == TEXT("Upvalues") || Detail == TEXT("All");

	TArray<TSharedPtr<FJsonValue>> Frames;
	lua_Debug Ar;
	int32 Level = 0, TotalDepth = 0;

	while (Level < MaxDepth && lua_getstack(L, Level, &Ar))
	{
		lua_getinfo(L, "nSltu", &Ar);
		++TotalDepth;

		FString Source = Ar.source ? FString(UTF8_TO_TCHAR(Ar.source)) : FString();

		if (!SourceFilter.IsEmpty() && !FNexusStringMatchUtils::Matches(Source, SourceFilter))
		{
			++Level;
			continue;
		}
		if (bDrillMode && !DrillFrameSet.Contains(Level))
		{
			++Level;
			continue;
		}

		TSharedRef<FJsonObject> Frame = MakeShared<FJsonObject>();
		Frame->SetNumberField(TEXT("Level"), Level);
		if (!Source.IsEmpty())                    Frame->SetStringField(TEXT("Source"),       Source);
		if (Ar.short_src[0] != '\0')              Frame->SetStringField(TEXT("ShortSource"),  UTF8_TO_TCHAR(Ar.short_src));
		if (Ar.currentline > 0)                   Frame->SetNumberField(TEXT("CurrentLine"),  Ar.currentline);
		if (Ar.linedefined > 0)                   Frame->SetNumberField(TEXT("LineDefined"),  Ar.linedefined);
		if (Ar.lastlinedefined > 0)               Frame->SetNumberField(TEXT("LastLineDefined"), Ar.lastlinedefined);
		if (Ar.name)                              Frame->SetStringField(TEXT("name"),         UTF8_TO_TCHAR(Ar.name));
		if (Ar.namewhat && Ar.namewhat[0] != '\0') Frame->SetStringField(TEXT("NameWhat"),    UTF8_TO_TCHAR(Ar.namewhat));
		if (Ar.what)                              Frame->SetStringField(TEXT("What"),         UTF8_TO_TCHAR(Ar.what));

		if (bWantLocals)   CollectLocals(L, Ar, Frame);
		if (bWantUpvalues) CollectUpvalues(L, Ar, Frame);

		Frames.Add(MakeShared<FJsonValueObject>(Frame));
		++Level;
	}
	while (lua_getstack(L, Level, &Ar)) { ++TotalDepth; ++Level; }

	FCapabilityResult R;
	TSharedPtr<FJsonObject> OutEntry = MakeShared<FJsonObject>();
	OutEntry->SetNumberField(TEXT("Depth"),  TotalDepth);
	OutEntry->SetArrayField(TEXT("Frames"), Frames);
	R.Entries.Add(MakeShared<FJsonValueObject>(OutEntry));
	return R;
}

REGISTER_MCP_CAPABILITY(FNexusGetRuntimeLuaStackCapability)

#endif // WITH_UNLUA

