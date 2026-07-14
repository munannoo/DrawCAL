#include "object.h"
#include <cstring>

// Static default material instance used by shapes that haven't been assigned a material
static R3D_Material s_defaultMaterial = { 0 };

std::vector<std::unique_ptr<shape>> objects;
std::vector<shape*> selectedObjects;
extern shape* activeObject;
// Class shape method definitions

shape::shape()
	: transform{ { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } },
	meshData{},
	mesh{},
	id(nextId++)
{
	material = &defaultMaterial;
}

shape::~shape()
{
	if (R3D_IsMeshDataValid(meshData)){R3D_UnloadMeshData(meshData);}
	if (R3D_IsMeshValid(mesh)){R3D_UnloadMesh(mesh);}
}

bool shape::loadMesh()
{
	if (!R3D_IsMeshDataValid(meshData)) return false;
	if (primitiveType != R3D_PRIMITIVE_TRIANGLES) primitiveType = R3D_PRIMITIVE_TRIANGLES;

	mesh = R3D_LoadMesh(primitiveType, meshData, nullptr);

	// Use the actual R3D mesh validity check if one exists.
	meshDirty = false;
	return true;
}

// Update this later
void shape::drawSelectionWireframe(Color color) const
{
	if (!getSelected()) return;

	Matrix transformMatrix = getMatrix();
	R3D_Mesh torus = R3D_GenMeshTorus(0.5f, 0.1f, 32, 16);
	R3D_Material material = R3D_GetDefaultMaterial();

	// A torus for now for checking if object has been selected, will change to a wireframe box later
	R3D_DrawMesh(torus, material, getTransform().translation, 5.0f);

}

void shape::applyMaterial(MaterialType type)
{

	R3D_Material* newMaterial = GetMaterial(type);
	TraceLog(LOG_INFO, "Old material: %p", (void*)material);
	TraceLog(LOG_INFO, "New material: %p", (void*)newMaterial);

	materialType = type;

	if (newMaterial == nullptr) return;

	materialType = type;
	material = newMaterial;
}

// Get World Bounding Box by transforming the local AABB corners to world space and computing the new AABB
BoundingBox shape::getWorldBoundingBox() const
{
	BoundingBox local = mesh.aabb; // local bounding box already determined by R3D_CalculateMeshDataBoundingBox(MeshData meshData);
	Matrix matrix = getMatrix();

	Vector3 corners[8] = {
		{ local.min.x, local.min.y, local.min.z },
		{ local.max.x, local.min.y, local.min.z },
		{ local.min.x, local.max.y, local.min.z },
		{ local.max.x, local.max.y, local.min.z },
		{ local.min.x, local.min.y, local.max.z },
		{ local.max.x, local.min.y, local.max.z },
		{ local.min.x, local.max.y, local.max.z },
		{ local.max.x, local.max.y, local.max.z }
	};

	Vector3 first = Vector3Transform(corners[0], matrix);

	BoundingBox world = {first,first};

	for (int i = 1; i < 8; i++)
	{
		Vector3 point = Vector3Transform(corners[i], matrix);

		world.min.x = fminf(world.min.x, point.x);
		world.min.y = fminf(world.min.y, point.y);
		world.min.z = fminf(world.min.z, point.z);

		world.max.x = fmaxf(world.max.x, point.x);
		world.max.y = fmaxf(world.max.y, point.y);
		world.max.z = fmaxf(world.max.z, point.z);
	}

	return world;
}

Vector3 shape::getVertexLocalPosition(int index) const
{
	if (index < 0 || index >= meshData.vertexCount) return { 0.0f,0.0f,0.0f };

	return meshData.vertices[index].position;
}

void shape::setVertexLocalPosition(int index, Vector3 position)
{
	if (index < 0 || index >= meshData.vertexCount) return;

	meshData.vertices[index].position = position;

	meshDirty = true;
}

Vector3 shape::getVertexWorldPosition(int index) const
{
	Vector3 localPosition = getVertexLocalPosition(index);

	return Vector3Transform(localPosition, getMatrix());
}


