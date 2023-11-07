// Free to use WIP Blueprint Events subsystem, Alessandro Profera 07/11/2023

#pragma once

#include <CoreMinimal.h>
#include <Subsystems/GameInstanceSubsystem.h>
#include <GameplayTagContainer.h>

#include "EventsSubsystem.generated.h"

struct FGameplayTag;

USTRUCT()
struct FFunctionBinding
{
	GENERATED_BODY()

	FFunctionBinding() = default;
	FFunctionBinding(UObject* InObj, UFunction* InFunc) : Obj(InObj), Func(InFunc) {}
	
	TWeakObjectPtr<UObject> Obj;
	TWeakObjectPtr<UFunction> Func;

	bool operator == (const FFunctionBinding& InBinding) const
	{
		return InBinding.Obj == Obj && InBinding.Func == Func;
	}

	bool IsValid() const
	{
		return Obj.IsValid() && Func.IsValid();
	}

	friend uint32 GetTypeHash(const FFunctionBinding& InFunctionBinding)
	{
		return HashCombine(GetTypeHash(InFunctionBinding.Obj), GetTypeHash(InFunctionBinding.Func));
	}
};

UCLASS()
class OUTERWILDS_API UEventsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (DefaultToSelf = InNotifiedObject))
	bool TryBindFunctionToChannel(FGameplayTag InTag, FName InNotifiedFunctionName, UObject* InNotifiedObject);
	UFUNCTION(BlueprintCallable)
	bool TryCreateChannelByTag(FGameplayTag InTag);
	UFUNCTION(BlueprintCallable, CustomThunk,  meta = (CustomStructureParam = "Params"))
	void BroadcastChannelEventByTag_Params(FGameplayTag InTag, const int32& Params);
	UFUNCTION(BlueprintCallable)
	void BroadcastChannelEventByTag_NoParams(FGameplayTag InTag);
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

private:
	static void Internal_BroadcastEventByTag(FGameplayTag InTag, void* InParams, const FProperty* ParamType);
	static bool Internal_TagValidation(const FGameplayTag& InTag);
	static bool Internal_FunctionParamsMatchesEventParams(const UFunction* InFunc, const FProperty* InParamsProperty);

	DECLARE_FUNCTION(execBroadcastChannelEventByTag_Params);
	
};
