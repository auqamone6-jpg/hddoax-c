#include <imgui.h>

#include "core/module.h"

// Corner readout of frame rate and frame time.
class PerfOverlay final : public Module {
public:
    PerfOverlay() : Module("Performance overlay", "Misc") {}

    void OnFrame() override {
        const ImGuiIO& io = ImGui::GetIO();
        const float pad = 10.0f;
        const ImVec2 pos(corner_ & 1 ? io.DisplaySize.x - pad : pad,
                         corner_ & 2 ? io.DisplaySize.y - pad : pad);
        const ImVec2 pivot(corner_ & 1 ? 1.0f : 0.0f, corner_ & 2 ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(pos, ImGuiCond_Always, pivot);
        ImGui::SetNextWindowBgAlpha(0.45f);
        const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
                                       ImGuiWindowFlags_AlwaysAutoResize |
                                       ImGuiWindowFlags_NoSavedSettings |
                                       ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        if (ImGui::Begin("##perf", nullptr, flags))
            ImGui::Text("%.0f fps  %.2f ms", io.Framerate, 1000.0f / io.Framerate);
        ImGui::End();
    }

    void OnMenu() override {
        ImGui::SetNextItemWidth(160);
        ImGui::Combo("Corner", &corner_, "Top left\0Top right\0Bottom left\0Bottom right\0");
    }

private:
    int corner_ = 1;
};

REGISTER_MODULE(PerfOverlay)
