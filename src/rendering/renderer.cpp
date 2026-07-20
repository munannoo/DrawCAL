#include "renderer.h"
#include "raymath.h"
#include "rlgl.h"
#include "objects/object.h"
#include "features/shadings/lighting.h"
#include <r3d.h>

Shader gridShader;
Model gridPlane;

int camPosLoc;
int gridColorLoc;
int farGridColorLoc;
int scaleLoc;

// Grid Shader initialised for infinite grid rendering
void initgridShader() 
{
    TraceLog(LOG_INFO, "grid.vs exists: %s", FileExists("shaders/grid.vs") ? "YES" : "NO");
    TraceLog(LOG_INFO, "grid.fs exists: %s", FileExists("shaders/grid.fs") ? "YES" : "NO");

    gridShader = LoadShader("shaders/grid.vs","shaders/grid.fs");

    gridPlane = LoadModelFromMesh(GenMeshPlane(2000.0f, 2000.0f, 1, 1));
    gridPlane.materials[0].shader = gridShader;

    camPosLoc = GetShaderLocation(gridShader, "cameraPos");
    gridColorLoc = GetShaderLocation(gridShader, "gridColor");
    farGridColorLoc = GetShaderLocation(gridShader, "farGridColor");
    scaleLoc = GetShaderLocation(gridShader, "scale");
}

Vector4 gridCol = { 0.5f, 0.5f, 0.5f, 0.8f };
Vector4 farGridCol = { 0.2f, 0.2f, 0.2f, 0.2f };
float scale = 1.0f;

void drawGrid(const Camera3D &camera) {
    Vector3 camPos = camera.position;

    SetShaderValue(gridShader, camPosLoc, &camPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(gridShader, gridColorLoc, &gridCol, SHADER_UNIFORM_VEC4);
    SetShaderValue(gridShader, farGridColorLoc, &farGridCol, SHADER_UNIFORM_VEC4);
    SetShaderValue(gridShader, scaleLoc, &scale, SHADER_UNIFORM_FLOAT);


    rlDisableBackfaceCulling();

    BeginBlendMode(BLEND_ALPHA);
    DrawModel(gridPlane, { camera.position.x, 0.0f, camera.position.z }, 1.0f, WHITE);
    EndBlendMode();

    rlEnableBackfaceCulling();


}

void RenderCameraSceneToTexture(const Camera3D& camera, Rectangle viewport, RenderTexture2D& target)
{
    Rectangle localViewport = { 0, 0, viewport.width, viewport.height }; // local, not screen-space

    R3D_View view{};
    view.camera = R3D_CameraFromRL(camera);
    view.target = target;
    view.viewport = localViewport;
    R3D_BeginPro(view);
    renderAllObjects();
    R3D_End();

    BeginTextureMode(target);
    rlViewport(0, 0, (int)viewport.width, (int)viewport.height);

    BeginMode3D(camera);
    {
        rlDrawRenderBatchActive();
        rlMatrixMode(RL_PROJECTION);
        rlLoadIdentity();
        float aspect = viewport.width / viewport.height;
        Matrix proj = (camera.projection == CAMERA_PERSPECTIVE)
            ? MatrixPerspective(camera.fovy * DEG2RAD, aspect, 0.01, 1000.0)
            : MatrixOrtho(-camera.fovy * aspect / 2, camera.fovy * aspect / 2,
                -camera.fovy / 2, camera.fovy / 2, 0.01, 1000.0);
        rlSetMatrixProjection(proj);

        rlMatrixMode(RL_MODELVIEW);
        rlLoadIdentity();
        Matrix view3d = MatrixLookAt(camera.position, camera.target, camera.up);
        rlMultMatrixf(MatrixToFloat(view3d));

        renderLightObjects();
        drawGrid(camera);
        drawObjectTransformGizmo(camera, viewport);
    }
    EndMode3D();
    EndTextureMode();
}

void DrawCameraScene(const Camera3D& camera, Rectangle viewport, RenderTexture2D& target)
{
    RenderCameraSceneToTexture(camera, viewport, target);

    // Screen-space offset only matters here, placing the finished texture on screen.
    DrawTextureRec(target.texture, { 0, 0, viewport.width, -viewport.height },
        { viewport.x, viewport.y }, WHITE);
}

// render all items
// render cube
static void renderCube() {
	for (auto& obj : objects)
	{
		obj->drawShape();
		TraceLog(LOG_INFO, "%d", static_cast<int>(obj->getMaterialType()));
	}
	for (const auto& object : objects)
	{
		if (object->getSelected())
		{
			object->drawSelectionWireframe(YELLOW);  // or whatever color
		}
	}
	//for (int i = 0; i < c; i++) {
		//cube[i].drawShape();
		//Color renderColor = Cu[i].color;
		//renderColor.a = Cu[i].isSelected ? 128 : 255; // changes transparency when objec tis selected
		//DrawObjectModel(cubeModel, Cu[i], renderColor);
	//}
}

static void renderSelectedObjects() {
	for (const auto& object : objects)
	{
		if (object->getSelected())
		{
			object->drawSelectionWireframe(YELLOW);  // or whatever color
		}
	}
}

void renderLightObjects() {
	for (const auto& object : lights)
	{
		R3D_DrawLightShape(object->getLight());
	}
}

void renderAllObjects() {
	renderCube();
	renderSelectedObjects();

	//renderSphere();
	//renderCylinder();
}
//render sphere
//void renderSphere() {
//    for (int i = 0; i < s; i++) {
//
//        if (Sp[i].isLight && Sp[i].lightIndex >= 0) {
//            SetSceneLightPosition(Sp[i].lightIndex, Sp[i].position);
//            SetSceneLightProperties(
//                Sp[i].lightIndex,
//                Sp[i].color,
//                Sp[i].lightIntensity,
//                Sp[i].lightRadius
//            );
//
//            // Draw light marker as unlit bright sphere
//            DrawSphere(Sp[i].position, Sp[i].scale.x, Sp[i].color);
//
//            if (Sp[i].isSelected) {
//                DrawSphereWires(Sp[i].position, Sp[i].scale.x * 1.15f, 16, 16, WHITE);
//            }
//
//            continue;
//        }
//
//        Color renderColor = Sp[i].color;
//        renderColor.a = Sp[i].isSelected ? 128 : 255;
//
//        DrawObjectModel(sphereModel, Sp[i], renderColor);
//    }
//}
////render cylinder
//void renderCylinder() {
//    for (int i = 0; i < y; i++) {
//        Color renderColor = cy[i].color;
//        renderColor.a = cy[i].isSelected ? 128 : 255; // changes transparency when objec tis selected
//        DrawObjectModel(cylinderModel, cy[i], renderColor);
//    }
//}