void shape::setVertexWorldPosition(int index, Vector3 worldPosition)
{
	if (index < 0 || index >= meshData.vertexCount) return;
	

	Matrix inverseTransform = MatrixInvert(getMatrix());
	Vector3 localPosition =	Vector3Transform(worldPosition, inverseTransform);

	setVertexLocalPosition(index,localPosition);
}


void shape::applyTransform()
{
	R3D_TransformMeshData(&meshData,getMatrix());

	// Reset transform to identity
	transform.translation ={ 0.0f, 0.0f, 0.0f };

	transform.rotation =QuaternionIdentity();

	transform.scale ={ 1.0f, 1.0f, 1.0f };

	meshDirty = true;
}

void shape::regenerateNormals()
{
	R3D_GenMeshDataNormals(&meshData,primitiveType);

	meshDirty = true;
}

void shape::regenerateTangents()
{
	R3D_GenMeshDataTangents(&meshData,primitiveType);

	meshDirty = true;
}

BoundingBox shape::getBoundingBox() const
{
	return R3D_CalculateMeshDataBoundingBox(meshData);
}

bool shape::getSelected() const
{
	return isSelected;
}

void shape::setSelected(bool selected)
{
	isSelected = selected;
}

bool shape::isMeshDirty() const
{
	return meshDirty;
}

bool shape::syncMesh()
{
	if (!meshDirty)
		return true;  // Already synchronized

	if (!R3D_IsMeshDataValid(meshData))
		return false; // CPU mesh is invalid

	if (!R3D_IsMeshValid(mesh))
		return false; // GPU mesh has not been loaded

	if (!R3D_UpdateMesh(&mesh, meshData, nullptr))
		return false; // Upload failed

	meshDirty = false;
	return true;
}

// Cube Model, Sphere Model, and Cylinder Model
bool cube::generateMeshData()
{
	meshData = R3D_GenMeshDataCubeEx(width,height,length, resX, resY, resZ);
	return R3D_IsMeshDataValid(meshData);
}

int cube::cubeCount = 0;

cube::cube(
	Vector3 position,Quaternion rotation,Vector3 scale,
	float width,float height,float length,
	int resX, int resY, int resZ
)
	: shape(),
	width(width),height(height),length(length), 
	resX(resX), resY(resY), resZ(resZ)
{
	// Set object transform
	transform.translation = position;
	transform.rotation = rotation;
	transform.scale = scale;

	// Generate editable CPU-side cube geometry
	generateMeshData();

	// Cube is made of triangles
	primitiveType = R3D_PRIMITIVE_TRIANGLES;

	loadMesh();  // Load the mesh to GPU

	cubeCount++;
}




void cube::drawShape() {
	if (R3D_IsMeshValid(mesh))
	{
		R3D_DrawMeshPro(mesh, *getMaterial(), MatrixCompose(transform.translation, transform.rotation, transform.scale));
		if (getSelected())
		{
			drawSelectionWireframe(RED);
		}
	}
}


static Model cubeModel;
static Model sphereModel;
static Model cylinderModel;

static BoundingBox Cubebounds;
static BoundingBox Spherebounds;
static BoundingBox Cylinderbounds;

static ObjectInstance Cu[100];
static ObjectInstance Sp[100];
static ObjectInstance cy[100];


static void DrawObjectModel(Model& model, ObjectInstance obj, Color color);
static Matrix GetObjectTransform(ObjectInstance obj);
static void EnsureSceneVertices();
static void AssignDefaultVertices(ObjectInstance& obj, ObjectType type);
static void DrawSelectedVertexHandles(Camera3D camera);

static ObjectType selectedType = ObjectType::NONE;
static const float DEFAULT_LIGHT_INTENSITY = 350.0f;
static const float DEFAULT_LIGHT_RADIUS = 35.0f;
static const float DEFAULT_LIGHT_HEIGHT = 3.0f;
static int c = 0;
static int s = 0;
static int y = 0;
static int totalSelectedCount = 0;

// for vertex handle rendering
enum HandleRole { HANDLE_CORNER, HANDLE_CENTER, HANDLE_RADIUS, HANDLE_AXIS };

