//
// Copyright (c) 2017-2020 the rbfx project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "../../Foundation/SceneViewTab/CreatePrefabFromNode.h"

#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Graphics/TextureCube.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/SystemUI/SystemUI.h>

namespace Urho3D
{

void Foundation_CreatePrefabFromNode(Context* context, SceneViewTab* sceneViewTab)
{
    sceneViewTab->RegisterAddon<CreatePrefabFromNode>();
}

PrefabFromNodeFactory::PrefabFromNodeFactory(Context* context)
    : BaseResourceFactory(context, 0, "Prefab from Node")
{
}

void PrefabFromNodeFactory::SetNodes(const WeakNodeVector& nodes)
{
    nodes_ = nodes;
}

ea::string PrefabFromNodeFactory::GetDefaultFileName() const
{
    if (nodes_.empty())
        return "(none)";
    else if (nodes_.size() > 1)
        return "(automatic)";

    const ea::string& name = nodes_[0]->GetName();
    if (name.empty())
        return "Prefab.xml";
    return Format("{}.xml", name);
}

bool PrefabFromNodeFactory::IsFileNameEditable() const
{
    return nodes_.size() == 1;
}

void PrefabFromNodeFactory::Render(const FileNameChecker& checker, bool& canCommit, bool& shouldCommit)
{
    BaseClassName::Render(checker, canCommit, shouldCommit);

    if (nodes_.size() > 1)
        canCommit = true;
    if (!IsFileNameEditable() && ui::IsKeyPressed(KEY_RETURN))
        shouldCommit = true;
}

void PrefabFromNodeFactory::CommitAndClose()
{
    BaseClassName::CommitAndClose();

    if (nodes_.size() == 1 && nodes_[0])
    {
        SaveNodeAsPrefab(nodes_[0], GetFinalFileName());
    }
    else
    {
        const ea::string& filePath = GetFinalFilePath();
        for (Node* node : nodes_)
        {
            if (!node)
                continue;

            const ea::string fileName = FindBestFileName(node, filePath);
            if (!fileName.empty())
                SaveNodeAsPrefab(node, fileName);
        }
    }
}

ea::string PrefabFromNodeFactory::FindBestFileName(Node* node, const ea::string& filePath) const
{
    const auto getAvailableFileName = [&](const ea::string& prefabName) -> ea::optional<ea::string>
    {
        auto fs = GetSubsystem<FileSystem>();
        const ea::string& fileName = filePath + prefabName + ".xml";
        if (fs->FileExists(fileName) || fs->DirExists(fileName))
            return ea::nullopt;

        return fileName;
    };

    ea::string prefabName = GetSanitizedName(node->GetName()).trimmed();
    if (prefabName.empty())
        prefabName = "Prefab";

    if (const auto fileName = getAvailableFileName(prefabName))
        return *fileName;

    const int maxAttempts = 100;
    for (int i = 1; i < maxAttempts; ++i)
    {
        if (const auto fileName = getAvailableFileName(Format("{}_{}", prefabName, i)))
            return *fileName;
    }

    URHO3D_LOGERROR("Cannot find available file name for prefab");
    return "";
}

void PrefabFromNodeFactory::SaveNodeAsPrefab(Node* node, const ea::string& fileName)
{
    auto scene = MakeShared<Scene>(context_);

    SetupPrefabScene(scene, node);

    auto xmlFile = MakeShared<XMLFile>(context_);
    XMLElement rootElement = xmlFile->CreateRoot("scene");
    if (scene->SaveXML(rootElement))
        xmlFile->SaveFile(fileName);
}

void PrefabFromNodeFactory::SetupPrefabScene(Scene* scene, Node* node)
{
    auto cache = GetSubsystem<ResourceCache>();

    scene->CreateComponent<Octree>();

    auto prefabNode = node->Clone(scene);

    auto skyboxNode = scene->CreateChild("Default Skybox");
    auto skybox = skyboxNode->CreateComponent<Skybox>();
    skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
    skybox->SetMaterial(cache->GetResource<Material>("Materials/DefaultSkybox.xml"));

    auto zoneNode = scene->CreateChild("Default Zone");
    auto zone = zoneNode->CreateComponent<Zone>();
    zone->SetBoundingBox(BoundingBox{-1000.0f, 1000.0f});
    zone->SetAmbientColor(Color::BLACK);
    zone->SetBackgroundBrightness(1.0f);
    zone->SetZoneTexture(cache->GetResource<TextureCube>("Textures/DefaultSkybox.xml"));
}

CreatePrefabFromNode::CreatePrefabFromNode(SceneViewTab* owner)
    : SceneViewAddon(owner)
    , factory_(MakeShared<PrefabFromNodeFactory>(context_))
{
    owner->OnSelectionEditMenu.Subscribe(this, &CreatePrefabFromNode::RenderMenu);
}

void CreatePrefabFromNode::RenderMenu(SceneViewPage& page, Scene* scene, SceneSelection& selection)
{
    const bool hasNodesSelected = !selection.GetNodes().empty();
    if (ui::MenuItem("Create Prefab", nullptr, false, hasNodesSelected))
        CreatePrefabs(selection);
}

void CreatePrefabFromNode::CreatePrefabs(const SceneSelection& selection)
{
    const auto& selectedNodes = selection.GetNodes();

    PrefabFromNodeFactory::WeakNodeVector nodes{selectedNodes.begin(), selectedNodes.end()};
    for (auto& node : nodes)
    {
        const auto isParentOfNode = [&node](const WeakPtr<Node>& otherNode) { return node->IsChildOf(otherNode); };
        const bool isChildNode = ea::any_of(nodes.begin(), nodes.end(), isParentOfNode);
        if (isChildNode)
            node = nullptr;
    }
    ea::erase(nodes, nullptr);

    factory_->SetNodes(nodes);

    auto project = owner_->GetProject();
    auto request = MakeShared<CreateResourceRequest>(factory_);
    project->ProcessRequest(request, owner_);

}

}
