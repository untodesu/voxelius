#ifndef F5A3A7CE_B769_4286_A715_E64D2971E7A5
#define F5A3A7CE_B769_4286_A715_E64D2971E7A5

#include "client/gui/widget.hh"

namespace gui
{
template<typename Derived>
class ContainerBuilder : public WidgetBuilder<Derived> {
public:
    Derived& add_child(Widget& child);
    virtual void layout(void) override;
    virtual void translate(void) override;

private:
    std::vector<Widget*> m_children {};
};
} // namespace gui

namespace gui
{
class Container final : public ContainerBuilder<Container> {};
} // namespace gui

template<typename Derived>
Derived& gui::ContainerBuilder<Derived>::add_child(Widget& child)
{
    m_children.push_back(&child);

    return this->self();
}

template<typename Derived>
void gui::ContainerBuilder<Derived>::layout(void)
{
    for(auto child : m_children) {
        if(child->visible()) {
            child->layout();
        }
    }
}

template<typename Derived>
void gui::ContainerBuilder<Derived>::translate(void)
{
    for(auto child : m_children) {
        child->translate();
    }
}

#endif /* F5A3A7CE_B769_4286_A715_E64D2971E7A5 */