struct VertexHandle {
	Vector3 worldPos;
	HandleRole role;
};

void load() {
	loadScene();
}
void save() {
	saveScene();
}

// Manages triangles for lighitng issues
static Mesh GenTexturedCylinder(float radius, float height, int slices)
{
	Mesh mesh = { 0 };

	if (slices < 3) slices = 3;

	float halfHeight = height * 0.5f;

	int sideVertexCount = (slices + 1) * 2;
	int topVertexCount = slices + 2;
	int bottomVertexCount = slices + 2;
	int totalVertexCount = sideVertexCount + topVertexCount + bottomVertexCount;

	int sideTriangleCount = slices * 2;
	int topTriangleCount = slices;
	int bottomTriangleCount = slices;

	int totalTriangleCount =
		sideTriangleCount + topTriangleCount + bottomTriangleCount;

	mesh.vertexCount = totalVertexCount;
	mesh.triangleCount = totalTriangleCount;

	mesh.vertices = (float*)MemAlloc(totalVertexCount * 3 * sizeof(float));
	mesh.normals = (float*)MemAlloc(totalVertexCount * 3 * sizeof(float));
	mesh.texcoords = (float*)MemAlloc(totalVertexCount * 2 * sizeof(float));
	mesh.indices = (unsigned short*)MemAlloc(totalTriangleCount * 3 * sizeof(unsigned short));

	int v = 0;

	auto SetVertex = [&](int index, Vector3 position, Vector3 normal, Vector2 uv)
		{
			mesh.vertices[index * 3 + 0] = position.x;
			mesh.vertices[index * 3 + 1] = position.y;
			mesh.vertices[index * 3 + 2] = position.z;

			mesh.normals[index * 3 + 0] = normal.x;
			mesh.normals[index * 3 + 1] = normal.y;
			mesh.normals[index * 3 + 2] = normal.z;

			mesh.texcoords[index * 2 + 0] = uv.x;
			mesh.texcoords[index * 2 + 1] = uv.y;
		};

	for (int i = 0; i <= slices; i++)
	{
		float t = (float)i / (float)slices;
		float angle = t * 2.0f * PI;

		float x = cosf(angle) * radius;
		float z = sinf(angle) * radius;

		Vector3 normal = Vector3Normalize({ x, 0.0f, z });

		SetVertex(
			v++,
			{ x, -halfHeight, z },
			normal,
			{ t, 1.0f }
		);

		SetVertex(
			v++,
			{ x, halfHeight, z },
			normal,
			{ t, 0.0f }
		);
	}

	int topCenterIndex = v;

	SetVertex(
		v++,
		{ 0.0f, halfHeight, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.5f, 0.5f }
	);

	int topRingStart = v;

	for (int i = 0; i <= slices; i++)
	{
		float t = (float)i / (float)slices;
		float angle = t * 2.0f * PI;

		float x = cosf(angle) * radius;
		float z = sinf(angle) * radius;

		float u = 0.5f + cosf(angle) * 0.5f;
		float vv = 0.5f + sinf(angle) * 0.5f;

		SetVertex(
			v++,
			{ x, halfHeight, z },
			{ 0.0f, 1.0f, 0.0f },
			{ u, vv }
		);
	}

	int bottomCenterIndex = v;

	SetVertex(
		v++,
		{ 0.0f, -halfHeight, 0.0f },
		{ 0.0f, -1.0f, 0.0f },
		{ 0.5f, 0.5f }
	);

	int bottomRingStart = v;

	for (int i = 0; i <= slices; i++)
	{
		float t = (float)i / (float)slices;
		float angle = t * 2.0f * PI;

		float x = cosf(angle) * radius;
		float z = sinf(angle) * radius;

		float u = 0.5f + cosf(angle) * 0.5f;
		float vv = 0.5f - sinf(angle) * 0.5f;

		SetVertex(
			v++,
			{ x, -halfHeight, z },
			{ 0.0f, -1.0f, 0.0f },
			{ u, vv }
		);
	}

	int idx = 0;

	auto AddTriangle = [&](unsigned short a, unsigned short b, unsigned short c)
		{
			mesh.indices[idx++] = a;
			mesh.indices[idx++] = b;
			mesh.indices[idx++] = c;
		};

	for (int i = 0; i < slices; i++)
	{
		unsigned short bottom0 = (unsigned short)(i * 2);
		unsigned short top0 = (unsigned short)(i * 2 + 1);
		unsigned short bottom1 = (unsigned short)((i + 1) * 2);
		unsigned short top1 = (unsigned short)((i + 1) * 2 + 1);

		AddTriangle(bottom0, top0, top1);
		AddTriangle(bottom0, top1, bottom1);
	}

	for (int i = 0; i < slices; i++)
	{
		unsigned short ring0 = (unsigned short)(topRingStart + i);
		unsigned short ring1 = (unsigned short)(topRingStart + i + 1);

		AddTriangle(
			(unsigned short)topCenterIndex,
			ring1,
			ring0
		);
	}

	for (int i = 0; i < slices; i++)
	{
		unsigned short ring0 = (unsigned short)(bottomRingStart + i);
		unsigned short ring1 = (unsigned short)(bottomRingStart + i + 1);

		AddTriangle(
			(unsigned short)bottomCenterIndex,
			ring0,
			ring1
		);
	}

	return mesh;
}

