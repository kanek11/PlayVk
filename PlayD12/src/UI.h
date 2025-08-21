#pragma once
#include "PCH.h"

#include "Math/MMath.h"
#include "Base.h"
#include "Delegate.h"
#include "Event.h"

#include "FSM.h"

/*
* design decision: UIManager is not responsible for the ownership;
* make sure the UI is valid on calling side;
*/


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

inline FRect CenterRect(const FRect& parentRect, const FRect& childRect) {
	int cornerX = (parentRect.w - childRect.w) / 2;
	int cornerY = (parentRect.h - childRect.h) / 2;
	return FRect{ cornerX, cornerY, childRect.w, childRect.h };
}

inline Float2 ScreenToNDC(int x, int y, int screenWidth, int screenHeight) {
	float ndcX = (2.0f * static_cast<float>(x + 0.5f) / static_cast<float>(screenWidth)) - 1.0f;
	float ndcY = 1.0f - (2.0f * static_cast<float>(y + 0.5f) / static_cast<float>(screenHeight)); // Invert Y for top-left 
	//std::cout << "get ndc:" << ndcX << " " << ndcY << '\n';
	return { ndcX, ndcY };
}



//=====================

namespace UI {

	enum class SizePolicy { Fixed, AutoText };

	//percent of parent size;
	enum class Unit { Pixel, Percent };

	struct UISize {

		//value is interpreted by unit
		Unit unit{ Unit::Pixel };
		float value{ 0.f }; 

		//factory
		static UISize Px(float v) { return { Unit::Pixel, v }; }
		static UISize Pc(float v) { return { Unit::Percent, v }; }
	};

	struct Anchors { // 0~1 relative to parent;
		float minX{ 0 }, minY{ 0 }, maxX{ 0 }, maxY{ 0 };

		static Anchors TopLeft() { return { 0,0,0,0 }; }
		static Anchors StretchAll() { return { 0,0,1,1 }; }
	};

	struct UIMargins { int l = 0, t = 0, r = 0, b = 0; };

	//drive the final screen space;
	struct LayoutSpec { 
		UISize  offsetX = UISize::Px(0);
		UISize  offsetY = UISize::Px(0);
		UISize  width = UISize::Px(100);
		UISize  height = UISize::Px(30);
		
		//auto text override the current w, h;
	    SizePolicy policy = SizePolicy::Fixed;
		Anchors anchors = Anchors::TopLeft();
		UIMargins   margin{};
		// pivot if needed..
	};



}








//======================


//classic four states 
enum class UIState {
	Disabled,
	Idle,
	Hovered,
	PressedInside,
	PressedOutside,
};


using UIId = uint32_t;

class UIElement {
public:
	UIElement();
	virtual ~UIElement() = default;


	virtual void OnRegister() {};

	virtual void Tick(float delta) {
		//a temp way to verify init:
		assert(id != 0 && "ui id is not properly initialized");

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

	FRect GetLayout() {
		assert(layout.has_value());
		return layout.value();
	}
	std::optional<FRect> layout;


	std::string name = "UIElement"; // for debugging 
	UIId id{ 0 };
	static std::atomic<uint32_t> GIdGenerator;
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
	UICanvasPanel();
	virtual void Tick(float delta) override;



};




class UIManager {
public:
	void Tick(float delta) {

		for (auto& [id, ui] : rootElements) {
			ui->Tick(delta);
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
		this->RegisterRootElement(elem.get());
		return elem;
	}

	void RegisterRootElement(UIElement* elem) {
		if (elem) {
			rootElements[elem->id] = elem;
			elem->OnRegister(); 
		}
	}

	void UnregisterRootElement(UIElement* elem) {
		if (elem) {
			rootElements.erase(elem->id);
		}
	}

	void ClearRoot() {
		rootElements.clear();
	}

private:
	std::unordered_map<UIId, UIElement*> rootElements;
	UIElement* captured = nullptr;

	void DispatchToRoots(UIEvent& evt) {
		//if (captured) {
		//	DispatchToElement(captured, evt);
		//	return;
		//}

		for (auto& [id, elem] : rootElements) {
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

