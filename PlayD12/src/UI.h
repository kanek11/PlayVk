#pragma once
#include "PCH.h"

#include "Math/MMath.h"
#include "Base.h"
#include "Delegate.h"
#include "Event.h"

#include "FSM.h"


// geometry helpers
struct FRect { int x{}, y{}, w{}, h{}; };
inline bool IsPointInRect(const FRect& r, int px, int py) {
	return px >= r.x && px <= r.x + r.w && py >= r.y && py <= r.y + r.h;
}

inline FRect CenterRect(int screenWidth, int screenHeight, const FRect& rectSize) {
	int cornerX = (screenWidth - rectSize.w) / 2;
	int cornerY = (screenHeight - rectSize.h) / 2;
	return FRect{ cornerX, cornerY, rectSize.w, rectSize.h };
}

inline Float2 ScreenToNDC(int x, int y, int screenWidth, int screenHeight) {
	float ndcX = (2.0f * static_cast<float>(x + 0.5f) / static_cast<float>(screenWidth)) - 1.0f;
	float ndcY = 1.0f - (2.0f * static_cast<float>(y + 0.5f) / static_cast<float>(screenHeight)); // Invert Y for top-left 
	//std::cout << "get ndc:" << ndcX << " " << ndcY << '\n';
	return { ndcX, ndcY };
}

 
//classic four states 
enum class UIState {
	Disabled,
	Idle,
	Hovered,
	PressedInside,
	PressedOutside,  
};  


class UIElement {
public:
	UIElement();
	virtual ~UIElement() = default;

	virtual void Tick(float delta) { 

		for (auto* child : m_children)
		{
			child->Tick(delta);
		}
	};

	virtual void RenderBack() {}; 

public: 
	UIState state = UIState::Idle; 

	virtual bool HitTest(int x, int y) const { return false; };
 
	void OnEvent(UIEvent& evt) {
		std::visit([this](auto& e) {
			//std::cerr << "UIEvent holds: " << typeid(e).name() << '\n';
			if (HitTest(e.x, e.y)) e.handled = true; // mark as handled 
			}, evt); 
	 
		std::visit(overloaded{
			[this](UIMouseMove& e) { OnMouseMove(e); },
			[this](UIMouseButtonDown& e) { OnMouseButtonDown(e); },
			[this](UIMouseButtonUp& e) { OnMouseButtonUp(e); }
			}, evt);
	}
	
	virtual void OnMouseMove(const UIMouseMove& e) {

		//std::cout << "UI: handle move, curr state:" << (int)state << '\n';
		bool hit = HitTest(e.x, e.y);

		if (state == UIState::Idle) {
			if (hit) { 
				OnHoverEnter.BlockingBroadCast();
				state = UIState::Hovered;
				std::cout << "UI: button hover enter at (" << e.x << ", " << e.y << ")" << '\n';
	 			} 
		}
		else if (state == UIState::Hovered) {
			if (!hit) {

				OnHoverExit.BlockingBroadCast();
				state = UIState::Idle;
				std::cout << "UI: button hover out" << '\n';
			} 
		}
		else if (state == UIState::PressedInside) {
			if (!hit) {
				OnHoverExit.BlockingBroadCast();
				state = UIState::Idle;
				//std::cout << "UI: button pressed outside of area" << '\n';
			}
		}
		 

	}

	virtual void OnMouseButtonDown(const UIMouseButtonDown& e) {
		//std::cout << "UI: handle hit down, curr state:" << (int)state << '\n';
		//if (HitTest(e.x, e.y)) {
		//	std::cout << "UI: button pressed at (" << e.x << ", " << e.y << ")" << '\n';
		//	state = UIStateId::Pressed;
		//}
		if (state == UIState::Hovered) {
			state = UIState::PressedInside;
			//std::cout << "UI: button pressed :" << name << '\n';
		} 
	}

