#pragma once

namespace Challenge::Tests {

    //! Used to call virtually registered callback from mock-ups
    template<typename _CallbackType>
    class CallbackArgument {
    public:
        bool registerCallback(_CallbackType _callback, ... ) {
            bool result = m_callback != nullptr;
            m_callback = _callback;
            return result;
        }

        void fireCallback() { if ( m_callback ){ m_callback(); } }

        bool isValid() const { return m_callback != nullptr; }

    private:
        _CallbackType m_callback;
    };
} // namespace Challenge::Tests
