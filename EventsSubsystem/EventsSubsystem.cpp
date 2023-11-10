// Free to use WIP Blueprint Events subsystem, Alessandro Profera 07/11/2023

#include "EventsSubsystem.h"
#include <GameplayTagContainer.h>

static TMap<FGameplayTag, TArray<FFunctionBinding>> BindingPairs;

void UEventsSubsystem::Internal_BroadcastEventByTag(FGameplayTag InTag, void* InParams,	const FProperty* ParamType)
{
	if (!Internal_TagValidation(InTag))
	{
		return;
	}

	if (const auto* BoundObjects = BindingPairs.Find(InTag); BoundObjects && !BoundObjects->IsEmpty())
	{
		for (auto& BoundObject : *BoundObjects)
		{
			if (!BoundObject.IsValid())
			{
				continue;
			}

			if(!Internal_FunctionParamsMatchesEventParams(BoundObject.Func.Get(), ParamType))
			{
				continue;
			}
			
			BoundObject.Obj->ProcessEvent(BoundObject.Func.Get(), InParams);
		}
	}
}
bool UEventsSubsystem::Internal_TagValidation(const FGameplayTag& InTag)
{
	if (!InTag.IsValid())
	{
#if WITH_EDITOR
		UE_LOG(LogTemp, Warning, TEXT("Invalid Tag passed to EventsSubsystem"));
#endif
		return false;
	}

	return true;
}

bool UEventsSubsystem::Internal_FunctionParamsMatchesEventParams(const UFunction* InFunc,
	const FProperty* InParamsProperty)
{
	int32 FuncInParamsNum = 0;
	for (TFieldIterator<FProperty> It(InFunc); It; ++It)
	{
		const FProperty* FuncProperty = *It;
		if (FuncProperty->PropertyFlags & CPF_Parm)
		{
			++FuncInParamsNum;
			if(!FuncProperty->SameType(InParamsProperty))
			{
				return false;		
			}
		}
	}

	if(FuncInParamsNum == 0)
	{
		if(InParamsProperty)
		{
			return false;
		}
	}

	return true;
	
}

bool UEventsSubsystem::TryBindFunctionToChannel(FGameplayTag InChannelTag, FName InNotifiedFunctionName, UObject* InNotifiedObject)
{
	check(InNotifiedObject);
	if (!Internal_TagValidation(InChannelTag))
	{
		return false;
	}

	if (InNotifiedFunctionName.IsNone())
	{
		return false;
	}

	UFunction* FoundFunction = InNotifiedObject->FindFunction(InNotifiedFunctionName);
	if (!FoundFunction)
	{
		return false;
	}

	if (auto* BoundObjects = BindingPairs.Find(InChannelTag))
	{
		if (!Algo::FindByPredicate(*BoundObjects, [&FoundFunction, &InNotifiedObject](const FFunctionBinding& Binding)
		{
			if (!Binding.IsValid())
			{
				return false;
			}

			return Binding == FFunctionBinding{InNotifiedObject, FoundFunction};
		}))
		{
			BoundObjects->Emplace(InNotifiedObject, FoundFunction);
			return true;
		}
		
#if WITH_EDITOR
			UE_LOG(LogTemp, Warning, TEXT("Trying to bind a function that is already bound to channel: %s"), *InChannelTag.ToString());
#endif
		return false;
		
	}

	return false;
}

bool UEventsSubsystem::TryRemoveFunctionBindingFromChannel(FGameplayTag InChannelTag, FName InboundFunctionName,
	UObject* InBoundObject)
{
	if(!Internal_TagValidation(InChannelTag))
	{
		return false;
	}

	if (InboundFunctionName.IsNone())
	{
		return false;
	}

	UFunction* FoundFunction = InBoundObject->FindFunction(InboundFunctionName);
	if (!FoundFunction)
	{
		return false;
	}

	if (auto* BoundObjects = BindingPairs.Find(InChannelTag))
	{
		if (Algo::FindByPredicate(*BoundObjects, [&FoundFunction, &InBoundObject](const FFunctionBinding& Binding)
		{
			if (!Binding.IsValid())
			{
				return false;
			}

			return Binding == FFunctionBinding{InBoundObject, FoundFunction};
		}))
		{
			BoundObjects->RemoveSingle({InBoundObject, FoundFunction});
			return true;
		}
		
#if WITH_EDITOR
		UE_LOG(LogTemp, Warning, TEXT("Trying to remove function binding that is not bound to channel: %s"), *InChannelTag.ToString());
#endif
		return false;
		
	}

	return false;
}

bool UEventsSubsystem::TryCreateChannelByTag(FGameplayTag InChannelTag)
{
	if (!Internal_TagValidation(InChannelTag))
	{
		return false;
	}

	if (!BindingPairs.Find(InChannelTag))
	{
		BindingPairs.Emplace(InChannelTag, TArray<FFunctionBinding>{});
		return true;
	}

#if WITH_EDITOR
	UE_LOG(LogTemp, Warning, TEXT("Trying to create a channel that already exists"));
#endif
	return false;
	
}

void UEventsSubsystem::BroadcastChannelEventByTag_Params(FGameplayTag InChannelTag, const int32& Params)
{
	check(0);
}

void UEventsSubsystem::BroadcastChannelEventByTag_NoParams(FGameplayTag InChannelTag)
{
	Internal_BroadcastEventByTag(InChannelTag, nullptr, nullptr);
}

void UEventsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UEventsSubsystem::Deinitialize()
{
	BindingPairs.Empty();
	Super::Deinitialize();
}

bool UEventsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer);
}

DEFINE_FUNCTION(UEventsSubsystem::execBroadcastChannelEventByTag_Params)
{
	/*Create Tag that is going to be populated*/
	FGameplayTag TagPtr = FGameplayTag{};
	Stack.MostRecentProperty = nullptr;
	/*Go one step forward in the stack, find the Tag and populate the pointer passed*/
	Stack.Step(Stack.Object, &TagPtr);
	/*Go another step forward in the stack and find the wildcard value*/
	Stack.Step(Stack.Object, nullptr);
	/*We don't care what the wildcard value is, just create a void pointer pointing to it*/
	void* ParamAddress = Stack.MostRecentPropertyAddress;

	P_FINISH
	P_NATIVE_BEGIN;
		Internal_BroadcastEventByTag(TagPtr, ParamAddress, Stack.MostRecentProperty);
	P_NATIVE_END;
}
