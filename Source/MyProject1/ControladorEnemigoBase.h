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
class UAISenseConfig_Damage; // <--- AGREGAR ESTO AQUÍ

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

	// Configuración específica del Daño
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IA|Percepcion")
	UAISenseConfig_Damage* ConfigDanio; // <--- AGREGAR ESTO AQUÍ

	// Función que el motor llamará automáticamente cuando el NPC vea, escuche o reciba daño
	UFUNCTION()
	void AlDetectarEstimulo(AActor* ActorDetectado, FAIStimulus Estimulo);

public:
	// Función que se ejecuta en cada frame para calcular la matemática suavemente
	virtual void Tick(float DeltaTime) override;

protected:
	// --- SISTEMA DE SIGILO Y SUSPENSO --- //

	// Nivel actual de sospecha (0 a 100)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IA|Sigilo")
	float NivelSospecha = 0.0f;

	// ¿A qué velocidad sube la barra por segundo cuando te ve?
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IA|Sigilo")
	float TasaIncrementoSospecha = 50.0f;

	// ¿A qué velocidad se vacía la barra cuando te escondes?
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IA|Sigilo")
	float TasaDecrementoSospecha = 25.0f;

	// Variable interna para saber si el cronómetro debe subir o bajar
	bool bEstaViendoJugador = false;

	// Guardamos al jugador temporalmente mientras sube la barra
	AActor* JugadorDetectadoTemp = nullptr;
};
