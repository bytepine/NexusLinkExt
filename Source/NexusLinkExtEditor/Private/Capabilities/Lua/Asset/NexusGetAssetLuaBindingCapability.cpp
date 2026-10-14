// Copyright byteyang. All Rights Reserved.

#include "Capabilities/Lua/Asset/NexusGetAssetLuaBindingCapability.h"

#if WITH_UNLUA

#include "NexusCapabilityRegistry.h"
#include "NexusMcpSchemaBuilder.h"
#include "Utils/NexusLuaUtils.h"
#include "Utils/NexusAssetUtils.h"
#include "Engine/Blueprint.h"
#include "Misc/Paths.h"

void FNexusGetAssetLuaBindingCapability::BuildDefinition(FNexusCapabilityDefinition& Out) const
{
	Out.Name = TEXT("GetAssetLuaBinding");
	Out.Description = TEXT("Resolve BP-bound UnLua module. Returns bound/bFileExists; requires UnLua.");
	Out.InputSchema = FNexusSchema::Object()
		.Required(TEXT("AssetPath"), FNexusSchema::Str(TEXT("Blueprint asset path")))
		.Build();
	Out.Tags = {FNexusMcpTags::Readonly, FNexusMcpTags::Blueprint };
	Out.ExtraSearchKeywords = { TEXT("unlua"), TEXT("module"), TEXT("mapping"), TEXT("filepath"), TEXT("script") };
	Out.RelatedCapabilities = { TEXT("GetRuntimeLuaObject"), TEXT("GetRuntimeLuaEnv"), TEXT("ManageAssetLuaBinding") };
	Out.Prerequisites = { TEXT("unlua"), TEXT("EditorOnly") };
	Out.WhenToUse = TEXT("Find bound file path before read/edit Lua");
}

FCapabilityResult FNexusGetAssetLuaBindingCapability::Execute(const TSharedPtr<FJsonObject>& Arguments) const
{
	FCapabilityResult R;

	FString AssetPath;
	if (!RequireString(Arguments, TEXT("AssetPath"), AssetPath, R.Entries))
		return R;

	UObject* Obj = FNexusAssetUtils::LoadAssetWithFallback<UObject>(AssetPath);
	if (!Obj)
		return FCapabilityResult::MakeFatal(FString::Printf(TEXT("Asset not found: %s"), *AssetPath));

	UBlueprint* BP = Cast<UBlueprint>(Obj);
	if (!BP || !BP->GeneratedClass)
		return FCapabilityResult::MakeFatal(TEXT("AssetPath must point to Blueprint asset"));

	TSharedPtr<FJsonObject> Entry = MakeShared<FJsonObject>();
	Entry->SetStringField(TEXT("Path"), AssetPath);
	if (BP->ParentClass)
		Entry->SetStringField(TEXT("ParentClass"), BP->ParentClass->GetName());

	const FString ModuleName = FNexusLuaUtils::GetUnLuaModuleName(BP->GeneratedClass);
	if (ModuleName.IsEmpty())
	{
		Entry->SetBoolField(TEXT("bBound"), false);
		Entry->SetStringField(TEXT("Hint"),
			TEXT("Blueprint has no UnLua binding; do not guess Lua paths. Use GetAssetBlueprint or runtime Lua caps instead."));
		R.Entries.Add(MakeShared<FJsonValueObject>(Entry));
		return R;
	}

	const FString LuaAbsPath = FPaths::Combine(
		FPaths::ProjectContentDir(), TEXT("Script"), ModuleName.Replace(TEXT("."), TEXT("/"))) + TEXT(".lua");

	Entry->SetBoolField(TEXT("bBound"), true);
	Entry->SetStringField(TEXT("ModuleName"), ModuleName);
	Entry->SetStringField(TEXT("ExpectedLuaPath"), LuaAbsPath);

	if (FPaths::FileExists(LuaAbsPath))
	{
		Entry->SetBoolField(TEXT("bFileExists"), true);
		Entry->SetStringField(TEXT("LuaFilePath"), LuaAbsPath);
	}
	else
	{
		Entry->SetBoolField(TEXT("bFileExists"), false);
		Entry->SetStringField(TEXT("Hint"),
			TEXT("UnLua module registered but .lua missing on disk; check Content/Script or create file."));
	}

	R.Entries.Add(MakeShared<FJsonValueObject>(Entry));
	return R;
}

REGISTER_MCP_CAPABILITY(FNexusGetAssetLuaBindingCapability)

#endif // WITH_UNLUA