void initModels()
{
	//cube cube1;
	objects.push_back(std::make_unique<cube>());
	//InitLighting();

	Mesh cubeMesh = GenMeshCube(2.0f, 2.0f, 2.0f);
	GenMeshTangents(&cubeMesh);
	cubeModel = LoadModelFromMesh(cubeMesh);
	Cubebounds = GetModelBoundingBox(cubeModel);

	Mesh sphereMesh = GenMeshSphere(1.0f, 32, 32);
	GenMeshTangents(&sphereMesh);
	sphereModel = LoadModelFromMesh(sphereMesh);
	Spherebounds = GetModelBoundingBox(sphereModel);

	Mesh cylinderMesh = GenTexturedCylinder(1.0f, 2.0f, 64);
	GenMeshTangents(&cylinderMesh);
	UploadMesh(&cylinderMesh, false);

	cylinderModel = LoadModelFromMesh(cylinderMesh);
	Cylinderbounds = GetModelBoundingBox(cylinderModel);
	LoadPBRTextures();

	//ApplyLightingShader(cubeModel);
	//ApplyLightingShader(sphereModel);
	//ApplyLightingShader(cylinderModel);

	EnsureSceneVertices();
}
void UploadSceneToRayTracer()
{
	// Legacy compatibility no-op. Ray tracing was removed from the active
	// rendering path and replaced with rPBR-style PBR lighting.
}
ObjectInstance* getFirstSelectedMutable(int* outType, int* outIndex)
{
	for (int i = 0; i < c; i++)
	{
		if (Cu[i].isSelected)
		{
			if (outType) *outType = 1;
			if (outIndex) *outIndex = i;

			return &Cu[i];
		}
	}

	for (int i = 0; i < s; i++)
	{
		if (Sp[i].isSelected)
		{
			if (outType) *outType = 2;
			if (outIndex) *outIndex = i;

			return &Sp[i];
		}
	}

	for (int i = 0; i < y; i++)
	{
		if (cy[i].isSelected)
		{
			if (outType) *outType = 3;
			if (outIndex) *outIndex = i;

			return &cy[i];
		}
	}

	if (outType) *outType = 0;
	if (outIndex) *outIndex = -1;

	return nullptr;
}

bool getFirstSelected(
	ObjectInstance* out,
	int* outType,
	int* outIndex
)
{
	ObjectInstance* selected =
		getFirstSelectedMutable(outType, outIndex);

	if (selected == nullptr)
	{
		return false;
	}

	if (out)
	{
		*out = *selected;
	}

	return true;
}

void deselectAllObjects()
{
	for (int i = 0; i < c; i++)
	{
		Cu[i].isSelected = false;
	}

	for (int i = 0; i < s; i++)
	{
		Sp[i].isSelected = false;
	}

	for (int i = 0; i < y; i++)
	{
		cy[i].isSelected = false;
	}

	totalSelectedCount = 0;
	selectedType = ObjectType::NONE;
}

