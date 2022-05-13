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

#include "../Foundation/SceneViewTab.h"

namespace Urho3D
{

void Foundation_DefaultCameraController3D(Context* context, SceneViewTab* sceneViewTab);

/// Interface of Camera controller used by Scene.
class DefaultCameraController3D : public SceneCameraController
{
    URHO3D_OBJECT(DefaultCameraController3D, SceneCameraController);

public:
    explicit DefaultCameraController3D(Scene* scene, Camera* camera);

    /// Implement SceneCameraController.
    /// @{
    void SerializeInBlock(Archive& archive) override {}

    ea::string GetTitle() const override { return "3D Camera"; }
    bool IsMouseHidden() { return false; }
    void Update() {}
    /// @}
};

}