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
    void AController::OnTick(float delta)
    {
        AActor::OnTick(delta);

        ProcessPlayerInput(delta);
    }

    void AController::ProcessPlayerInput(float delta)
    {
        if (currInputMode == EInputMode::UIOnly) return;

        if (CurrInputComp)
        {
            CurrInputComp->TickComponent(delta);
        }
    }

    void AController::Possess(WeakPtr<APawn> inPawn)
    {
        if (!Pawn.expired())
            Pawn.lock()->UnPossessed();
        Pawn = inPawn;
        if (!Pawn.expired())
            Pawn.lock()->PossessedBy(this);
    }

    void AController::UnPossess()
    {
        if (!Pawn.expired())
        {
            Pawn.lock()->UnPossessed();
        }
        Pawn.reset(); //reset weakptr ~ nullify;
    }

    void AController::GetPlayerViewPoint(Float3& OutLocation, DirectX::XMVECTOR& OutRotation) const
    {
    }

    AActor* AController::GetViewTarget() const
    {
        if (!Pawn.expired())
        {
            return Pawn.lock().get();
        }
        else {
            return nullptr;
        }
        
    }


}