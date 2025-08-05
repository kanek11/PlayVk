#include "PCH.h"
#include "AController.h"

#include "Gameplay/Actors/AController.h"
#include "Gameplay/Actors/APawn.h"


namespace Gameplay
{

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


}