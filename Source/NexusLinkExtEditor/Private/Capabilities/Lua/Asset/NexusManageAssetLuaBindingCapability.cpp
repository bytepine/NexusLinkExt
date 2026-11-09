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
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_FunctionResult.h"
#if NX_UE_HAS_BP_INTERFACE_ASSET_PATH
#include "UObject/TopLevelAssetPath.h"
#endif
#endif

void FNexusManageAssetLuaBindingCapability::BuildDefinition(FNexusCapabilityDefinition& Out) const
{
	Out.Name = TEXT("ManageAssetLuaBinding");
	Out.bInjectCompile = true;
	Out.Description = TEXT("Bind/unbind BP UnLua interface and write GetModuleName. Compile explicitly, then verify with GetAssetLuaBinding.");
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
	Out.WhenToUse = TEXT("Bind a constant UnLua module; refuses linked/dynamic GetModuleName logic. Use bCompile=true and verify GetAssetLuaBinding.");
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
	FString ModuleName = FNexusArgs(Op).Str(TEXT("ModuleName"));
	if (ModuleName.IsEmpty())
	{
		ModuleName = BP->GetOutermost()->GetName();
		ModuleName.RemoveFromStart(TEXT("/Game/"));
		ModuleName.RemoveFromStart(TEXT("/"));
		ModuleName.ReplaceInline(TEXT("/"), TEXT("."));
	}
	if (ModuleName.StartsWith(TEXT(".")) || ModuleName.EndsWith(TEXT(".")) || ModuleName.Contains(TEXT(".."))
		|| ModuleName.Contains(TEXT("/")) || ModuleName.Contains(TEXT("\\")) || ModuleName.Contains(TEXT(":")))
	{
		Ctx.Entry->SetStringField(TEXT("Error"), TEXT("ModuleName must be a dotted Lua module name"));
		return;
	}
	UClass* Interface = LoadObject<UClass>(nullptr, TEXT("/Script/UnLua.UnLuaInterface"));
	if (!Interface) { Ctx.Entry->SetStringField(TEXT("Error"), TEXT("UnLuaInterface is unavailable")); return; }
	bool bOwnInterface = false;
	for (const FBPInterfaceDescription& Description : BP->ImplementedInterfaces)
		if (Description.Interface == Interface) bOwnInterface = true;
	if (!bOwnInterface && BP->ParentClass && BP->ParentClass->ImplementsInterface(Interface))
	{
		Ctx.Entry->SetStringField(TEXT("Error"), TEXT("Inherited UnLua binding requires an explicit GetModuleName override; parent logic was not changed"));
		return;
	}
	if (!bOwnInterface)
	{
		TArray<UEdGraph*> ExistingGraphs;
		BP->GetAllGraphs(ExistingGraphs);
		for (UEdGraph* Graph : ExistingGraphs)
			if (Graph && Graph->GetFName() == TEXT("GetModuleName"))
			{ Ctx.Entry->SetStringField(TEXT("Error"), TEXT("GetModuleName graph already exists without UnLuaInterface; no changes made")); return; }
		BP->Modify();
#if NX_UE_HAS_BP_INTERFACE_ASSET_PATH
		const bool bImplemented = FBlueprintEditorUtils::ImplementNewInterface(BP, FTopLevelAssetPath(TEXT("/Script/UnLua"), TEXT("UnLuaInterface")));
#else
		const bool bImplemented = FBlueprintEditorUtils::ImplementNewInterface(BP, UnLuaIfaceName);
#endif
		if (!bImplemented) { Ctx.Entry->SetStringField(TEXT("Error"), TEXT("Could not implement UnLuaInterface")); return; }
	}
	TArray<UEdGraph*> Graphs;
	BP->GetAllGraphs(Graphs);
	UEdGraph* ModuleGraph = nullptr;
	TArray<UEdGraphPin*> ReturnPins;
	for (UEdGraph* Graph : Graphs)
	{
		if (!Graph || Graph->GetFName() != TEXT("GetModuleName")) continue;
		ModuleGraph = Graph;
		for (UEdGraphNode* Node : Graph->Nodes)
			if (auto* Result = Cast<UK2Node_FunctionResult>(Node))
				if (UEdGraphPin* Pin = Result->FindPin(UEdGraphSchema_K2::PN_ReturnValue))
				{
					if (Pin->LinkedTo.Num() > 0)
					{ Ctx.Entry->SetStringField(TEXT("Error"), TEXT("GetModuleName return is linked; dynamic binding logic was not changed")); return; }
					ReturnPins.Add(Pin);
				}
	}
	if (!ModuleGraph || ReturnPins.Num() == 0 || !ModuleGraph->GetSchema())
	{ Ctx.Entry->SetStringField(TEXT("Error"), TEXT("GetModuleName has no writable return pin; binding is incomplete")); return; }
	BP->Modify();
	ModuleGraph->Modify();
	for (UEdGraphPin* Pin : ReturnPins)
	{
		Pin->GetOwningNode()->Modify();
		ModuleGraph->GetSchema()->TrySetDefaultValue(*Pin, ModuleName);
		if (Pin->DefaultValue != ModuleName)
		{ Ctx.Entry->SetStringField(TEXT("Error"), TEXT("GetModuleName return value verification failed")); return; }
	}
	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);
	Ctx.Entry->SetStringField(TEXT("ModuleName"), ModuleName);
	Ctx.Entry->SetBoolField(TEXT("bModuleNameWritten"), true);
	Ctx.Entry->SetStringField(TEXT("Note"), TEXT("Module name written and read back from return pins. Compile and verify with GetAssetLuaBinding; file existence is separate."));
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
