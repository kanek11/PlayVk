#pragma once
#include "PCH.h"

#include "Base.h"
#include "Delegate.h"
#include "Event.h"


// geometry helpers
struct Rect { int x{}, y{}, w{}, h{}; };
inline bool IsPointInRect(const Rect& r, int px, int py) {
    return px >= r.x && px <= r.x + r.w && py >= r.y && py <= r.y + r.h;
} 

//classic four states 
enum class UIState {
	Normal,
	Hovered,
	Pressed, 
	Disabled
};


//forward decl:
class UIRenderer;

class UIElement {
public:
	virtual ~UIElement() = default; 
	 
	virtual void Render(UIRenderer& renderer) = 0;

	virtual void OnMouseMove(const UIMouseMove& e) {
		if (HitTest(e.x, e.y)) {
			//std::cout << "UI: mouse hovered at (" << e.x << ", " << e.y << ")" << '\n';
			state = UIState::Hovered;
		}
		else {
			state = UIState::Normal;
		}
	}

	virtual void OnMouseButtonDown(const UIMouseButtonDown& e) {
		if (HitTest(e.x, e.y)) {
			//std::cout << "UI: button pressed at (" << e.x << ", " << e.y << ")" << '\n';
			state = UIState::Pressed;
		}
	}

	virtual void OnMouseButtonUp(const UIMouseButtonUp& e) {
		if (HitTest(e.x, e.y)) {
			if (state == UIState::Pressed) {
				OnClick.BlockingBroadCast(); 
				state = UIState::Normal;
			} 
		}
		else {
			state = UIState::Normal;
			std::cout << "UI: button up outside of button area" << '\n';
		}
	}

	virtual bool HitTest(int x, int y) const = 0;

	//todo: implement the tree;
	virtual std::span<UIElement* const> GetChildren() const { return {}; }
     

	UIState state = UIState::Normal;

	FDelegate<void()> OnClick;  
};



//todo: visible ,enable; blocking;
class UIButton : public UIElement
{
public: 
	virtual ~UIButton() = default;
	UIButton(Rect rect): layout(rect) { } 

	virtual void Render(UIRenderer& renderer) override;

	void Tick(float delta); 

	virtual bool HitTest(int x, int y) const override {
		return IsPointInRect(layout, x, y);
	}

	Rect layout;  
};




class UIManager {
public: 
    void ProcessEvents() {

		auto uiEvents = UIEventQueue::Get().Drain();
		//std::cout << "UIManager: processing " << uiEvents.size() << " UI events" << '\n';

        for (UIEvent& e : uiEvents) { 
			std::visit([&](auto& evt) { DispatchToUIRoots(evt); }, e);
        }
        uiEvents.clear(); 
    }

    void RegisterRootElement(UIElement* elem) {
        rootElements.push_back(elem);
    }

private: 
    std::vector<UIElement*> rootElements;
    UIElement* mouseCaptured = nullptr;

    // T must be a UIEvent variant member
    template<typename T>
    void DispatchToUIRoots(T& evt) {
		if (mouseCaptured && !evt.handled) {
            DispatchToElement(mouseCaptured, evt);
            return;
        }

        for (UIElement* elem : rootElements) {
            if (DispatchRecursive(elem, evt)) break;
        }
    }

	//todo: dispatch within the element;
    template<typename T>  
    void DispatchToElement(UIElement* elem, T& evt) {
        if constexpr (std::is_same_v<T, UIMouseMove>) {
            elem->OnMouseMove(evt);
        }
        else if constexpr (std::is_same_v<T, UIMouseButtonDown>) {
            elem->OnMouseButtonDown(evt);
			//capture mouse on down:
			if (mouseCaptured == nullptr) 
			mouseCaptured = elem;

        }
        else if constexpr (std::is_same_v<T, UIMouseButtonUp>) {
            elem->OnMouseButtonUp(evt);
			//release capture on up:
			if (mouseCaptured == elem)  
				mouseCaptured = nullptr;  
		} 
    }

	//DFS dispatch
    template<typename T> 
    bool DispatchRecursive(UIElement* elem, T& evt) { 
		//ignore if the element is handled, or not hit;
		if (evt.handled)
            return false;

        for (UIElement* child : elem->GetChildren()) {
            if (DispatchRecursive(child, evt)) return true;
        }
		//std::cout << "UIManager: dispatching event to element" << '\n'; 

		DispatchToElement(elem, evt); 
		
		//todo: element-dependent propagation logic;
		//evt.StopPropagation();

        return true;
    }

};




