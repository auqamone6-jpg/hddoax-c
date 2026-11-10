# vendor/

## Dear ImGui

This project requires Dear ImGui (v1.89.x or later).

### Setup

1. Download or clone ImGui from https://github.com/ocornut/imgui
2. Place the contents so the layout is:

```
vendor/
  imgui/
    imgui.h
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui_internal.h
    imconfig.h
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h
    backends/
      imgui_impl_win32.h
      imgui_impl_win32.cpp
      imgui_impl_dx11.h
      imgui_impl_dx11.cpp
```

### Quick clone

```
cd vendor
git clone --depth 1 https://github.com/ocornut/imgui imgui
```

The vcxproj already references `vendor\imgui` and `vendor\imgui\backends`
as include directories, so no further configuration is needed.
