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
	Out.Description = TEXT("Resolve BP-bound UnLua module. Returns bBound/bFileExists/SearchedPaths. Uses the UnLua source root, then Content/Script.");
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

	// UnLua 启动时把脚本根写进 GLuaSrcFullPath（LetsGo 会改到 Content/Script 以外）。
	extern UNLUA_API FString GLuaSrcFullPath;
	const FString RelativeLua = ModuleName.Replace(TEXT("."), TEXT("/")) + TEXT(".lua");
	TArray<FString> Roots;
	if (!GLuaSrcFullPath.IsEmpty())
	{
		Roots.Add(GLuaSrcFullPath);
	}
	Roots.Add(FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Script")));

	TArray<TSharedPtr<FJsonValue>> Searched;
	FString LuaAbsPath;
	for (const FString& Root : Roots)
	{
		FString Candidate = Root;
		if (!Candidate.EndsWith(TEXT("/")) && !Candidate.EndsWith(TEXT("\\")))
		{
			Candidate += TEXT("/");
		}
		Candidate = FPaths::ConvertRelativePathToFull(Candidate + RelativeLua);
		bool bSeen = false;
		for (const TSharedPtr<FJsonValue>& Existing : Searched)
		{
			if (Existing.IsValid() && Existing->AsString().Equals(Candidate, ESearchCase::IgnoreCase))
			{
				bSeen = true;
				break;
			}
		}
		if (bSeen)
		{
			continue;
		}
		Searched.Add(MakeShared<FJsonValueString>(Candidate));
		if (LuaAbsPath.IsEmpty() && FPaths::FileExists(Candidate))
		{
			LuaAbsPath = Candidate;
		}
	}

	Entry->SetBoolField(TEXT("bBound"), true);
	Entry->SetStringField(TEXT("ModuleName"), ModuleName);
	Entry->SetArrayField(TEXT("SearchedPaths"), Searched);
	if (!LuaAbsPath.IsEmpty())
	{
		Entry->SetBoolField(TEXT("bFileExists"), true);
		Entry->SetStringField(TEXT("ExpectedLuaPath"), LuaAbsPath);
		Entry->SetStringField(TEXT("LuaFilePath"), LuaAbsPath);
	}
	else
	{
		Entry->SetBoolField(TEXT("bFileExists"), false);
		if (Searched.Num() > 0)
		{
			Entry->SetStringField(TEXT("ExpectedLuaPath"), Searched[0]->AsString());
		}
		Entry->SetStringField(TEXT("Hint"),
			TEXT("UnLua module registered but .lua was not found under the UnLua source root or Content/Script. See SearchedPaths."));
	}

	R.Entries.Add(MakeShared<FJsonValueObject>(Entry));
	return R;
}

REGISTER_MCP_CAPABILITY(FNexusGetAssetLuaBindingCapability)

#endif // WITH_UNLUA
