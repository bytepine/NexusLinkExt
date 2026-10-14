// Copyright byteyang. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if WITH_UNLUA

#include "NexusCapability.h"
#include "NexusRuntimeCapability.h"

struct lua_State;

/** GetRuntimeLuaEnv — 列出 _G 或指定 table 的 keys。 */
class FNexusGetRuntimeLuaEnvCapability : public FNexusRuntimeCapability
{
protected:
	virtual void BuildDefinition(FNexusCapabilityDefinition& Out) const override;
	virtual FCapabilityResult Execute(const TSharedPtr<FJsonObject>& Arguments) const override;
};

#endif // WITH_UNLUA
