// Fill out your copyright notice in the Description page of Project Settings.

#include "ControladorEnemigoBase.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Damage.h"

AControladorEnemigoBase::AControladorEnemigoBase()
{
	//0. Le decimos a Unreal que queremos usar la función Tick en este controlador
	PrimaryActorTick.bCanEverTick = true;

	// 1. Instanciar el componente principal
	ComponentePercepcion = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PercepcionIA"));
	SetPerceptionComponent(*ComponentePercepcion);

	// 2. Configurar el sentido de la VISTA
	ConfigVista = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("ConfigVista"));
	ConfigVista->SightRadius = 1500.0f; // Distancia máxima a la que te ve
	ConfigVista->LoseSightRadius = 2000.0f; // Distancia a la que te pierde de vista
	ConfigVista->PeripheralVisionAngleDegrees = 75.0f; // Cono de visión en grados
	ConfigVista->SetMaxAge(5.0f); // Cuánto tiempo recuerda haberte visto

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

	// --- AQUI ESTABA EL ERROR: Faltaba crear el sentido del Daño ---
	ConfigDanio = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("ConfigDanio"));
	ConfigDanio->SetMaxAge(3.0f);

	// 4. Registrar TODOS los sentidos en el componente principal
	ComponentePercepcion->ConfigureSense(*ConfigVista);
	ComponentePercepcion->ConfigureSense(*ConfigOido);
	ComponentePercepcion->ConfigureSense(*ConfigDanio); // Registrar el daño
	ComponentePercepcion->SetDominantSense(ConfigVista->GetSenseImplementation());
}

void AControladorEnemigoBase::BeginPlay()
{
	Super::BeginPlay();

	if (ComponentePercepcion)
	{
		ComponentePercepcion->OnTargetPerceptionUpdated.AddDynamic(this, &AControladorEnemigoBase::AlDetectarEstimulo);
	}
}

void AControladorEnemigoBase::AlDetectarEstimulo(AActor* ActorDetectado, FAIStimulus Estimulo)
{
	if (ActorDetectado && ActorDetectado->ActorHasTag(FName("Jugador")))
	{
		if (UBlackboardComponent* Memoria = GetBlackboardComponent())
		{
			FAISenseID IdVista = UAISense::GetSenseID<UAISense_Sight>();
			FAISenseID IdOido = UAISense::GetSenseID<UAISense_Hearing>();
			FAISenseID IdDanio = UAISense::GetSenseID<UAISense_Damage>(); // Escrito sin ñ

			// --- CASO 1: EL NPC TE ESTÁ VIENDO ---
			if (Estimulo.Type == IdVista)
			{
				if (Estimulo.WasSuccessfullySensed())
				{
					Memoria->SetValueAsVector(FName("UltimaPosicionConocida"), Estimulo.StimulusLocation);
					bEstaViendoJugador = true;
					JugadorDetectadoTemp = ActorDetectado;
				}
				else
				{
					bEstaViendoJugador = false;
					Memoria->SetValueAsVector(FName("UltimaPosicionConocida"), Estimulo.StimulusLocation);
				}
			}

			// --- CASO 2: EL NPC ESCUCHÓ UN RUIDO ---
			else if (Estimulo.Type == IdOido)
			{
				if (Estimulo.WasSuccessfullySensed())
				{
					Memoria->SetValueAsVector(FName("UltimaPosicionConocida"), Estimulo.StimulusLocation);
					Memoria->SetValueAsBool(FName("EscuchoRuido"), true);
				}
			}

			// --- CASO 3: EL NPC RECIBIÓ UN IMPACTO DE BALA ---
			else if (Estimulo.Type == IdDanio)
			{
				if (Estimulo.WasSuccessfullySensed())
				{
					NivelSospecha = 100.0f;
					bEstaViendoJugador = true;
					JugadorDetectadoTemp = ActorDetectado;

					Memoria->SetValueAsVector(FName("UltimaPosicionConocida"), Estimulo.StimulusLocation);
					Memoria->SetValueAsObject(FName("ActorJugador"), ActorDetectado);
					Memoria->SetValueAsBool(FName("JugadorVisto"), true);
				}
			}
		}
	}
}

void AControladorEnemigoBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (UBlackboardComponent* Memoria = GetBlackboardComponent())
	{
		bool bYaEnCombate = Memoria->GetValueAsBool(FName("JugadorVisto"));

		if (bEstaViendoJugador)
		{
			if (bYaEnCombate)
			{
				NivelSospecha = 100.0f;
				Memoria->SetValueAsBool(FName("Sospechando"), false);
			}
			else
			{
				if (JugadorDetectadoTemp)
				{
					float Distancia = FVector::Dist(GetPawn()->GetActorLocation(), JugadorDetectadoTemp->GetActorLocation());
					float MultiplicadorDistancia = FMath::GetMappedRangeValueClamped(FVector2D(600.0f, 1500.0f), FVector2D(5.0f, 0.5f), Distancia);

					FVector DireccionAlJugador = (JugadorDetectadoTemp->GetActorLocation() - GetPawn()->GetActorLocation()).GetSafeNormal();
					FVector FrenteNPC = GetPawn()->GetActorForwardVector();
					float ProductoPunto = FVector::DotProduct(FrenteNPC, DireccionAlJugador);
					float MultiplicadorVision = FMath::GetMappedRangeValueClamped(FVector2D(0.5f, 0.95f), FVector2D(0.3f, 1.0f), ProductoPunto);

					NivelSospecha += (TasaIncrementoSospecha * MultiplicadorDistancia * MultiplicadorVision) * DeltaTime;
				}
				else
				{
					NivelSospecha += TasaIncrementoSospecha * DeltaTime;
				}

				if (NivelSospecha >= 100.0f)
				{
					NivelSospecha = 100.0f;
					Memoria->SetValueAsBool(FName("JugadorVisto"), true);
					Memoria->SetValueAsBool(FName("Sospechando"), false);

					if (JugadorDetectadoTemp)
					{
						Memoria->SetValueAsObject(FName("ActorJugador"), JugadorDetectadoTemp);
					}
				}
				else if (NivelSospecha > 0.0f)
				{
					Memoria->SetValueAsBool(FName("Sospechando"), true);
				}
			}
		}
		else
		{
			if (bYaEnCombate)
			{
				NivelSospecha -= TasaDecrementoSospecha * DeltaTime;

				if (NivelSospecha < 80.0f)
				{
					Memoria->SetValueAsBool(FName("JugadorVisto"), false);
					Memoria->SetValueAsObject(FName("ActorJugador"), nullptr);
				}
			}
			else
			{
				if (NivelSospecha > 0.0f)
				{
					NivelSospecha -= TasaDecrementoSospecha * DeltaTime;

					if (NivelSospecha <= 0.0f)
					{
						NivelSospecha = 0.0f;
						bool bEraFalsaAlarma = Memoria->GetValueAsBool(FName("Sospechando"));
						Memoria->SetValueAsBool(FName("Sospechando"), false);

						if (bEraFalsaAlarma)
						{
							Memoria->ClearValue(FName("UltimaPosicionConocida"));
						}
					}
				}
			}
		}
	}
}