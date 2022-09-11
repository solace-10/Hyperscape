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

#include "entity/components/trailcomponent.hpp"

#include <imgui/imgui.h>
#include <genesis.h>

#include "entity/components/transformcomponent.hpp"
#include "entity/entity.hpp"
#include "sector/sector.h"
#include "trail/trailmanager.h"
#include "trail/trail.h"
#include "game.hpp"

namespace Nullscape
{

TrailComponent::TrailComponent()
    : m_Offset(0.0f)
    , m_Width(1.0f)
    , m_Decay(1.0f)
    , m_Color(1.0f)
{

}

TrailComponent::~TrailComponent()
{
    TrailSharedPtr pTrail = m_pTrail.lock();
    if (pTrail)
    {
        pTrail->SetOrphan();
    }
}

void TrailComponent::Update(float delta)
{
    if (m_pTrail.expired() && g_pGame->GetCurrentSector())
    {
        m_pTrail = g_pGame->GetCurrentSector()->GetTrailManager()->Add(m_Width, m_Decay, m_Color);
    }

    TransformComponent* pTransformComponent = GetOwner()->GetComponent<TransformComponent>();
    if (pTransformComponent)
    {
        TrailSharedPtr pTrail = m_pTrail.lock();
        if (pTrail)
        {
            pTrail->AddPoint(pTransformComponent->GetPosition());
        }
    }
}

void TrailComponent::UpdateDebugUI()
{
    ImGui::SliderFloat("Width", &m_Width, 0.0f, 20.0f);
    ImGui::SliderFloat("Decay", &m_Decay, 0.1f, 20.0f);
}

void TrailComponent::Render() 
{

}

bool TrailComponent::Serialize(nlohmann::json& data)
{
    bool success = Component::Serialize(data);
    Component::Serialize(data, "offset", m_Offset);
    Component::Serialize(data, "width", m_Width);
    Component::Serialize(data, "decay", m_Decay);
    Component::Serialize(data, "color", m_Color);
    return success;
}

bool TrailComponent::Deserialize(const nlohmann::json& data)
{
    bool success = Component::Deserialize(data);
    success &= TryDeserialize(data, "offset", m_Offset);
    success &= TryDeserialize(data, "width", m_Width);
    success &= TryDeserialize(data, "decay", m_Decay);
    success &= TryDeserialize(data, "color", m_Color);
    return success;
}

void TrailComponent::CloneFrom(Component* pComponent)
{
    Component::CloneFrom(pComponent);
    TrailComponent* pOtherComponent = reinterpret_cast<TrailComponent*>(pComponent);
    m_Offset = pOtherComponent->m_Offset;
    m_Width = pOtherComponent->m_Width;
    m_Decay = pOtherComponent->m_Decay;
    m_Color = pOtherComponent->m_Color;
}

} // namespace Nullscape
