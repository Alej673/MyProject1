// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "ControladorEnemigoBase.generated.h"

// Declaraciones anticipadas para optimizar la compilación
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;

UCLASS()
class MYPROJECT1_API AControladorEnemigoBase : public AAIController
{
	GENERATED_BODY()

public:
	// Constructor
	AControladorEnemigoBase();

protected:
	virtual void BeginPlay() override;

	// --- COMPONENTES DE PERCEPCIÓN --- //

	// El componente principal que agrupa los sentidos
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IA|Percepcion")
	UAIPerceptionComponent* ComponentePercepcion;

	// Configuración específica de la Vista
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IA|Percepcion")
	UAISenseConfig_Sight* ConfigVista;

	// Configuración específica del Oído
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IA|Percepcion")
	UAISenseConfig_Hearing* ConfigOido;

	// Función que el motor llamará automáticamente cuando el NPC vea o escuche algo
	UFUNCTION()
	void AlDetectarEstimulo(AActor* ActorDetectado, FAIStimulus Estimulo);
};