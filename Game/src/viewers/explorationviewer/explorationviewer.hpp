// Copyright 2022 Pedro Nunes
//
// This file is part of Hyperscape.
//
// Hyperscape is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Hyperscape is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hyperscape. If not, see <http://www.gnu.org/licenses/>.

#pragma once

// clang-format off
#include <externalheadersbegin.hpp>
#include <glm/vec2.hpp>
#include <imgui/imgui.h>
#include <externalheadersend.hpp>
// clang-format on

#include <coredefines.h>

namespace Hyperscape
{

GENESIS_DECLARE_SMART_PTR(System);

class ExplorationViewer
{
public:
    ExplorationViewer();
    ~ExplorationViewer();

    void UpdateDebugUI();

    void View(SystemSharedPtr pSystem);

private:
    void DrawCanvas();
    void DrawScannerArc(const ImVec2& topLeft, const ImVec2& bottomRight, const ImVec2& offset);

    ImVec2 m_WindowSize;
    SystemWeakPtr m_pSystem;
    bool m_IsOpen;
    float m_Angle;
    float m_Aperture;
    float m_RangeMin;
    float m_RangeMax;
};

} // namespace Hyperscape