	virtual void OnMouseButtonUp(const UIMouseButtonUp& e) {
		//std::cout << "UI: handle hit release, curr state:" << (int)state << '\n';
		if (state == UIState::PressedInside) {
			OnClick.BlockingBroadCast();
			state = UIState::Hovered;  
		} 

	}
public: 
	//todo: implement the tree;
	UIElement* GetParent() const { return m_parent; } 
	virtual std::span<UIElement* const> GetChildren() const
	{
		return m_children;
	}

	UIElement* m_parent = nullptr;
	std::vector<UIElement*> m_children;

	void AttachTo(UIElement* newParent) {
		if (newParent == nullptr) {
			std::cerr << "UIElement: cannot attach to null parent!" << '\n';
			return;
		}
		if (m_parent) {
			auto& siblings = m_parent->m_children;
			siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
		}

		m_parent = newParent; 
		m_parent->m_children.push_back(this); 
	}
	void DetachFromParent()
	{
		if (m_parent) {
			auto& siblings = m_parent->m_children;
			siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
			m_parent = nullptr;
		}
	}

public:
	Float4 baseColor = Color::White;

	FDelegate<void()> OnClick;
	FDelegate<void()> OnHoverEnter;
	FDelegate<void()> OnHoverExit;


public:
	void SetLayout(const FRect& rect) {
		layout = rect;
	}
	std::optional<FRect> layout;


	std::string name = "UIElement"; // for debugging
};
 

//todo: visible ,enable; blocking;
class UIButton : public UIElement
{
public: 
	UIButton();
	virtual ~UIButton() = default;

	virtual void RenderBack() override;
	void RenderText();

	virtual void Tick(float delta) override;

	virtual bool HitTest(int x, int y) const override {
		assert(layout.has_value());
		return IsPointInRect(layout.value(), x, y);
	}

	std::string text = "HELLO";
};
 
class UICanvasPanel : public UIElement {
public:
	virtual void Tick(float delta) override;
 };




class UIManager {
public:
	void Tick(float delta) {

		for (auto& root : rootElements) {
			root->Tick(delta);  
		} 
	}

	void RouteEvents() {

		auto uiEvents = UIEventQueue::Get().Drain();
		//std::cout << "UIManager: processing " << uiEvents.size() << " UI events" << '\n';

		for (UIEvent& e : uiEvents) {
			//std::visit([&](auto& evt) { DispatchToRoots(evt); }, e);
			DispatchToRoots(e);
		}
		uiEvents.clear();
	}


	template<DerivedFrom<UIElement> T>
	SharedPtr<T> CreateUIAsRoot() {
		SharedPtr<T> elem = CreateShared<T>();
		rootElements.push_back(elem.get());
		return elem;
	}
 
	void RegisterRootElement(UIElement* elem) {
		if (elem) {
			rootElements.push_back(elem);
		}
	}

	void ClearRoot() {
		rootElements.clear();
	}

private:
	std::vector<UIElement*> rootElements;
	UIElement* captured = nullptr;

	void DispatchToRoots(UIEvent& evt) {
		//if (captured) {
		//	DispatchToElement(captured, evt);
		//	return;
		//}

		for (UIElement* elem : rootElements) {
			if (DispatchRecursive(elem, evt)) break;
		}
	}

	//DFS dispatch 
	bool DispatchRecursive(UIElement* elem, UIEvent& evt) {

		for (UIElement* child : elem->GetChildren()) {
			//return on first child that handles the event
			//std::cout << "UI: dispatching event to child: " << child->name << '\n';
			if (DispatchRecursive(child, evt)) return true;
		}

		//std::cout << "UI: dispatching event to: " << elem->name << '\n';

		DispatchToElement(elem, evt);
 
		return std::visit([](auto const& e) {
			//return true if the event was handled
			return e.handled;
			}, evt);
	} 

	//todo: dispatch within the element; 
	void DispatchToElement(UIElement* elem, UIEvent& evt) { 
		elem->OnEvent(evt); 
	} 


};