int getTotalSelectedCount() {
	int total = 0;
	for (int i = 0; i < c; i++) if (Cu[i].isSelected) total++;
	for (int i = 0; i < s; i++) if (Sp[i].isSelected) total++;
	for (int i = 0; i < y; i++) if (cy[i].isSelected) total++;
	return total;
}

void DrawSceneForShadowMap()
{
	// Legacy compatibility no-op. Shadow maps are not used by the rPBR path.
}

// DefaultVertices -> gives local position of the shapes
static std::vector<Vector3> GetCubeDefaultVertices() {
	return {
		{ -1.0f, -1.0f, -1.0f },
		{  1.0f, -1.0f, -1.0f },
		{  1.0f, -1.0f,  1.0f },
		{ -1.0f, -1.0f,  1.0f },
		{ -1.0f,  1.0f, -1.0f },
		{  1.0f,  1.0f, -1.0f },
		{  1.0f,  1.0f,  1.0f },
		{ -1.0f,  1.0f,  1.0f },
	};
}

static std::vector<Vector3> GetSphereDefaultVertices() {
	return {
		{  0.0f,  0.0f,  0.0f },
		{  0.0f,  1.0f,  0.0f },
		{  0.0f, -1.0f,  0.0f },
		{  1.0f,  0.0f,  0.0f },
		{ -1.0f,  0.0f,  0.0f },
		{  0.0f,  0.0f,  1.0f },
		{  0.0f,  0.0f, -1.0f },
	};
}

static std::vector<Vector3> GetCylinderDefaultVertices() {
	std::vector<Vector3> vertices;
	const int slices = 16;

	vertices.push_back({ 0.0f,  1.0f, 0.0f });
	vertices.push_back({ 0.0f, -1.0f, 0.0f });

	for (int i = 0; i < slices; i++) {
		float angle = (2.0f * PI * i) / slices;
		float x = cosf(angle);
		float z = sinf(angle);

		vertices.push_back({ x,  1.0f, z });
		vertices.push_back({ x, -1.0f, z });
	}

	return vertices;
}

static void AssignDefaultVertices(ObjectInstance& obj, ObjectType type) {
	switch (type) {
	case ObjectType::CUBE:
		obj.vertices = GetCubeDefaultVertices();
		break;
	case ObjectType::SPHERE:
		obj.vertices = GetSphereDefaultVertices();
		break;
	case ObjectType::CYLINDER:
		obj.vertices = GetCylinderDefaultVertices();
		break;
	default:
		obj.vertices.clear();
		break;
	}
}

static void EnsureVertices(ObjectInstance& obj, ObjectType type) {
	if (obj.vertices.empty()) {
		AssignDefaultVertices(obj, type);
	}
}

static void EnsureSceneVertices() {
	for (int i = 0; i < c; i++) EnsureVertices(Cu[i], ObjectType::CUBE);
	for (int i = 0; i < s; i++) EnsureVertices(Sp[i], ObjectType::SPHERE);
	for (int i = 0; i < y; i++) EnsureVertices(cy[i], ObjectType::CYLINDER);
}

static std::vector<VertexHandle> GetHandles(const ObjectInstance& obj, ObjectType type) {
	std::vector<VertexHandle> handles;
	Matrix transform = GetObjectTransform(obj);

	for (size_t i = 0; i < obj.vertices.size(); i++) {
		HandleRole role = HANDLE_CORNER;

		if (type == ObjectType::SPHERE) {
			role = (i == 0) ? HANDLE_CENTER : HANDLE_RADIUS;
		}
		else if (type == ObjectType::CYLINDER) {
			role = (i < 2) ? HANDLE_AXIS : HANDLE_RADIUS;
		}

		handles.push_back({ Vector3Transform(obj.vertices[i], transform), role });
	}

	return handles;
}

