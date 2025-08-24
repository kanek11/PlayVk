#include "PCH.h"
#include "CameraComponent.h"

#include "Application.h"

using namespace DirectX;

namespace Gameplay {


    UCameraComponent::UCameraComponent()
    {
        float aspectRatio = GameApplication::GetInstance()->getAspectRatio();
        this->m_aspectRatio = aspectRatio;
    }

    void UCameraComponent::TickComponent(float delta)
    {
        USceneComponent::TickComponent(delta);

        //std::cout << "camera pos:" << ToString(this->GetWorldPosition()) << '\n';
    }

    Float4x4 UCameraComponent::GetProjectionMatrix() const
    {
        auto proj_ = MMath::PerspectiveFovLH(
            MMath::ToRadians(m_fovDegrees),
            m_aspectRatio,
            m_nearClip,
            m_farClip
        );

        return proj_;
    }

    Float4x4 UCameraComponent::GetInvProjectionMatrix() const
    {
        auto proj_ = MMath::InversePerspectiveFovLH(
            MMath::ToRadians(m_fovDegrees),
            m_aspectRatio,
            m_nearClip,
            m_farClip
        );

        return proj_;
    }


    USpringArmComponent::USpringArmComponent()
    {
        //new: skip tree update flag:
        bAbsolute = true;

        //SetRelativePosition(Float3{ 0.0f, 2.0f, -5.0f });
        //SetRelativeRotation(DirectX::XMQuaternionRotationRollPitchYaw(MMath::ToRadians(20.0f), 0.0f, 0.0f));

        LocalOffset = { 0.0f, 1.0f, -4.0f }; //Float3{ 0.0f, 4.0f, -5.0f }; //offset from parent
        LocalRotationEuler = Float3{ MMath::ToRadians(0.0f), MMath::ToRadians(0.0f), 0.0f }; //Float3{ MMath::ToRadians(30.0f), 0.0f, 0.0f }; //pitch, yaw, roll 
    }


    //todo: move this logic to "late tick"
    void USpringArmComponent::TickComponent(float delta)
    {
        USceneComponent::TickComponent(delta);

        //DirectX::XMVECTOR desiredRot = DirectX::XMQuaternionIdentity();
        //this->SetWorldRotation(desiredRot); 
        if (!m_parent) return;

        Float3 basePosition = m_parent->GetWorldPosition();
        //auto baseRotation = m_parent->GetWorldRotation(); 

        auto desiredRot = XMQuaternionRotationRollPitchYaw(
            LocalRotationEuler.x(),
            LocalRotationEuler.y(),
            LocalRotationEuler.z());

        Float3 finalPosition = basePosition + LocalOffset;

        SetWorldPosition(finalPosition);
        SetWorldRotation(desiredRot);

        //manually set the children:
        this->UpdateChildTransforms();

        //get euler angles from XMQuaternion 
        //auto euler = ToEuler(baseRotation);  
        //Float3 finalRotation;
        //finalRotation.x = bInheritPitch ? baseRotation.x + LocalRotationEuler.x : LocalRotationEuler.x;
        //finalRotation.y = bInheritYaw ? baseRotation.y + LocalRotationEuler.y : LocalRotationEuler.y;
        //finalRotation.z = bInheritRoll ? baseRotation.z + LocalRotationEuler.z : LocalRotationEuler.z;


    }

}