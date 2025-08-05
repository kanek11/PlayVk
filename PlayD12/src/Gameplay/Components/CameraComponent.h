#pragma once 
#include "PCH.h"
#include "Gameplay/SceneComponent.h"


namespace Gameplay {


    class USpringArmComponent : public USceneComponent
    {
    public:
		USpringArmComponent();
        virtual void TickComponent(float delta); 
         
		float TargetArmLength = 3.0f; 
        Float3 LocalRotationEuler = { 0, 0, 0 };
		Float3 LocalOffset = { 0, 0, 0 }; 

        bool bInheritPitch = false;
        bool bInheritYaw = true;
        bool bInheritRoll = false;
    };
     

    class UCameraComponent : public USceneComponent
    {
    public:
        UCameraComponent();
        virtual void TickComponent(float delta);

		void SetFieldOfView(float fovDegrees) {
			m_fovDegrees = fovDegrees;
		}
        float GetFieldOfView() const
		{
			return m_fovDegrees;
		}

        void SetAspectRatio(float aspect) 
		{
			m_aspectRatio = aspect;
		}
        float GetAspectRatio() const
		{
			return m_aspectRatio;
		}

        void SetNearClip(float nearClip)
		{
			m_nearClip = nearClip;
		}
        float GetNearClip() const
		{
			return m_nearClip;
		}

        void SetFarClip(float farClip)
		{
			m_farClip = farClip;
		}
        float GetFarClip() const
		{
			return m_farClip;
		}

        void SetOrthoWidth(float width);
        float GetOrthoWidth() const;

        void SetProjectionMode(bool bPerspective);
        bool IsPerspective() const;

        Float4x4 GetProjectionMatrix() const;
        Float4x4 GetInvProjectionMatrix() const;
        //the view matrix is instead derived
        //Float4x4 GetViewMatrix() const;
        //Float4x4 GetViewProjectionMatrix() const; 

    protected:
        float m_fovDegrees = 60.0f;
        float m_aspectRatio = 16.0f / 9.0f;
        float m_nearClip = 0.1f;
        float m_farClip = 1000.0f;
        bool  m_bPerspective = true;

        //
        float m_orthoWidth = 512.0f;
    };

}