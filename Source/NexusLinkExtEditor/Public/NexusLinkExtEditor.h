// Copyright byteyang. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/** 可选插件的编辑器 Capability。cap 在 .cpp 里用 REGISTER_MCP_CAPABILITY 自注册。 */
class FNexusLinkExtEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
