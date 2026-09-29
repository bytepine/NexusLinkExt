// Copyright byteyang. All Rights Reserved.

#include "NexusLinkExt.h"
#include "NexusInstructionRegistry.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

static FString LoadOwnInstructions(const TCHAR* FileName)
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("NexusLinkExt"));
	if (!Plugin.IsValid())
	{
		return FString();
	}
	const FString Path = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Resources"), FileName);
	FString Text;
	FFileHelper::LoadFileToString(Text, *Path);
	return Text;
}

void FNexusLinkExtModule::StartupModule()
{
	FNexusInstructionRegistry::Get().Register(
		TEXT("NexusLinkExt"),
		LoadOwnInstructions(TEXT("InitializeInstructions.SearchMode.md")),
		LoadOwnInstructions(TEXT("InitializeInstructions.MultiTool.md")));
}

void FNexusLinkExtModule::ShutdownModule()
{
	FNexusInstructionRegistry::Get().Unregister(TEXT("NexusLinkExt"));
}

IMPLEMENT_MODULE(FNexusLinkExtModule, NexusLinkExt)
