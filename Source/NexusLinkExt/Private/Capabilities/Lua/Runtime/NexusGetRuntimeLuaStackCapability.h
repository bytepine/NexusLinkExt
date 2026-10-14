// Copyright byteyang. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if WITH_UNLUA

#include "NexusCapability.h"
#include "NexusRuntimeCapability.h"

struct lua_State;

/** GetRuntimeLuaStack — 检查 Lua 调用栈帧。 */
class FNexusGetRuntimeLuaStackCapability : public FNexusRuntimeCapability
{
protected:
	virtual void BuildDefinition(FNexusCapabilityDefinition& Out) const override;
	virtual FCapabilityResult Execute(const TSharedPtr<FJsonObject>& Arguments) const override;
};

#endif // WITH_UNLUA
