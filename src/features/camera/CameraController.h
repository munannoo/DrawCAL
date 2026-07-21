#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H
#include "raylib.h"
#include "rcamera.h"
#include "ui/themes/themes.h"
#include "raymath.h"
#include <vector>
#include <memory>




// orbit = move with shift mmb, rotate with mmb, walk = move with wasd, rotate with mouse
enum class cameraNavigationMode{Orbit,Walk};
enum class cameraView { Free = 0, Front, Top, Left, Right};
enum class editorViewMode { Single, Split };

extern editorViewMode currentViewMode;

class cameraController
{
private:
    Camera3D camera;

    // By default in NavigationMode
    cameraNavigationMode navigationMode = cameraNavigationMode::Orbit;
	cameraView currentView = cameraView::Free;

    // Orbit
    float orbitSensitivity = 2.0f;
    float panSensitivity = 0.05f;
    float zoomSensitivity = 1.5f;

    // Walk
    //float walkSpeed = 20.0f;
    float mouseSensitivity = 100.0f;

    // Projection
    float perspectiveFov = 40.0f;
    float orthoScale = 10.0f;

    void updateBlender();

    void setCameraPerspective();
    void setCameraOrthographic();
public:
    cameraController(Vector3 position = { 10.0f, 10.0f, 10.0f } , Vector3 target = { 0.0f, 0.0f, 0.0f }, Vector3 up = { 0.0f, 1.0f, 0.0f }, CameraProjection projection = CAMERA_PERSPECTIVE);
    void syncCamera() {
        camera.fovy = camera.projection == CAMERA_ORTHOGRAPHIC ? orthoScale : perspectiveFov;
    }
    void updateCamera();
    void cameraLookAt(Vector3 position, Vector3 target, Vector3 up, CameraProjection projection) {
		camera.position = position; camera.target = target; camera.up = up;     
        if (projection == CAMERA_PERSPECTIVE) setCameraPerspective();
        else setCameraOrthographic();
    }

    float& getWalkSpeed() { return panSensitivity; }
    float& getMouseSensitivity() { return orbitSensitivity; }
    float& getFovy() { return (camera.projection == CAMERA_ORTHOGRAPHIC ? orthoScale : perspectiveFov); }

    const float& getWalkSpeed() const { return panSensitivity; }
    const float& getMouseSensitivity() const { return orbitSensitivity; }
    const float& getFovy() const { return (camera.projection == CAMERA_ORTHOGRAPHIC ? orthoScale : perspectiveFov); }

    const char* getCameraProjection() {
        if (camera.projection == CAMERA_ORTHOGRAPHIC)
        {
            return "Orthographic";
        }
        else {
            return "Perspective";
        }
    }
    void setView(cameraView view, Vector3 target);

	// Switch between navigation modes (Orbit and Walk)
	// Orbit mode: camera orbits around a target point, with pan and zoom (Blender like)
	// Walk mode: camera moves freely in 3D space, with WASD movement and mouse look (Raylib)
	void setNavigationMode(cameraNavigationMode mode) { navigationMode = mode; }
	cameraNavigationMode getNavigationMode() const { return navigationMode; }

	// CameraProject is an enum defined in raylib.h, with values CAMERA_PERSPECTIVE=0 and CAMERA_ORTHOGRAPHIC=0
    void toggleProjection();

    Camera3D& getCamera() { return camera; }
    const Camera3D& getCamera() const { return camera; } // RO return
};

struct ViewportSlot
{
    cameraController camera;
    cameraView presetView = cameraView::Free;
    bool editable = false;
    bool trackSelection = true;

    float userZoom = 1.0f; // scroll-wheel multiplier, layered over auto-fit baseline

    RenderTexture2D target = { 0 };
    int targetWidth = 0;
    int targetHeight = 0;


    // Ensures `target` matches the requested size, (re)allocating only if needed.
    void ensureTarget(int width, int height)
    {
        if (width <= 0 || height <= 0) return;
        if (targetWidth == width && targetHeight == height && target.id != 0) return;

        if (target.id != 0) UnloadRenderTexture(target);
        target = LoadRenderTexture(width, height);
        targetWidth = width;
        targetHeight = height;
    }

    void releaseTarget()
    {
        if (target.id != 0)
        {
            UnloadRenderTexture(target);
            target = { 0 };
            targetWidth = 0;
            targetHeight = 0;
        }
    }
};

// Draw an on-screen list of the camera controller settings and keybindings
void drawCameraControllerSettings(cameraController&);
#endif