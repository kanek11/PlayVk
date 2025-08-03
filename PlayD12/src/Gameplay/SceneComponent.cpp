#include "PCH.h"
#include "SceneComponent.h"

using namespace DirectX;

FTransform FTransform::CombineWith(const FTransform& Parent) const
{ 
	FTransform Combined; 
  
    Float3 outScale = {
        scale.x() * Parent.scale.x(),
        scale.y() * Parent.scale.y(),
        scale.z() * Parent.scale.z()
    };
	// warn: the order of multiplication matters
    DirectX::XMVECTOR outRot = DirectX::XMQuaternionMultiply(rotation, Parent.rotation);
  
	DirectX::XMFLOAT3 xmPos = DirectX::XMFLOAT3{ position.x(), position.y(), position.z() };
    DirectX::XMVECTOR localPos = DirectX::XMVector3Rotate(DirectX::XMLoadFloat3(&xmPos), Parent.rotation);
    DirectX::XMFLOAT3 xmOutPos;
    DirectX::XMStoreFloat3(&xmOutPos, localPos);  

    Float3 outPos = {
        xmOutPos.x * Parent.scale.x() + Parent.position.x(),
        xmOutPos.y * Parent.scale.y() + Parent.position.y(),
        xmOutPos.z * Parent.scale.z() + Parent.position.z()
    };
    return FTransform{ outPos, outRot, outScale };
}

Float4x4 FTransform::ToMatrix() const
{
    auto S = MMath::MatrixScaling(scale.x(), scale.y(), scale.z()); 
    auto T = MMath::MatrixTranslation(position.x(), position.y(), position.z());
    
    XMMATRIX R_ = XMMatrixRotationQuaternion(rotation);
    Float4x4 R;
	R[0] = { R_.r[0].m128_f32[0], R_.r[0].m128_f32[1], R_.r[0].m128_f32[2] , 0.0f };
	R[1] = { R_.r[1].m128_f32[0], R_.r[1].m128_f32[1], R_.r[1].m128_f32[2] , 0.0f };
	R[2] = { R_.r[2].m128_f32[0], R_.r[2].m128_f32[1], R_.r[2].m128_f32[2] , 0.0f };
	R[3] = { 0.0f, 0.0f, 0.0f, 1.0f }; 

	Float4x4 modelMatrix = MMath::MatrixIdentity<float, 4>();
	modelMatrix = MatrixMultiply(S, modelMatrix);  
	modelMatrix = MatrixMultiply(R, modelMatrix);   
	modelMatrix = MatrixMultiply(T, modelMatrix); 
	return modelMatrix;
    

}

void Gameplay::USceneComponent::UpdateWorldTransform()
{
    if (m_parent)
        m_worldTransform = m_relativeTransform.CombineWith(m_parent->m_worldTransform);
    else
        m_worldTransform = m_relativeTransform;

    for (auto* child : m_children)
    {
        child->UpdateWorldTransform();
    }
    m_transformDirty = false;
}

void Gameplay::USceneComponent::AttachTo(USceneComponent* newParent)
{
    if (m_parent == newParent) return;

	//unbind from current parent if any
    if (m_parent)
    {
        auto& siblings = m_parent->m_children;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
    }

    m_parent = newParent;

    if (m_parent)
    {
        m_parent->m_children.push_back(this);
    }

    MarkDirty();
}

void Gameplay::USceneComponent::DetachFromParent()
{
    if (m_parent)
    {
        auto& siblings = m_parent->m_children;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
        m_parent = nullptr;
        MarkDirty();
    }
}

void Gameplay::USceneComponent::MarkDirty()
{
    m_transformDirty = true;
    for (auto* child : m_children)
        child->MarkDirty();
}