static std::vector<Vector3> GetVertices(const ObjectInstance& obj, ObjectType type) {
	std::vector<Vector3> positions;

	for (const auto& handle : GetHandles(obj, type)) {
		positions.push_back(handle.worldPos);
	}

	return positions;
}

// checks if the vertices fall within the camera frame
static bool IsInFrontOfCamera(Vector3 position, Camera3D camera) {
	Vector3 cameraForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
	Vector3 toPosition = Vector3Subtract(position, camera.position);

	return Vector3DotProduct(cameraForward, toPosition) > 0.0f;
}

static void DrawHandle(Vector3 worldPos, HandleRole role, bool hovered) {
	Color color = WHITE;
	float radius = 0.055f;

	switch (role) {
	case HANDLE_CENTER:
		color = hovered ? YELLOW : GRAY;
		radius = 0.07f;
		break;
	case HANDLE_CORNER:
		color = hovered ? YELLOW : WHITE;
		break;
	case HANDLE_RADIUS:
		color = hovered ? ORANGE : WHITE;
		break;
	case HANDLE_AXIS:
		color = hovered ? SKYBLUE : BLUE;
		radius = 0.065f;
		break;
	}

	DrawSphere(worldPos, radius, color);
	DrawSphereWires(worldPos, radius * 1.35f, 8, 8, DARKGRAY);
}

static void DrawVertexHandlesForObject(const ObjectInstance& obj, ObjectType type, Camera3D camera) {
	Vector2 mouse = GetMousePosition();
	std::vector<VertexHandle> handles = GetHandles(obj, type);

	rlDisableDepthTest(); // draw over the objects.

	for (const auto& handle : handles) {
		if (!IsInFrontOfCamera(handle.worldPos, camera)) continue;

		Vector2 screenPos = GetWorldToScreen(handle.worldPos, camera);
		bool hovered = CheckCollisionPointCircle(mouse, screenPos, 8.0f);

		DrawHandle(handle.worldPos, handle.role, hovered);
	}

	rlEnableDepthTest();
}

static void DrawSelectedVertexHandles(Camera3D camera) {
	EnsureSceneVertices();

	for (int i = 0; i < c; i++) {
		if (Cu[i].isSelected) DrawVertexHandlesForObject(Cu[i], ObjectType::CUBE, camera);
	}

	for (int i = 0; i < s; i++) {
		if (Sp[i].isSelected) DrawVertexHandlesForObject(Sp[i], ObjectType::SPHERE, camera);
	}

	for (int i = 0; i < y; i++) {
		if (cy[i].isSelected) DrawVertexHandlesForObject(cy[i], ObjectType::CYLINDER, camera);
	}
}

//void cube(const Vector3 pos,Color color) {
//    if (c < 100) {
//        Cu[c].position = pos;
//        Cu[c].rotation = { 0.0f, 0.0f, 0.0f };
//        Cu[c].scale = { 1.0f, 1.0f, 1.0f };
//        Cu[c].color = color;
//        Cu[c].isSelected = false;
//        Cu[c].isLight = false;
//        Cu[c].lightIndex = -1;
//        Cu[c].lightIntensity = 0.0f;
//        Cu[c].lightRadius = 0.0f;
//        Cu[c].material = MATERIAL_WOOD;
//        AssignDefaultVertices(Cu[c], CUBE);
//        c++;
//    }
//}
//
//void sphere(const Vector3 pos,Color color){
//    if(s<100){
//        Sp[s].position = pos;
//        Sp[s].rotation = Vector3{0.0f, 0.0f, 0.0f};
//        Sp[s].scale = Vector3{1.0f, 1.0f, 1.0f};
//        Sp[s].color = color;
//        Sp[s].isSelected = false;
//        Sp[s].isLight = false;
//        Sp[s].lightIndex = -1;
//        Sp[s].lightIntensity = 0.0f;
//        Sp[s].lightRadius = 0.0f;
//        Sp[s].material = MATERIAL_WOOD;
//        AssignDefaultVertices(Sp[s], SPHERE);
//        s++;
//    }
//}
//
//void cylinder(const Vector3 pos,Color color){
//    if(y<100){
//        cy[y].position = pos;
//        cy[y].rotation = Vector3{0.0f, 0.0f, 0.0f};
//        cy[y].scale = Vector3{1.0f, 1.0f, 1.0f};
//        cy[y].color = color;
//        cy[y].isSelected = false;
//        cy[y].isLight = false;
//        cy[y].lightIndex = -1;
//        cy[y].lightIntensity = 0.0f;
//        cy[y].lightRadius = 0.0f;
//        cy[y].material = MATERIAL_WOOD;
//        AssignDefaultVertices(cy[y], CYLINDER);
//        y++;
//    }
//}

