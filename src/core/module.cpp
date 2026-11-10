#include "core/module.h"

#include <algorithm>

#include "core/log.h"

namespace {
// Function-local so registration from static initialisers is order-safe.
std::vector<std::unique_ptr<Module>>& List() {
    static std::vector<std::unique_ptr<Module>> list;
    return list;
}
}

void Module::SetEnabled(bool enabled) {
    if (enabled == enabled_) return;
    enabled_ = enabled;
    if (enabled) OnEnable();
    else OnDisable();
}

void Modules::Register(std::unique_ptr<Module> module) { List().push_back(std::move(module)); }

void Modules::InitAll() {
    auto& list = List();
    std::erase_if(list, [](const std::unique_ptr<Module>& m) {
        if (m->OnInit()) return false;
        LOG("[modules] '%s' failed to initialise, hidden", m->Name());
        return true;
    });
    LOG("[modules] %zu module(s) ready", list.size());
}

void Modules::FrameAll() {
    for (auto& m : List())
        if (m->Enabled()) m->OnFrame();
}

void Modules::ShutdownAll() {
    for (auto& m : List()) m->SetEnabled(false);
}

const std::vector<std::unique_ptr<Module>>& Modules::All() { return List(); }
