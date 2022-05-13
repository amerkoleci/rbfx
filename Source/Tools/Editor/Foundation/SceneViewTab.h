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

#pragma once

#include "../Core/ResourceDragDropPayload.h"
#include "../Project/ProjectEditor.h"
#include "../Project/ResourceEditorTab.h"

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Utility/SceneRendererToTexture.h>

namespace Urho3D
{

void Foundation_SceneViewTab(Context* context, ProjectEditor* projectEditor);

/// Interface of Camera controller used by Scene.
class SceneCameraController : public Object
{
    URHO3D_OBJECT(SceneCameraController, Object);

public:
    explicit SceneCameraController(Scene* scene, Camera* camera);
    ~SceneCameraController() override;

    /// Return name in UI.
    virtual ea::string GetTitle() const = 0;
    /// Return whether the mouse should be hidden.
    virtual bool IsMouseHidden() { return false; }
    /// Update controller for given camera object.
    virtual void Update() = 0;

protected:
    const WeakPtr<Scene> scene_;
    const WeakPtr<Camera> camera_;
};

using SceneCameraControllerPtr = SharedPtr<SceneCameraController>;

/// Description of camera controller for SceneViewTab.
struct SceneCameraControllerDesc
{
    ea::string name_;
    ea::function<SceneCameraControllerPtr(Scene* scene, Camera* camera)> factory_;
};

/// Single page of SceneViewTab.
struct SceneViewPage
{
    SharedPtr<Scene> scene_;
    SharedPtr<SceneRendererToTexture> renderer_;
    ea::vector<SceneCameraControllerPtr> cameraControllers_;
    ea::string cfgFileName_;

    /// UI state
    /// @{
    unsigned currentCameraController_{};
    /// @}
};

/// Tab that renders Scene and enables Scene manipulation.
class SceneViewTab : public ResourceEditorTab
{
    URHO3D_OBJECT(SceneViewTab, ResourceEditorTab);

public:
    explicit SceneViewTab(Context* context);
    ~SceneViewTab() override;

    /// Register new type of camera controller. Should be called before any scenes are loaded.
    void RegisterCameraController(const SceneCameraControllerDesc& desc);
    template <class T> void RegisterCameraController();

    /// ResourceEditorTab implementation
    /// @{
    ea::string GetResourceTitle() { return "Scene"; }
    bool SupportMultipleResources() { return true; }
    bool CanOpenResource(const OpenResourceRequest& request) override;
    /// @}

protected:
    /// ResourceEditorTab implementation
    /// @{
    void UpdateAndRenderContextMenuItems() override;
    void ApplyHotkeys(HotkeyManager* hotkeyManager) override;

    void OnResourceLoaded(const ea::string& resourceName) override;
    void OnResourceUnloaded(const ea::string& resourceName) override;
    void OnActiveResourceChanged(const ea::string& resourceName) override;
    void OnResourceSaved(const ea::string& resourceName) override;

    void UpdateAndRenderContent() override;
    /// @}

private:
    IntVector2 GetContentSize() const;

    SceneViewPage* GetPage(const ea::string& resourceName);
    SceneViewPage* GetActivePage();
    SceneViewPage CreatePage(Scene* scene) const;

    void SavePageConfig(const SceneViewPage& page) const;
    void LoadPageConfig(SceneViewPage& page) const;

    ea::vector<SceneCameraControllerDesc> cameraControllers_;
    ea::unordered_map<ea::string, SceneViewPage> scenes_;
};

template <class T>
void SceneViewTab::RegisterCameraController()
{
    SceneCameraControllerDesc desc;
    desc.name_ = T::GetTypeNameStatic();
    desc.factory_ = [](Scene* scene, Camera* camera) { return MakeShared<T>(scene, camera); };
    RegisterCameraController(desc);
}

}