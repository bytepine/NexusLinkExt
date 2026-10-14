// Copyright byteyang. All Rights Reserved.

#include "Capabilities/Lua/Asset/NexusManageAssetLuaBindingCapability.h"

#if WITH_UNLUA

#include "NexusCapabilityRegistry.h"
#include "NexusMcpSchemaBuilder.h"
#include "Utils/NexusAssetUtils.h"
#include "Utils/NexusArgs.h"
#include "Utils/NexusVersionCompat.h"
#include "Engine/Blueprint.h"
#include "NexusMcpTool.h"
#if WITH_EDITOR
#include "Kismet2/BlueprintEditorUtils.h"
#if NX_UE_HAS_BP_INTERFACE_ASSET_PATH
#include "UObject/TopLevelAssetPath.h"
#endif
#endif

void FNexusManageAssetLuaBindingCapability::BuildDefinition(FNexusCapabilityDefinition& Out) const
{
	Out.Name = TEXT("ManageAssetLuaBinding");
	Out.Description = TEXT("Bind/unbind BP UnLua interface. Action=bind|unbind.");
	TSharedPtr<FJsonObject> OpSchema = FNexusSchema::Object()
		.Required(TEXT("Action"), FNexusSchema::Enum(TEXT("Action"), { TEXT("Bind"), TEXT("Unbind") }))
		.Prop(TEXT("ModuleName"), FNexusSchema::Str(TEXT("Lua module name (bind; optional, defaults from asset path)")))
		.Build();
	Out.InputSchema = FNexusSchema::Object()
		.Required(TEXT("AssetPath"), FNexusSchema::Str(TEXT("Blueprint asset path")))
		.Required(TEXT("Operations"), FNexusSchema::ArrayOf(TEXT("Operation list"), OpSchema.ToSharedRef()))
		.Build();
	Out.Tags = { FNexusMcpTags::Write, FNexusMcpTags::Blueprint };
	Out.ExtraSearchKeywords = { TEXT("unlua"), TEXT("binding"), TEXT("module") };
	Out.RelatedCapabilities = { TEXT("GetAssetLuaBinding"), TEXT("GetRuntimeLuaObject") };
	Out.Prerequisites = { TEXT("unlua"), TEXT("EditorOnly") };
	Out.WhenToUse = TEXT("Implement/remove UnLuaInterface on BP; do not set_* non-property");
}

#if WITH_EDITOR
struct FLuaBindingActionState
{
	UBlueprint* BP = nullptr;
	bool bDirty = false;
};

static FLuaBindingActionState* LuaBindState(FNexusActionContext& Ctx)
{
	return static_cast<FLuaBindingActionState*>(Ctx.Target);
}

static UBlueprint* LuaBindFrom(FNexusActionContext& Ctx)
{
	FLuaBindingActionState* S = LuaBindState(Ctx);
	return S ? S->BP : nullptr;
}

static void MarkLuaBindDirty(FNexusActionContext& Ctx)
{
	if (FLuaBindingActionState* S = LuaBindState(Ctx))
	{
		S->bDirty = true;
	}
}

#if !NX_UE_HAS_BP_INTERFACE_ASSET_PATH
static const FName UnLuaIfaceName(TEXT("UnLuaInterface"));
#endif

static void HandleLua_Bind(const TSharedPtr<FJsonObject>& Op, FNexusActionContext& Ctx)
{
	UBlueprint* BP = LuaBindFrom(Ctx);
	if (!BP)
	{
		Ctx.Entry->SetStringField(TEXT("Error"), TEXT("Blueprint is null"));
		return;
	}
#if NX_UE_HAS_BP_INTERFACE_ASSET_PATH
	FBlueprintEditorUtils::ImplementNewInterface(BP, FTopLevelAssetPath(TEXT("/Script/UnLua"), TEXT("UnLuaInterface")));
#else
	FBlueprintEditorUtils::ImplementNewInterface(BP, UnLuaIfaceName);
#endif
	FString ModuleName;
	Op->TryGetStringField(TEXT("ModuleName"), ModuleName);
	if (!ModuleName.IsEmpty()) Ctx.Entry->SetStringField(TEXT("ModuleName"), ModuleName);
	Ctx.Entry->SetStringField(TEXT("Note"), TEXT("UnLuaInterface requested; GetModuleName must return module in graph"));
	MarkLuaBindDirty(Ctx);
}

static void HandleLua_Unbind(const TSharedPtr<FJsonObject>& Op, FNexusActionContext& Ctx)
{
	(void)Op;
	UBlueprint* BP = LuaBindFrom(Ctx);
	if (!BP)
	{
		Ctx.Entry->SetStringField(TEXT("Error"), TEXT("Blueprint is null"));
		return;
	}
#if NX_UE_HAS_BP_INTERFACE_ASSET_PATH
	FBlueprintEditorUtils::RemoveInterface(BP, FTopLevelAssetPath(TEXT("/Script/UnLua"), TEXT("UnLuaInterface")));
#else
	FBlueprintEditorUtils::RemoveInterface(BP, UnLuaIfaceName);
#endif
	Ctx.Entry->SetBoolField(TEXT("bUnbound"), true);
	MarkLuaBindDirty(Ctx);
}
#endif // WITH_EDITOR

bool FNexusManageAssetLuaBindingCapability::PrepareTarget(
	const TSharedPtr<FJsonObject>& Args,
	TSharedPtr<FJsonObject>& Entry,
	void*& OutTarget,
	FString& OutError) const
{
#if !WITH_EDITOR
	(void)Args;
	(void)Entry;
	(void)OutTarget;
	OutError = TEXT("ManageAssetLuaBinding only available in editor builds");
	return false;
#else
	const FString AssetPath = FNexusArgs(Args).Str(TEXT("AssetPath"));
	Entry->SetStringField(TEXT("Path"), AssetPath);
	UBlueprint* BP = FNexusAssetUtils::LoadAssetWithFallback<UBlueprint>(AssetPath);
	if (!BP)
	{
		OutError = FString::Printf(TEXT("Blueprint not found: %s"), *AssetPath);
		return false;
	}
	FLuaBindingActionState* State = new FLuaBindingActionState();
	State->BP = BP;
	OutTarget = State;
	return true;
#endif
}

void FNexusManageAssetLuaBindingCapability::FinalizeTarget(void* Target) const
{
#if WITH_EDITOR
	FLuaBindingActionState* State = static_cast<FLuaBindingActionState*>(Target);
	if (!State) return;
	if (State->bDirty && State->BP)
	{
		State->BP->MarkPackageDirty();
	}
	delete State;
#else
	(void)Target;
#endif
}

void FNexusManageAssetLuaBindingCapability::RegisterActions(TMap<FString, FNexusActionHandler>& OutHandlers) const
{
#if WITH_EDITOR
	OutHandlers.Add(TEXT("Bind"),   &HandleLua_Bind);
	OutHandlers.Add(TEXT("Unbind"), &HandleLua_Unbind);
#else
	(void)OutHandlers;
#endif
}

REGISTER_MCP_CAPABILITY(FNexusManageAssetLuaBindingCapability)

#endif
