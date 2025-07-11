// -----------------------------------------------------------------------------
 // Simple layout strategies (just Flex & Absolute for now)
enum class LayoutMode { Absolute, Horizontal, Vertical };  // extend to Flex/Grid later

struct LayoutParams {
    LayoutMode mode = LayoutMode::Absolute;
    float spacing = 4.f; // for linear layouts
};

class UILayoutEngine {
public:
    static void Layout(const std::shared_ptr<UIContainer>& root, const Rect& bounds) {
        switch (root->layout.mode) {
        case LayoutMode::Absolute: layoutAbsolute(root, bounds); break;
        case LayoutMode::Horizontal: layoutLinear(root, bounds, /*vertical?*/false); break;
        case LayoutMode::Vertical:   layoutLinear(root, bounds, /*vertical?*/true);  break;
        }
    }
private:
    static void layoutAbsolute(const std::shared_ptr<UIContainer>& node, const Rect& bounds) {
        node->rect = bounds;
        for (auto& c : node->children) {
            // assume child already has rect set by authoring; nothing to do.
            if (auto cc = std::dynamic_pointer_cast<UIContainer>(c))
                UILayoutEngine::Layout(cc, c->rect);
        }
    }
    static void layoutLinear(const std::shared_ptr<UIContainer>& node, const Rect& bounds, bool vertical) {
        node->rect = bounds;
        float cursor = vertical ? bounds.y : bounds.x;
        for (auto& c : node->children) {
            float w = vertical ? bounds.w : c->preferredSize.x();
            float h = vertical ? c->preferredSize.y() : bounds.h;
            c->rect = { vertical ? bounds.x : cursor, vertical ? cursor : bounds.y, w, h };
            cursor += (vertical ? h : w) + node->layout.spacing;
            if (auto cc = std::dynamic_pointer_cast<UIContainer>(c))
                UILayoutEngine::Layout(cc, c->rect);
        }
    }
};

// Each container may carry layout params
struct LayoutMixin { LayoutParams layout; };

// Extend UIContainer with mixin (simple multiple inheritance for now)
class UIStackPanel : public UIContainer, public LayoutMixin {
public:
    UIStackPanel(LayoutMode dir = LayoutMode::Vertical) { layout.mode = dir; }
};