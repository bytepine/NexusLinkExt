// Copyright byteyang. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/** UnLua 蓝图绑定 Capability（仅编辑器）。cap 在 .cpp 里用 REGISTER_MCP_CAPABILITY 自注册。 */
class FNexusLinkExtEditor_UnLuaModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