//void SyncObjectLightsToScene()
//{
//	for (int i = 0; i < s; i++)
//	{
//		if (!Sp[i].isLight || Sp[i].lightIndex < 0) continue;
//
//		SetSceneLightPosition(Sp[i].lightIndex, Sp[i].position);
//		SetSceneLightProperties(
//			Sp[i].lightIndex,
//			Sp[i].color,
//			Sp[i].lightIntensity,
//			Sp[i].lightRadius
//		);
//	}
//}

// render all items
// render cube
void renderCube() {
	for (auto& obj : objects)
	{
		obj->drawShape();
		//TraceLog(LOG_INFO, "%d", static_cast<int>(obj->getMaterialType()));
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

void renderSelectedObjects() {
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

void leftclick(Ray ray)
{
	shape* closestObject = nullptr;
	float closestDist = FLT_MAX;

	// Find the closest object hit by the ray
	for (auto& object : objects)
	{
		RayCollision collision = GetRayCollisionBox(
			ray,
			object->getWorldBoundingBox()
		);

		if (collision.hit && collision.distance < closestDist)
		{
			closestDist = collision.distance;
			closestObject = object.get();
		}
	}

	const bool additiveSelection =
		IsKeyDown(KEY_LEFT_CONTROL) ||
		IsKeyDown(KEY_RIGHT_CONTROL);

	// Clear existing selection unless Ctrl is held
	if (!additiveSelection)
	{
		for (auto& object : objects)
		{
			object->setSelected(false);
		}
	}
	// Clicked an object
	if (closestObject != nullptr)
	{
		if (additiveSelection)
		{
			// Ctrl + click selected object -> deselect it
			if (closestObject->getSelected())
			{
				closestObject->setSelected(false);

				selectedObjects.erase(
					std::remove(
						selectedObjects.begin(),
						selectedObjects.end(),
						closestObject
					),
					selectedObjects.end()
				);
			}
			// Ctrl + click unselected object -> add it
			else
			{
				closestObject->setSelected(true);
				selectedObjects.push_back(closestObject);
			}
		}
		else
		{
			// Normal click -> this becomes the only selection
			closestObject->setSelected(true);
			selectedObjects.push_back(closestObject);
		}

		TraceLog(
			LOG_INFO,
			"Clicked object ID: %u",
			closestObject->getId()
		);
	}


	totalSelectedCount = static_cast<int>(selectedObjects.size());
}


static Matrix GetObjectTransform(ObjectInstance obj)
{
	Matrix matScale = MatrixScale(
		obj.scale.x,
		obj.scale.y,
		obj.scale.z
	);

	Matrix matRotation = MatrixRotateXYZ({
		DEG2RAD * obj.rotation.x,
		DEG2RAD * obj.rotation.y,
		DEG2RAD * obj.rotation.z
		});

	Matrix matTranslation = MatrixTranslate(
		obj.position.x,
		obj.position.y,
		obj.position.z
	);

	Matrix transform = MatrixMultiply(
		MatrixMultiply(matScale, matRotation),
		matTranslation
	);

	return transform;
}

static void DrawObjectModel(Model& model, ObjectInstance obj, Color color)
{
	ApplyPBRMaterial(model, obj.material);

	Model tempModel = model;
	tempModel.transform = GetObjectTransform(obj);

	DrawModel(tempModel, { 0.0f, 0.0f, 0.0f }, 1.0f, color);
}

bool updateObjectTransformGizmo(Camera3D camera) {
	return UpdateTransformGizmo(
		camera,
		Cu, c,
		Sp, s,
		cy, y
	);
}

void drawObjectTransformGizmo(Camera3D camera) {
	DrawTransformGizmo(
		Cu, c,
		Sp, s,
		cy, y
	);
	DrawSelectedVertexHandles(camera);
}
//
//void lightSphere(const Vector3 pos, Color color)
//{
//	if (s < 100)
//	{
//		Vector3 lightPos = pos;
//		lightPos.y += DEFAULT_LIGHT_HEIGHT;
//
//		int lightIndex = CreatePointLight(lightPos);
//
//		if (lightIndex == -1) return;
//
//		Sp[s].position = lightPos;
//		Sp[s].rotation = Vector3{ 0.0f, 0.0f, 0.0f };
//		Sp[s].scale = Vector3{ 0.25f, 0.25f, 0.25f };
//		Sp[s].color = color;
//		Sp[s].isSelected = false;
//
//		Sp[s].isLight = true;
//		Sp[s].lightIndex = lightIndex;
//		Sp[s].lightIntensity = DEFAULT_LIGHT_INTENSITY;
//		Sp[s].lightRadius = DEFAULT_LIGHT_RADIUS;
//
//		SetSceneLightPosition(Sp[s].lightIndex, Sp[s].position);
//		SetSceneLightProperties(
//			Sp[s].lightIndex,
//			Sp[s].color,
//			Sp[s].lightIntensity,
//			Sp[s].lightRadius
//		);
//
//		s++;
//	}
//}

void deleteobj() {
	for (int i = 0; i < c; i++) {
		if (Cu[i].isSelected) {
			for (int j = i; j < c - 1; j++) {
				Cu[j] = Cu[j + 1];
			}
			c--;
			i--;
		}
	}
	for (int i = 0; i < s; i++) {
		if (Sp[i].isSelected) {

			if (Sp[i].isLight && Sp[i].lightIndex >= 0) {
				int deletedLightIndex = Sp[i].lightIndex;
				//DeleteSceneLight(deletedLightIndex);

				// Fix light indices of remaining light spheres
				for (int k = 0; k < s; k++) {
					if (Sp[k].isLight && Sp[k].lightIndex > deletedLightIndex) {
						Sp[k].lightIndex--;
					}
				}
			}

			for (int j = i; j < s - 1; j++) {
				Sp[j] = Sp[j + 1];
			}

			s--;
			i--;
		}
	}
	for (int i = 0; i < y; i++) {
		if (cy[i].isSelected) {
			for (int j = i; j < y - 1; j++) {
				cy[j] = cy[j + 1];
			}
			y--;
			i--;
		}
	}

	totalSelectedCount = 0;
	selectedType = ObjectType::NONE;
}

// ideally gonna use no models and just mesh
// Would be better if nothing was static here
void unloadModels(void) {
	UnloadModel(cubeModel);
	UnloadModel(sphereModel);
	UnloadModel(cylinderModel);

}

int getObjectCount(ObjectType objectType)
{
	switch (objectType)
	{
	case ObjectType::CUBE: return c;
	case ObjectType::SPHERE: return s;
	case ObjectType::CYLINDER: return y;
	default: return 0;
	}
}

ObjectInstance* getObjectMutable(ObjectType objectType, int objectIndex)
{
	if (objectIndex < 0 || objectIndex >= getObjectCount(objectType)) return nullptr;

	switch (objectType)
	{
	case ObjectType::CUBE: return &Cu[objectIndex];
	case ObjectType::SPHERE: return &Sp[objectIndex];
	case ObjectType::CYLINDER: return &cy[objectIndex];
	default: return nullptr;
	}
}

bool selectObject(ObjectType objectType, int objectIndex, bool additive)
{
	ObjectInstance* object = getObjectMutable(objectType, objectIndex);
	if (object == nullptr) return false;

	if (!additive) deselectAllObjects();
	object->isSelected = additive ? !object->isSelected : true;
	totalSelectedCount = getTotalSelectedCount();
	selectedType = object->isSelected ? static_cast<ObjectType>(objectType) : ObjectType::NONE;
	return true;
}
