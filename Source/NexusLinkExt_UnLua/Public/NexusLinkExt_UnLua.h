// Copyright byteyang. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/** UnLua 运行时 Capability 模块。cap 在 .cpp 里用 REGISTER_MCP_CAPABILITY 自注册。 */
class FNexusLinkExt_UnLuaModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
