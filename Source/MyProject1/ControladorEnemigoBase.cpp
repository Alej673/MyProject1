// Fill out your copyright notice in the Description page of Project Settings.


#include "ControladorEnemigoBase.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"

AControladorEnemigoBase::AControladorEnemigoBase()
{
	// 1. Instanciar el componente principal
	ComponentePercepcion = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PercepcionIA"));
	SetPerceptionComponent(*ComponentePercepcion);

	// 2. Configurar el sentido de la VISTA
	ConfigVista = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("ConfigVista"));
	ConfigVista->SightRadius = 1500.0f; // Distancia máxima a la que te ve
	ConfigVista->LoseSightRadius = 2000.0f; // Distancia a la que te pierde de vista
	ConfigVista->PeripheralVisionAngleDegrees = 65.0f; // Cono de visión en grados
	ConfigVista->SetMaxAge(5.0f); // Cuánto tiempo recuerda haberte visto

	// Necesario en Unreal Engine para que detecte jugadores
	ConfigVista->DetectionByAffiliation.bDetectEnemies = true;
	ConfigVista->DetectionByAffiliation.bDetectNeutrals = true;
	ConfigVista->DetectionByAffiliation.bDetectFriendlies = true;

	// 3. Configurar el sentido del OÍDO
	ConfigOido = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("ConfigOido"));
	ConfigOido->HearingRange = 2500.0f; // Radio para escuchar pasos o recargas
	ConfigOido->SetMaxAge(3.0f); // Cuánto tiempo recuerda el sonido

	ConfigOido->DetectionByAffiliation.bDetectEnemies = true;
	ConfigOido->DetectionByAffiliation.bDetectNeutrals = true;
	ConfigOido->DetectionByAffiliation.bDetectFriendlies = true;

	// 4. Registrar los sentidos en el componente principal
	ComponentePercepcion->ConfigureSense(*ConfigVista);
	ComponentePercepcion->ConfigureSense(*ConfigOido);
	ComponentePercepcion->SetDominantSense(ConfigVista->GetSenseImplementation());
}

void AControladorEnemigoBase::BeginPlay()
{
	Super::BeginPlay();

	// Vincular nuestra función al evento nativo del motor cuando percibe algo
	if (ComponentePercepcion)
	{
		ComponentePercepcion->OnTargetPerceptionUpdated.AddDynamic(this, &AControladorEnemigoBase::AlDetectarEstimulo);
	}
}

void AControladorEnemigoBase::AlDetectarEstimulo(AActor* ActorDetectado, FAIStimulus Estimulo)
{
	// 1. Obtenemos acceso a la libreta (Blackboard) del NPC
	if (UBlackboardComponent* Memoria = GetBlackboardComponent())
	{
		// 2. Verificamos si el estímulo está activo (entró en visión o hizo ruido)
		if (Estimulo.WasSuccessfullySensed())
		{
			// Anotamos la coordenada exacta donde detectó el ruido o te vio
			Memoria->SetValueAsVector(FName("UltimaPosicionConocida"), Estimulo.StimulusLocation);

			// Le decimos al cerebro que te está viendo
			Memoria->SetValueAsBool(FName("JugadorVisto"), true);
		}
		else
		{
			// 3. Si el estímulo caducó (saliste de su cono de visión o el sonido terminó)
			// Le decimos al cerebro que te perdió de vista
			Memoria->SetValueAsBool(FName("JugadorVisto"), false);
		}
	}
}