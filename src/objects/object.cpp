#include "object.h"
#include <cstring>

std::vector<std::unique_ptr<shape>> objects;
std::vector<shape*> selectedObjects;
shape* activeObject;

// Class shape method definitions

static void updateSelectedObjects();

void selectAnyByRay(Ray ray)
{
	shape* closestObject = nullptr;
	float closestObjectDist = FLT_MAX;

	for (auto& object : objects)
	{
		RayCollision collision = GetRayCollisionBox(ray, object->getWorldBoundingBox());
		if (collision.hit && collision.distance < closestObjectDist) {
			closestObjectDist = collision.distance;
			closestObject = object.get();
		}
	}

	Light* closestLight = nullptr;
	float closestLightDist = FLT_MAX;

	for (auto& lightPtr : lights)
	{
		RayCollision collision = GetRayCollisionSphere(ray, lightPtr->getPosition(), lightPtr->getPickRadius());
		if (collision.hit && collision.distance < closestLightDist) {
			closestLightDist = collision.distance;
			closestLight = lightPtr.get();
		}
	}

	const bool additiveSelection = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

	// Whichever type is actually closer to the camera wins the click.
	if (closestLightDist < closestObjectDist)
	{
		if (!additiveSelection) selectObjects(nullptr, false); // clear shape selection
		selectLights(closestLight, additiveSelection);
	}
	else
	{
		if (!additiveSelection) selectLights(nullptr, false); // clear light selection
		selectObjects(closestObject, additiveSelection);
	}
}

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
	if (R3D_IsMeshDataValid(meshData)) { R3D_UnloadMeshData(meshData); }
	if (R3D_IsMeshValid(mesh)) { R3D_UnloadMesh(mesh); }
}

