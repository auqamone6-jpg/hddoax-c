#include "core/menu.h"

#include <imgui.h>

#include <cstring>
#include <vector>

#include "core/module.h"

namespace {
bool g_open = true;
}

bool Menu::IsOpen() { return g_open; }
void Menu::Toggle() { g_open = !g_open; }

void Menu::Draw() {
    if (!g_open) return;

    ImGui::SetNextWindowSize(ImVec2(520, 380), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("fnutil", &g_open, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    // One tab per category, in first-registration order.
    std::vector<const char*> categories;
    for (auto& m : Modules::All()) {
        bool seen = false;
        for (auto* c : categories) seen |= std::strcmp(c, m->Category()) == 0;
        if (!seen) categories.push_back(m->Category());
    }

    if (ImGui::BeginTabBar("categories")) {
        for (auto* category : categories) {
            if (!ImGui::BeginTabItem(category)) continue;
            for (auto& m : Modules::All()) {
                if (std::strcmp(m->Category(), category) != 0) continue;
                ImGui::PushID(m.get());
                bool enabled = m->Enabled();
                if (ImGui::Checkbox(m->Name(), &enabled)) m->SetEnabled(enabled);
                ImGui::Indent();
                m->OnMenu();
                ImGui::Unindent();
                ImGui::PopID();
                ImGui::Separator();
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::TextDisabled("INSERT: menu   END: unload");
    ImGui::End();
}
