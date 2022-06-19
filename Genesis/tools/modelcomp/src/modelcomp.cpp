// Copyright 2022 Pedro Nunes
//
// This file is part of Genesis.
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
// along with Genesis. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "modelcomp.hpp"

// clang-format off
#include <externalheadersbegin.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <bitsery/bitsery.h>
#include <bitsery/adapter/stream.h>
#include <bitsery/traits/string.h>
#include <json.hpp>
#include <externalheadersend.hpp>
// clang-format on

#include <fstream>
#include <log.hpp>
#include "material.hpp"
#include "modelserialization.hpp"

namespace Genesis
{
namespace ResComp
{

ModelComp::ModelComp() {}
ModelComp::~ModelComp() {}

int ModelComp::Run()
{
    if (ReadAsset(GetFile()) == false)
    {
        OnAssetCompilationFailed(GetFile(), "Invalid model asset file.");
        return -1;
    }
    
    if (std::filesystem::exists(m_SourceModelPath) == false)
    {
        OnAssetCompilationFailed(GetFile(), "Model file doesn't exist.");
        return -1;
    }

    Assimp::Importer importer;
    const int flags = aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_JoinIdenticalVertices | aiProcess_FixInfacingNormals | aiProcess_SortByPType |
                      aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph;
    const aiScene* pScene = importer.ReadFile(m_SourceModelPath.generic_string(), flags);
    if (pScene == nullptr)
    {
        OnAssetCompilationFailed(GetFile(), importer.GetErrorString());
        return -1;
    }

    if (ValidateMaterials(pScene) == false)
    {
        OnAssetCompilationFailed(GetFile(), "Material validation failed.");
        return -1;
    }

    std::filesystem::path targetModelPath = GetTargetModelPath(m_SourceModelPath);
    if (Compile(pScene, targetModelPath))
    {
        OnResourceBuilt(GetFile(), m_SourceModelPath, targetModelPath);
        return 0;
    }
    else
    {
        OnAssetCompilationFailed(GetFile(), "Failed to compile file.");
        return -1;
    }
}

bool ModelComp::ReadAsset(const std::filesystem::path& assetPath) 
{
    using namespace nlohmann;
    std::ifstream file(assetPath);
    if (file.good())
    {
        json j;
        file >> j;
        file.close();

        json::iterator it = j.find("source");
        if (it == j.end() || it->is_string() == false)
        {
            Core::Log::Error() << "Couldn't find required 'source' field in asset " << assetPath;
            return false;
        }
        else
        {
            m_SourceModelPath = assetPath;
            m_SourceModelPath = m_SourceModelPath.remove_filename() / it->get<std::string>();
        }

        it = j.find("materials");
        if (it == j.end() || it->is_array() == false)
        {
            Core::Log::Error() << "Couldn't find required 'materials' array in asset " << assetPath;
            return false;
        }
        else
        {
            for (json& jMaterial : *it)
            {
                MaterialUniquePtr pMaterial = std::make_unique<Material>();

                json::iterator materialIt = jMaterial.find("name");
                if (materialIt == jMaterial.end() || materialIt->is_string() == false)
                {
                    Core::Log::Error() << "Couldn't find material name in asset " << assetPath;
                    return false;
                }
                else
                {
                    pMaterial->SetName(materialIt->get<std::string>());
                }

                materialIt = jMaterial.find("shader");
                if (materialIt == jMaterial.end() || materialIt->is_string() == false)
                {
                    Core::Log::Error() << "Couldn't find required 'shader' string for material " << pMaterial->GetName() << " in asset " << assetPath;
                    return false;
                }
                else
                {
                    pMaterial->SetShader(materialIt->get<std::string>());
                }

                materialIt = jMaterial.find("bindings");
                if (materialIt == jMaterial.end() || materialIt->is_array() == false)
                {
                    Core::Log::Error() << "Couldn't find required 'bindings' array for material " << pMaterial->GetName() << " in asset " << assetPath;
                    return false;
                }
                else
                {
                    Material::Bindings& bindings = pMaterial->GetBindings();
                    for (json& jBinding : *materialIt)
                    {
                        json::iterator bindingIt = jBinding.find("name");
                        if (bindingIt == jBinding.end() || bindingIt->is_string() == false)
                        {
                            Core::Log::Error() << "Couldn't find required 'name' string in bindings for material " << pMaterial->GetName() << " in asset " << assetPath;
                            return false;
                        }
                        std::string bindingName = bindingIt->get<std::string>();

                        bindingIt = jBinding.find("filename");
                        if (bindingIt == jBinding.end() || bindingIt->is_string() == false)
                        {
                            Core::Log::Error() << "Couldn't find required 'filename' string in bindings for material " << pMaterial->GetName() << " in asset " << assetPath;
                            return false;
                        }
                        std::string bindingFilename = bindingIt->get<std::string>();

                        bindings[bindingName] = bindingFilename;
                    }
                }
                
                m_Materials[pMaterial->GetName()] = std::move(pMaterial);
            }
        }

        return true;
    }
    return false;
}

bool ModelComp::ValidateMaterials(const aiScene* pScene) 
{
    bool mismatch = false;
    if (m_Materials.size() != pScene->mNumMaterials)
    {
        Core::Log::Error() << "Different number of materials between model file and asset.";
        mismatch = true;
    }

    for (auto& pMaterial : m_Materials)
    {
        bool found = false;
        for (unsigned int i = 0; i < pScene->mNumMaterials; ++i)
        {
            if (pMaterial.first == pScene->mMaterials[i]->GetName().C_Str())
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            mismatch = true;
            break;
        }
    }

    if (mismatch)
    {
        Core::Log::Error() << "Material mismatch between asset and model file.";

        Core::Log::Error() << "Asset:";
        for (auto& pMaterial : m_Materials)
        {
            Core::Log::Error() << "- " << pMaterial.first;
        }

        Core::Log::Error() << "Model file:";
        for (unsigned int i = 0; i < pScene->mNumMaterials; ++i)
        {
            Core::Log::Error() << "- " << pScene->mMaterials[i]->GetName().C_Str();
        }
    }

    return !mismatch;
}

std::filesystem::path ModelComp::GetTargetModelPath(const std::filesystem::path& sourceModelPath) const
{
    std::filesystem::path targetPath = GetDataDir() / std::filesystem::relative(GetFile(), GetAssetsDir()).remove_filename();
    std::filesystem::create_directories(targetPath);
    const std::string targetFileName = sourceModelPath.stem().string() + ".gmdl";
    return targetPath / targetFileName;
}

bool ModelComp::Compile(const aiScene* pImportedScene, std::filesystem::path& targetModelPath)
{
    std::ofstream file(targetModelPath, std::ios::out | std::ios::trunc | std::ios::binary);
    if (file.good())
    {
        Serialization::Model model;
        WriteHeader(model, pImportedScene);
        WriteMaterials(model);
        WriteMeshes(model, pImportedScene);

        bitsery::Serializer<bitsery::OutputBufferedStreamAdapter> ser{file};
        ser.object(model);
        ser.adapter().flush();

        file.close();
        return true;
    }
    else
    {
        return false;
    }
}

void ModelComp::WriteHeader(Serialization::Model& model, const aiScene* pImportedScene)
{
    model.header.format = "GMDL";
    model.header.version = 1;
    model.header.materials = static_cast<uint8_t>(pImportedScene->mNumMaterials);
    model.header.meshes = static_cast<uint8_t>(pImportedScene->mNumMeshes);
}

void ModelComp::WriteMaterials(Serialization::Model& model) 
{
    for (auto& materialPair : m_Materials)
    {
        Material* pMaterial = materialPair.second.get();
        Serialization::ModelMaterial material;
        material.name = pMaterial->GetName();
        material.shader = pMaterial->GetShader();
        material.bindings = pMaterial->GetBindings();
        model.materials.push_back(std::move(material));
    }
}

void ModelComp::WriteMeshes(Serialization::Model& model, const aiScene* pImportedScene)
{
    for (unsigned int i = 0; i < pImportedScene->mNumMeshes; ++i)
    {
        const aiMesh* pImportedMesh = pImportedScene->mMeshes[i];
        Serialization::Mesh mesh;
        WriteMeshHeader(mesh, pImportedMesh);
        WriteMesh(mesh, pImportedMesh);
        model.meshes.push_back(std::move(mesh));
    }
}

void ModelComp::WriteMeshHeader(Serialization::Mesh& mesh, const aiMesh* pImportedMesh)
{
    mesh.header.materialIndex = pImportedMesh->mMaterialIndex;
    mesh.header.vertices = pImportedMesh->mNumVertices;
    mesh.header.faces = pImportedMesh->mNumFaces;
    mesh.header.uvChannels = pImportedMesh->GetNumUVChannels();
}

void ModelComp::WriteMesh(Serialization::Mesh& mesh, const aiMesh* pImportedMesh)
{
    mesh.vertices.reserve(mesh.header.vertices);
    for (uint32_t i = 0; i < mesh.header.vertices; ++i)
    {
        const aiVector3D& vertex = pImportedMesh->mVertices[i];
        mesh.vertices.push_back({vertex.x, vertex.y, vertex.z});
    }

    mesh.faces.reserve(mesh.header.faces);
    for (uint32_t i = 0; i < mesh.header.faces; ++i)
    {
        const aiFace& face = pImportedMesh->mFaces[i];
        for (uint32_t j = 0; j < 3; ++j)
        {
            mesh.faces.push_back({face.mIndices[0], face.mIndices[1], face.mIndices[2]});
        }
    }

    mesh.uvChannels.reserve(mesh.header.uvChannels);
    for (uint32_t i = 0; i < mesh.header.uvChannels; ++i)
    {
        Serialization::UVChannel uvChannel;
        for (uint32_t j = 0; j < mesh.header.vertices; ++j)
        {
            const aiVector3D& uv = pImportedMesh->mTextureCoords[i][j];
            uvChannel.uvs.push_back({uv.x, uv.y});
        }
        mesh.uvChannels.push_back(std::move(uvChannel));
    }

    mesh.normals.reserve(mesh.header.vertices);
    for (uint32_t i = 0; i < mesh.header.vertices; ++i)
    {
        const aiVector3D& normal = pImportedMesh->mNormals[i];
        mesh.normals.push_back({normal.x, normal.y, normal.z});
    }
}

} // namespace ResComp
} // namespace Genesis
