#pragma once
#include "sol/sol.hpp"
#include "runtime/function/framework/component/component.h"

namespace Piccolo
{
    // 需要Piccolo_parser生成反射代码和序列化相关的代码
    REFLECTION_TYPE(LuaComponent)
    CLASS(LuaComponent : public Component, WhiteListFields)
    {
        REFLECTION_BODY(LuaComponent)

    public:
        LuaComponent() = default;

        // 序列化的时候需要的一个函数
        void postLoadResource(std::weak_ptr<GObject> parent_object) override;

        void tick(float delta_time) override;

        template<typename T>
        static void set(std::weak_ptr<GObject> game_object, const char* name, T value);

        template<typename T>
        static T get(std::weak_ptr<GObject> game_object, const char* name);

        static void invoke(std::weak_ptr<GObject> game_object, const char* name);
    protected:
        sol::state m_lua_state;
        META(Enable)                // 提醒Piccolo_parser序列化该字段
        std::string m_lua_script;   // Lua脚本
    };
} // namespace Piccolo
