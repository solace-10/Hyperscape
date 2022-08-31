// Copyright 2022 Pedro Nunes
//
// This file is part of Nullscape.
//
// Nullscape is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Nullscape is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Nullscape. If not, see <http://www.gnu.org/licenses/>.

#include <SDL_image.h>

#include "system/skybox.hpp"

#include "game.hpp"
#include "player.h"
#include "sector/sector.h"
#include "sector/sectorcamera.h"

#include <algorithm>
#include <configuration.h>
#include <genesis.h>
#include <math/misc.h>
#include <rendersystem.h>
#include <render/viewport.hpp>
#include <resources/resourceimage.h>
#include <resources/resourcemodel.h>
#include <shaderuniform.h>
#include <sstream>
#include <vertexbuffer.h>

namespace Nullscape
{

///////////////////////////////////////////////////////////////////////////////
// Skybox
///////////////////////////////////////////////////////////////////////////////

Skybox::Skybox(const std::string& seed)
    : m_pProteanCloudsShader(nullptr)
    , m_pShader(nullptr)
    , m_pVertexBuffer(nullptr)
    , m_AmbientColour(1.0)
    , m_ProteanCloudsGenerated(false)
{
    using namespace Genesis;

    ResourceManager* pResourceManager = FrameWork::GetResourceManager();

    m_pProteanCloudsShader = FrameWork::GetResourceManager()->GetResource<ResourceShader*>("data/shaders/proteanclouds.glsl");

    CreateGeometry();
    CreateCubemapTexture();

    m_pShader = FrameWork::GetResourceManager()->GetResource<ResourceShader*>("data/shaders/skybox.glsl");
    ShaderUniformSharedPtr pCubemapUniform = m_pShader->RegisterUniform("k_cubemap", ShaderUniformType::Cubemap);
    pCubemapUniform->Set(m_Cubemap, GL_TEXTURE0);
}

Skybox::~Skybox()
{
    delete m_pVertexBuffer;
    glDeleteTextures(1, &m_Cubemap);
}

void Skybox::CreateGeometry()
{
    using namespace Genesis;
    m_pVertexBuffer = new VertexBuffer(GeometryType::Triangle, VBO_POSITION | VBO_INDEX);

    const PositionData positions = {
        {-1.0f,  1.0f,  1.0f}, 
        {-1.0f, -1.0f,  1.0f},
        { 1.0f, -1.0f,  1.0f},
        { 1.0f,  1.0f,  1.0f},
        {-1.0f,  1.0f, -1.0f},
        {-1.0f, -1.0f, -1.0f},
        { 1.0f, -1.0f, -1.0f},
        { 1.0f,  1.0f, -1.0f}
    };
    m_pVertexBuffer->CopyPositions(positions);

    const IndexData indices = {
        0, 1, 2, 0, 2, 3, // Back
        4, 5, 6, 4, 6, 7, // Front
        0, 4, 1, 1, 4, 5, // Left
        2, 7, 6, 2, 3, 7, // Right
        0, 4, 7, 7, 3, 0, // Top
        1, 5, 6, 6, 2, 1, // Bottom
    };
    m_pVertexBuffer->CopyIndices(indices);
}

void Skybox::CreateCubemapTexture() 
{
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &m_Cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_Cubemap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA8, 2048, 2048);
}

void Skybox::Update(float delta)
{
    Genesis::SceneObject::Update(delta);
}

void Skybox::Render()
{
    using namespace Genesis;

    if (m_ProteanCloudsGenerated == false)
    {
        static int sCubemapResolution = 2048;
        RenderSystem* pRenderSystem = FrameWork::GetRenderSystem();
        Viewport* pPrimaryViewport = pRenderSystem->GetPrimaryViewport();
        
        pRenderSystem->ViewOrtho(sCubemapResolution, sCubemapResolution);
        glViewport(0, 0, sCubemapResolution, sCubemapResolution);
        m_pProteanCloudsShader->Use();

        VertexBuffer* vb = new VertexBuffer(GeometryType::Triangle, VBO_POSITION | VBO_UV);
        vb->CreateTexturedQuad(0.0f, 0.0f, static_cast<float>(sCubemapResolution), static_cast<float>(sCubemapResolution));

        GLuint fbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        for (int i = 0; i < 6; ++i)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_Cubemap, 0);
            vb->Draw();
        }

        delete vb;
        glDeleteFramebuffers(1, &fbo);

        pRenderSystem->SetRenderTarget(pPrimaryViewport->GetRenderTarget());
        m_ProteanCloudsGenerated = true;
    }

    glm::mat4 transform(1);
    Sector* pSector = g_pGame->GetCurrentSector();
    if (pSector != nullptr)
    {
        static float a = 0.0f;
        //a += 0.01f;
        transform = glm::rotate(a, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    transform = glm::scale(glm::vec3(100.0f));

    m_pShader->Use(transform);
    m_pVertexBuffer->Draw();
}

} // namespace Nullscape