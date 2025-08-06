#pragma once

#include "PCH.h"
#include "Gameplay/SceneComponent.h"

namespace Gameplay {
     
    class ULightComponent : public UActorComponent
    {
    public:
        virtual ~ULightComponent() = default;
        ULightComponent(); 

        enum class ELightType { Directional, Point, Spot };
        ELightType LightType = ELightType::Directional;


        bool bCastShadow = true;  
         
        virtual void TickComponent(float DeltaTime) override;
    };
     

    class UDirectionalLightComponent : public ULightComponent
    {
        Float3 Color = Float3(1.0f, 1.0f, 1.0f);
        float Intensity = 1.0f;
		Float3 Direction = Float3(0.0f, -1.0f, 0.0f); 
    };

    class UPointLightComponent : public ULightComponent
    {
        Float3 Color = Float3(1.0f, 1.0f, 1.0f);
        float Intensity = 1.0f;
        float AttenuationRadius = 100.f;
    };

    class USpotLightComponent : public UPointLightComponent
    {
        Float3 Color = Float3(1.0f, 1.0f, 1.0f);
        float Intensity = 1.0f;
        float InnerConeAngle = 15.f;
        float OuterConeAngle = 30.f;
    };


}