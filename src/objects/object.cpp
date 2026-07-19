#include "object.h"
#include <cstring>

std::vector<std::unique_ptr<shape>> objects;
std::vector<shape*> selectedObjects;
shape* activeObject;

// Class shape method definitions

static void updateSelectedObjects();

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
	return R3D_IsMeshValid(mesh);
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
	setObjectType(ObjectType::CUBE);
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
	bool valid = R3D_IsMeshValid(mesh);
	if (!valid) TraceLog(LOG_WARNING, "cube %d: mesh invalid, skipping draw", getId());
	if (valid)
	{
		R3D_DrawMeshPro(mesh, *getMaterial(), MatrixCompose(transform.translation, transform.rotation, transform.scale));
		if (getSelected()) drawSelectionWireframe(RED);
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

void initModels()
{
	//cube cube1;
	objects.push_back(std::make_unique<cube>());
	//InitLighting();

	LoadPBRTextures();

	EnsureSceneVertices();
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

void selectObjectByRay(Ray ray) {
	shape* closestObject = nullptr;
	float closestDist = FLT_MAX;

	// Find the closest object hit by the ray
	for (auto& object : objects)
	{
		RayCollision collision = GetRayCollisionBox(ray, object->getWorldBoundingBox());

		if (collision.hit && collision.distance < closestDist) {
			closestDist = collision.distance;
			closestObject = object.get();
		}
	}
	const bool additiveSelection = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

	selectObjects(closestObject, additiveSelection);

}

void selectObjects(shape* closestObject, bool additiveSelection)
{
	// Clear existing selection unless Ctrl is held
	if (!additiveSelection)
	{
		for (auto& object : objects) object->setSelected(false); 
		activeObject = nullptr;
	}

	// Clicked an object
	if (closestObject != nullptr)
	{
		if (additiveSelection)
		{
			closestObject->setSelected(!closestObject->getSelected());
			// Ctrl + click object -> toggle selection
			if (closestObject->getSelected()) activeObject = closestObject;
			else if (closestObject == activeObject) activeObject = nullptr;
		}
		else
		{
			// Normal click -> this becomes the only selection
			closestObject->setSelected(true);
			activeObject = closestObject;
		}

		TraceLog(
			LOG_INFO,
			"Clicked object ID: %u",
			closestObject->getId()
		);
	}

	updateSelectedObjects();
	totalSelectedCount = static_cast<int>(selectedObjects.size());
}

static void updateSelectedObjects()
{
	selectedObjects.clear();

	for (const auto& object : objects)
	{
		if (object && object->getSelected())
		{
			selectedObjects.push_back(object.get());
		}
	}
} 

void deleteObjects() {
	objects.erase(
		// manipulates the object array so that all objects that we want to keep are kept in the beginning and everything else is moved to the end of the array
		// std::remove_if returns an iterator to the new end of the array (the one which has useful objects), which is then used to erase the unwanted elements
		std::remove_if(
			objects.begin(),
			objects.end(),
			// [] -> lambda function to check if the object is selected
			[](const std::unique_ptr<shape>& obj) { return obj->getSelected(); }
		),
		objects.end()
	);
	selectedObjects.clear();
	activeObject = nullptr;
	totalSelectedCount = 0;
}

// ideally gonna use no models and just mesh
// Would be better if nothing was static here
void unloadModels(void) {
	UnloadModel(cubeModel);
	UnloadModel(sphereModel);
	UnloadModel(cylinderModel);
}