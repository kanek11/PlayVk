#pragma once

#include "PCH.h"
#include "Math/MMath.h"

#include "ActorComponent.h"

struct FTransform {
	Float3 position{ 0.0f, 0.0f, 0.0f };
	DirectX::XMVECTOR rotation{ DirectX::XMQuaternionIdentity() };
	Float3 scale{ 1.0f, 1.0f, 1.0f };

	FTransform CombineWith(const FTransform& Parent) const;
	Float4x4 ToMatrix() const;
};

namespace Gameplay {

	class USceneComponent : public UActorComponent {
	public:
		virtual ~USceneComponent() = default;

		const FTransform& GetRelativeTransform() const { return m_relativeTransform; }
		void SetRelativeTransform(const FTransform& NewTransform) { m_relativeTransform = NewTransform; }

		const FTransform& GetWorldTransform() const { return m_worldTransform; }
		void SetWorldTransform(const FTransform& t) { m_worldTransform = t; }

		//for the engine
		void UpdateWorldTransform();
		void UpdateChildTransforms();

	public:
		Float3 GetWorldPosition() const { return m_worldTransform.position; }
		DirectX::XMVECTOR GetWorldRotation() const { return m_worldTransform.rotation; }
		Float3 GetWorldScale() const { return m_worldTransform.scale; }

		void SetWorldPosition(const Float3& pos) { m_worldTransform.position = pos; MarkDirty(); }
		void SetWorldRotation(const DirectX::XMVECTOR& rot) { m_worldTransform.rotation = rot; MarkDirty(); }
		void SetWorldScale(const Float3& scl) { m_worldTransform.scale = scl; MarkDirty(); }


		Float3 GetRelativePosition() const { return m_relativeTransform.position; }
		DirectX::XMVECTOR GetRelativeRotation() const { return m_relativeTransform.rotation; }
		Float3 GetRelativeScale() const { return m_relativeTransform.scale; }

		void SetRelativePosition(const Float3& pos) { m_relativeTransform.position = pos; MarkDirty(); }
		void SetRelativeRotation(const DirectX::XMVECTOR& rot) { m_relativeTransform.rotation = rot; MarkDirty(); }
		void SetRelativeScale(const Float3& scl) { m_relativeTransform.scale = scl; MarkDirty(); }
	public:

		Float3 GetForwardVector() const;
		Float3 GetRightVector() const;
		Float3 GetUpVector() const;

		//i simply put it here
		Float4x4 GetViewMatrix() const;
		Float4x4 GetInvViewMatrix() const;
	public:
		void AttachTo(USceneComponent* newParent);
		void DetachFromParent();

		USceneComponent* GetParent() const { return m_parent; }
		const std::span<USceneComponent* const > GetChildren() const { return m_children; }

	protected:
		FTransform m_relativeTransform;
		FTransform m_worldTransform;

		USceneComponent* m_parent = nullptr;
		std::vector<USceneComponent*> m_children;

		void MarkDirty();
		bool m_transformDirty = true;


		bool bAbsolute{ false };
	};



}