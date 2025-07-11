// ui_framework.h — initial skeleton for an extensible UI system
// NOTE: This is a *header?only* prototype to get you started.  Nothing here is set in stone —
// add/remove pieces as you validate real requirements.
// -----------------------------------------------------------------------------
//  Motivation / Design goals
//  • Decoupled from platform + renderer (backend selected by dependency?injection).
//  • Hierarchical scene?graph of UIElement objects.
//  • Flexible layout (immediate?mode attributes, retained tree updates via layout engine).
//  • Event system with capture/target/bubble phases and stop?propagation flag.
//  • Bridge to GameFlow: any element can expose FDelegate signals; GUILayer wires these
//    to GameStateManager / ParamMap.
//  • All structs use POD where possible to allow hot?reloading & serialization later.
// -----------------------------------------------------------------------------
#pragma once
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <variant>
#include "Math/MMath.h"    
#include "Delegate.h"     
#include "Event.h"        

namespace UI {
    // -----------------------------------------------------------------------------
    // geometry helpers
    struct Rect { float x{}, y{}, w{}, h{}; };
    inline bool PointInRect(const Rect& r, float px, float py) {
        return px >= r.x && px <= r.x + r.w && py >= r.y && py <= r.y + r.h;
    }

    // -----------------------------------------------------------------------------
    // Forward declarations
    class UIElement;
    class UIContainer;
    class UILayoutEngine;
    class GUIRenderer;
    struct UIContext;

    // -----------------------------------------------------------------------------

    enum class PropagationPhase : uint8_t { Capture, Target, Bubble };

    // Return value of event handler.  If true -> stop propagation.
    using EventResult = bool;

    // -----------------------------------------------------------------------------
    // Base element interface
    class UIElement : public std::enable_shared_from_this<UIElement> {
    public:
        virtual ~UIElement() = default;

        // Layout input properties (margins/padding could be added later)
        MMath::FLOAT2     preferredSize{ 100.f, 30.f };   // default arbitrary
        bool              visible{ true };

        // Final computed rect after layout
        Rect              rect{};

        // Hierarchy
        std::weak_ptr<UIContainer> parent;

        // Rendering
        virtual void Draw(GUIRenderer& renderer) {}

        // Event dispatch (Capture -> Target -> Bubble)
        virtual EventResult OnEvent(const UIEvent& e, PropagationPhase phase) { (void)e; (void)phase; return false; }
    };

    // -----------------------------------------------------------------------------
    // Container element – holds children and delegates Draw/Layout/Event
    class UIContainer : public UIElement {
    public:
        std::vector<std::shared_ptr<UIElement>> children;

        void Add(const std::shared_ptr<UIElement>& child) {
            child->parent = shared_from_this();
            children.emplace_back(child);
        }

        // Layout is deferred to UILayoutEngine
        void Draw(GUIRenderer& renderer) override {
            for (auto& c : children) if (c->visible) c->Draw(renderer);
        }

        EventResult OnEvent(const UIEvent& e, PropagationPhase phase) override {
            // Capture phase: traverse children first
            if (phase == PropagationPhase::Capture) {
                for (auto it = children.rbegin(); it != children.rend(); ++it) {
                    if ((*it)->OnEvent(e, phase)) return true;
                }
            }
            // Target/Bubble handled by dispatcher later
            return false;
        }
    };

 

    // -----------------------------------------------------------------------------
    // Concrete widget example – Button
    class UIButton : public UIElement {
    public:
        std::string text;

        // Exposed signal
        FDelegate<void()> OnClicked;

        void Draw(GUIRenderer& renderer) override;

        EventResult OnEvent(const UIEvent& e, PropagationPhase phase) override {
            if (!visible) return false;
            if (phase != PropagationPhase::Target) return false;

            if (std::holds_alternative<Click>(e)) {
                OnClicked.BlockingBroadCast();
                return true; // consume event
            }
            return false;
        }
    };

    // -----------------------------------------------------------------------------
    // UI context – owned by GUILayer
    struct UIContext {
        std::shared_ptr<UIContainer> root;
        GUIRenderer* renderer = nullptr; // injected

        // Dispatch entry
        void ProcessInput(const std::vector<InputEvent>& inputEvents);

    private:
        void dispatch(UIElement* target, const UIEvent& ev);
    };

    // -----------------------------------------------------------------------------
    // GUIRenderer interface – implemented per?backend (e.g., Direct3D12)
    class GUIRenderer {
    public:
        virtual ~GUIRenderer() = default;
        virtual void DrawQuad(const Rect& r, const MMath::FLOAT4& color) = 0;
        virtual void DrawText(const Rect& r, const std::string& text) = 0;
    };

    // Example stub (fill in your D3D12 drawing calls)
    class D3D12GUIRenderer : public GUIRenderer {
    public:
        explicit D3D12GUIRenderer(D3D12HelloRenderer* r) : m_backend(r) {}
        void DrawQuad(const Rect& r, const MMath::FLOAT4& color) override;
        void DrawText(const Rect& r, const std::string& text)   override;
    private:
        D3D12HelloRenderer* m_backend{};
    };

    // -----------------------------------------------------------------------------
    // Implementation stubs (place in .cpp)
    inline void UIButton::Draw(GUIRenderer& renderer) {
        MMath::FLOAT4 clr{ 0.2f, 0.2f, 0.2f, 1.0f };
        renderer.DrawQuad(rect, clr);
        renderer.DrawText(rect, text);
    }

    // -----------------------------------------------------------------------------
    // GUILayer glue – quick sketch (should live in its own .cpp/.h but shown here)
    class GUILayerImpl : public GUILayer {
    public:
        void onInit() override {
            ctx.renderer = new D3D12GUIRenderer(GameApplication::GetInstance()->m_renderer);
            ctx.root = std::make_shared<UIStackPanel>(LayoutMode::Vertical);

            auto btn = std::make_shared<UIButton>();
            btn->text = "Start Game";
            btn->OnClicked.Add([] {
                // inject param to GameFlow
                Param::SetBool("StartClicked", true);
                });
            ctx.root->Add(btn);
        }
        void onUpdate() override {
            // convert InputSystem events to UIEvent list then call ctx.ProcessInput(...)
            ctx.root->Draw(*ctx.renderer);
        }
        void onDestroy() override { delete ctx.renderer; }
    private:
        UIContext ctx;
    };

} // namespace ui

