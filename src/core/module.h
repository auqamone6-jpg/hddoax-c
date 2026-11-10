#pragma once

#include <memory>
#include <vector>

// A feature. Each lives in its own .cpp under src/modules and registers itself
// with REGISTER_MODULE; nothing else needs editing to add one.
//
// All callbacks run on the render thread inside the Present hook.
class Module {
public:
    Module(const char* name, const char* category) : name_(name), category_(category) {}
    virtual ~Module() = default;

    const char* Name() const { return name_; }
    const char* Category() const { return category_; }
    bool Enabled() const { return enabled_; }
    void SetEnabled(bool enabled);

    // Called once after the renderer is up. Return false to hide the module
    // (e.g. a required function/offset was not found).
    virtual bool OnInit() { return true; }
    virtual void OnEnable() {}
    virtual void OnDisable() {}
    // Every frame while enabled, inside an ImGui frame (draw overlays here).
    virtual void OnFrame() {}
    // Settings widgets shown under the module's toggle in the menu.
    virtual void OnMenu() {}

private:
    const char* name_;
    const char* category_;
    bool enabled_ = false;
};

namespace Modules {
void Register(std::unique_ptr<Module> module);
void InitAll();
void FrameAll();
void ShutdownAll(); // disables every enabled module
const std::vector<std::unique_ptr<Module>>& All();
}

#define REGISTER_MODULE(Type)                                              \
    namespace {                                                            \
    const bool g_registered_##Type =                                       \
        (::Modules::Register(std::make_unique<Type>()), true);             \
    }
