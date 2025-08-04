#pragma once
 

#include "PCH.h"
#include "Gameplay/SceneComponent.h"


namespace Gameplay {

    class UCameraComponent : public USceneComponent
    {
    public: 
        UCameraComponent(); 
         
        void SetFieldOfView(float fovDegrees);    
        float GetFieldOfView() const;

        void SetAspectRatio(float aspect);  
        float GetAspectRatio() const;

        void SetNearClip(float nearClip);
        float GetNearClip() const;

        void SetFarClip(float farClip);
        float GetFarClip() const;

        void SetOrthoWidth(float width);       
        float GetOrthoWidth() const;

        void SetProjectionMode(bool bPerspective);  
        bool IsPerspective() const;
         
        Float4x4 GetViewMatrix() const;
        Float4x4 GetProjectionMatrix() const;
        Float4x4 GetViewProjectionMatrix() const;
         
        Float3 GetForwardVector() const;
        Float3 GetRightVector() const;
        Float3 GetUpVector() const;

    protected:
        float m_fovDegrees = 60.0f;
        float m_aspectRatio = 16.0f / 9.0f;
        float m_nearClip = 0.1f;
        float m_farClip = 1000.0f;
        bool  m_bPerspective = true;
        float m_orthoWidth = 512.0f;
    };

}