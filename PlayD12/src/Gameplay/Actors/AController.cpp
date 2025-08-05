#include "PCH.h"
#include "AController.h"

#include "Gameplay/Actors/APawn.h" 
#include "Gameplay/Actors/AController.h"
#include "Gameplay/World.h"
 

namespace Gameplay
{
    void AController::BeginPlay()
    {
        AActor::BeginPlay();

        //register itself;
        //if (auto world = GetWorld()) {
        //    auto& vec = world->playerControllers;
        //    auto it = std::ranges::find(vec, this);
        //    if (it == vec.end()) {
        //        world->AddPlayerController(this);
        //    }
        //}
    } 
     
    void AController::Possess(APawn* inPawn)
    {
        if (Pawn)
            Pawn->UnPossessed();
        Pawn = inPawn;
        if (Pawn)
            Pawn->PossessedBy(this);
    }

    void AController::UnPossess()
    {
        if (Pawn)
        {
            Pawn->UnPossessed();
        }
        Pawn = nullptr;
    }

    void AController::GetPlayerViewPoint(Float3& OutLocation, DirectX::XMVECTOR& OutRotation) const
    {
    }

    AActor* AController::GetViewTarget() const
    {
        return Pawn;
    }


}