bool shape::loadMesh()
{
	if (!R3D_IsMeshDataValid(meshData)) return false;
	if (primitiveType != R3D_PRIMITIVE_TRIANGLES) primitiveType = R3D_PRIMITIVE_TRIANGLES;

	mesh = R3D_LoadMesh(primitiveType, meshData, nullptr);

	meshDirty = false;
	return R3D_IsMeshValid(mesh);
}
void clearScene()
{
	objects.clear();       // unique_ptr destructors handle mesh/meshData unload via ~shape()
	lights.clear();        // unique_ptr destructors call R3D_DestroyLight via ~Light()
	selectedObjects.clear();
	activeObject = nullptr;
}
void shape::drawSelectionWireframe(Color color) const
{
	if (!getSelected()) return;
	if (!R3D_IsMeshDataValid(meshData)) return;
	if (meshData.vertexCount < 3) return;

	rlPushMatrix();
	rlMultMatrixf(MatrixToFloat(getMatrix()));

	rlBegin(RL_LINES);
	rlColor4ub(color.r, color.g, color.b, color.a);

	auto drawTriEdges = [](Vector3 a, Vector3 b, Vector3 c)
		{
			rlVertex3f(a.x, a.y, a.z); rlVertex3f(b.x, b.y, b.z);
			rlVertex3f(b.x, b.y, b.z); rlVertex3f(c.x, c.y, c.z);
			rlVertex3f(c.x, c.y, c.z); rlVertex3f(a.x, a.y, a.z);
		};

	if (meshData.indices != nullptr && meshData.indexCount >= 3)
	{
		// Indexed mesh — walk the index buffer three at a time (assumes
		// R3D_PRIMITIVE_TRIANGLES, which is what generateMeshData() sets).
		for (int i = 0; i + 2 < meshData.indexCount; i += 3)
		{
			Vector3 a = meshData.vertices[meshData.indices[i + 0]].position;
			Vector3 b = meshData.vertices[meshData.indices[i + 1]].position;
			Vector3 c = meshData.vertices[meshData.indices[i + 2]].position;
			drawTriEdges(a, b, c);
		}
	}
	else
	{
		// Unindexed — vertices are already laid out in triangle order.
		for (int i = 0; i + 2 < meshData.vertexCount; i += 3)
		{
			Vector3 a = meshData.vertices[i + 0].position;
			Vector3 b = meshData.vertices[i + 1].position;
			Vector3 c = meshData.vertices[i + 2].position;
			drawTriEdges(a, b, c);
		}
	}

	rlEnd();
	rlPopMatrix();
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

BoundingBox shape::getWorldBoundingBox() const
{
	BoundingBox local = mesh.aabb;
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

	BoundingBox world = { first,first };

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
	Vector3 localPosition = Vector3Transform(worldPosition, inverseTransform);

	setVertexLocalPosition(index, localPosition);
}

void shape::applyTransform()
{
	R3D_TransformMeshData(&meshData, getMatrix());

	transform.translation = { 0.0f, 0.0f, 0.0f };
	transform.rotation = QuaternionIdentity();
	transform.scale = { 1.0f, 1.0f, 1.0f };

	meshDirty = true;
}

void shape::regenerateNormals()
{
	R3D_GenMeshDataNormals(&meshData, primitiveType);
	meshDirty = true;
}

void shape::regenerateTangents()
{
	R3D_GenMeshDataTangents(&meshData, primitiveType);
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
		return true;

	if (!R3D_IsMeshDataValid(meshData))
		return false;

	if (!R3D_IsMeshValid(mesh))
		return false;

	if (!R3D_UpdateMesh(&mesh, meshData, nullptr))
		return false;

	meshDirty = false;
	return true;
}

bool cube::generateMeshData()
{
	meshData = R3D_GenMeshDataCubeEx(width, height, length, resX, resY, resZ);
	return R3D_IsMeshDataValid(meshData);
}

int cube::cubeCount = 0;

cube::cube(
	Vector3 position, Quaternion rotation, Vector3 scale,
	float width, float height, float length,
	int resX, int resY, int resZ
)
	: shape(),
	width(width), height(height), length(length),
	resX(resX), resY(resY), resZ(resZ)
{
	setObjectType(ObjectType::CUBE);
	transform.translation = position;
	transform.rotation = rotation;
	transform.scale = scale;

	generateMeshData();
	primitiveType = R3D_PRIMITIVE_TRIANGLES;
	loadMesh();

	cubeCount++;
}

void cube::drawShape() {
	bool valid = R3D_IsMeshValid(mesh);
	if (!valid) TraceLog(LOG_WARNING, "cube %d: mesh invalid, skipping draw", getId());
	if (valid)
	{
		R3D_DrawMeshPro(mesh, *getMaterial(), MatrixCompose(transform.translation, transform.rotation, transform.scale));
		//if (getSelected()) drawSelectionWireframe(RED);
	}
}


bool sphere::generateMeshData()
{

	meshData = R3D_GenMeshDataSphere(radius, rings, slices); 
	return R3D_IsMeshDataValid(meshData);
}

int sphere::sphereCount = 0;

sphere::sphere(Vector3 position, Quaternion rotation, Vector3 scale, float radius, int rings, int slices)
	: shape(), radius(radius), rings(rings), slices(slices)
{
	setObjectType(ObjectType::SPHERE);
	transform.translation = position;
	transform.rotation = rotation;
	transform.scale = scale;

	generateMeshData();
	primitiveType = R3D_PRIMITIVE_TRIANGLES;
	loadMesh();

	sphereCount++;
}

void sphere::drawShape() {
	bool valid = R3D_IsMeshValid(mesh);
	if (!valid) TraceLog(LOG_WARNING, "sphere %d: mesh invalid, skipping draw", getId());
	if (valid)
		R3D_DrawMeshPro(mesh, *getMaterial(), MatrixCompose(transform.translation, transform.rotation, transform.scale));
}

bool cylinder::generateMeshData()
{
	meshData = R3D_GenMeshDataCylinder(radius, height, slices);
	return R3D_IsMeshDataValid(meshData);
}

int cylinder::cylinderCount = 0;

cylinder::cylinder(Vector3 position, Quaternion rotation, Vector3 scale, float radius, float height, int slices)
	: shape(), radius(radius), height(height), slices(slices)
{
	setObjectType(ObjectType::CYLINDER);
	transform.translation = position;
	transform.rotation = rotation;
	transform.scale = scale;

	generateMeshData();
	primitiveType = R3D_PRIMITIVE_TRIANGLES;
	loadMesh();

	cylinderCount++;
}

void cylinder::drawShape() {
	bool valid = R3D_IsMeshValid(mesh);
	if (!valid) TraceLog(LOG_WARNING, "cylinder %d: mesh invalid, skipping draw", getId());
	if (valid)
		R3D_DrawMeshPro(mesh, *getMaterial(), MatrixCompose(transform.translation, transform.rotation, transform.scale));
}

static Model cubeModel;
static Model sphereModel;
static Model cylinderModel;

void initModels()
{
	objects.push_back(std::make_unique<cube>());
	LoadPBRTextures();
}

bool updateObjectTransformGizmo(Camera3D camera, Rectangle viewport) {
	return UpdateTransformGizmo(camera, viewport);
}

void drawObjectTransformGizmo(Camera3D camera, Rectangle viewport) {
	(void)camera;
	(void)viewport;
	DrawTransformGizmo();
}

void selectObjectByRay(Ray ray) {
	shape* closestObject = nullptr;
	float closestDist = FLT_MAX;

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
	if (!additiveSelection)
	{
		for (auto& object : objects) object->setSelected(false);
		activeObject = nullptr;
	}

	if (closestObject != nullptr)
	{
		if (additiveSelection)
		{
			closestObject->setSelected(!closestObject->getSelected());
			if (closestObject->getSelected()) activeObject = closestObject;
			else if (closestObject == activeObject) activeObject = nullptr;
		}
		else
		{
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
		std::remove_if(
			objects.begin(),
			objects.end(),
			[](const std::unique_ptr<shape>& obj) { return obj->getSelected(); }
		),
		objects.end()
	);
	selectedObjects.clear();
	activeObject = nullptr;
}

void unloadModels(void) {
	UnloadModel(cubeModel);
	UnloadModel(sphereModel);
	UnloadModel(cylinderModel);
}
