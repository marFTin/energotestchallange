#pragma once

#include <functional>

namespace Challenge {

    //! Similar to BOOST_SCOPE, but I don't want to introduce boost here only because this simple tool
    class ScopedAction{
    public:
        ScopedAction( std::function<void()> _action) : m_action(_action) {}
        ~ScopedAction() { if (m_action)m_action(); }

    private:
        std::function<void()> m_action;
    };
}
