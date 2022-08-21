// Copyright 2022 Pedro Nunes
//
// This file is part of Nullscape.
//
// Genesis is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Genesis is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Nullscape. If not, see <http://www.gnu.org/licenses/>.

#include "entity/components/modelcomponent.hpp"

#include <filesystem>

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <resources/resourcemodel.h>
#include <genesis.h>


namespace Nullscape
{

ModelComponent::ModelComponent()
    : m_Version(1)
    , m_pModel(nullptr)
{
}

ModelComponent::~ModelComponent() {}

void ModelComponent::Initialize() 
{
    using namespace Genesis;
    if (std::filesystem::exists(m_Filename) && std::filesystem::is_regular_file(m_Filename))
    {
        m_pModel = FrameWork::GetResourceManager()->GetResource<ResourceModel*>(m_Filename);
    }
    else
    {
        m_pModel = nullptr;
    }
}

void ModelComponent::Update(float delta) {}

void ModelComponent::UpdateDebugUI() 
{
    if (ImGui::InputText("Model", &m_Filename))
    {
        Initialize();
    }
}

void ModelComponent::Render()
{
    if (m_pModel != nullptr)
    {
        m_pModel->Render(glm::mat4(1.0f));
    }
}

} // namespace Nullscape
