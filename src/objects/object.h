#ifndef object_h
#define object_h

#include "raylib.h"
#include "features/manipulation/Transform.h"
#include <float.h>
#include "raymath.h"
#include "rlgl.h"
#include "features/shadings/textures.h"
#include "features/shadings/lighting.h"
#include "data/save_load/saveNload.h"
#include <iostream>
#include <vector>
#include <memory>
#include <r3d.h>

enum class ObjectType { NONE = -1, CUBE, SPHERE, CYLINDER, CUSTOM };


void initModels();
// Unload the models, required because the objects are defined static, and 
void unloadModels();
void clearScene();

void deleteObjects();

bool updateObjectTransformGizmo(Camera3D camera, Rectangle viewport);
void drawObjectTransformGizmo(Camera3D camera, Rectangle viewport);

class shape
{
protected:
	// Object transform
	Transform transform;

	// CPU-side editable geometry
	R3D_MeshData meshData;

	// GPU-side renderable mesh
	R3D_Mesh mesh;

	ObjectType objectType;
	// Mesh primitive type
	R3D_PrimitiveType primitiveType = R3D_PRIMITIVE_TRIANGLES;

	R3D_Material* material = NULL; // Initialized in constructor to point to a valid default material

	MaterialType materialType = MATERIAL_NONE;


	// Object state
	bool isSelected = false;
	bool meshDirty = false; // Mesh becomes dirty if mesh data is changed

	unsigned int id;
	inline static unsigned int nextId = 0;

public:
	// Constructor / Destructor
	shape();
	virtual ~shape();
	BoundingBox getWorldBoundingBox() const;
	// Transform

	Transform& getTransform() {
		return transform;
	}
	const Transform& getTransform() const {
		return transform;
	}
	void setTransform(const Transform& newTransform) {
		if (QuaternionEquals(newTransform.rotation,QuaternionIdentity()) && Vector3Equals(newTransform.scale,Vector3Zero()) && Vector3Equals(newTransform.translation, Vector3One()))
		{
			return;
		}
		transform = newTransform;
	}

	// Material
	void applyMaterial(MaterialType);
	MaterialType getMaterialType() const {
		return materialType;
	}
	void setMaterialType(MaterialType newType) {
		applyMaterial(newType);
	}
	R3D_Material* getMaterial()
	{
		return material;
	}
	// Object Type
	ObjectType getObjectType() const {
		return objectType;
	}
	void setObjectType(ObjectType newType) {
		objectType = newType;
	}
	const char* getObjectTypeString() const {
		switch (objectType) {
		case ObjectType::CUBE: return "Cube";
		case ObjectType::SPHERE: return "Sphere";
		case ObjectType::CYLINDER: return "Cylinder";
		case ObjectType::CUSTOM: return "Custom";
		default: return "Unknown";
		}
	}
	// Just makes use of MatrixCompose to get the Transform matrix from the transform struct
	Matrix getMatrix() const {
		return MatrixCompose(transform.translation, transform.rotation, transform.scale);
	}

	// Getters
	const R3D_MeshData& getMeshData() const {
		return meshData;
	}
	void setMeshData(const R3D_MeshData& newMeshData) {
		if (!R3D_IsMeshDataValid(newMeshData)) {
			return;
			TRACELOG(LOG_WARNING, "Attempted to set invalid mesh data for object ID %u", id);
		}
		meshData = newMeshData;
		loadMesh(); // load mesh does sync mesh
	}

	int getVertexCount() const {
		return meshData.vertexCount;
	}
	int getIndexCount() const {
		return meshData.indexCount;
	}

	void shape::drawSelectionWireframe(Color color) const;

	unsigned int getId() const
	{
		return id;
	}

	// Selection
	bool getSelected() const;
	void setSelected(bool selected);

	// Local-space vertex access
	Vector3 getVertexLocalPosition(int index) const;
	void setVertexLocalPosition(int index, Vector3 position);

	// World-space vertex access
	Vector3 getVertexWorldPosition(int index) const;
	void setVertexWorldPosition(int index, Vector3 position);

	// Apply object transform to mesh data
	void applyTransform();

	// Mesh processing
	void regenerateNormals();
	void regenerateTangents();

	BoundingBox getBoundingBox() const;



	// Dirty state
	bool isMeshDirty() const;

	virtual bool generateMeshData() = 0;
	bool loadMesh();
	// GPU synchronization
	virtual bool syncMesh();

	// Rendering
	virtual void drawShape() = 0;
};





class cube : public shape
{
private:
	float width;
	float height;
	float length;
	int resX, resY, resZ;

	ObjectType objectType;
public:
	static int cubeCount;

	static int getTotalCount() {
		return cubeCount;
	}

	cube(
		Vector3 position = { 0.0f, 0.0f, 0.0f },
		Quaternion rotation = QuaternionIdentity(),
		Vector3 scale = { 1.0f, 1.0f, 1.0f },
		float width = 1.0f,
		float height = 1.0f,
		float length = 1.0f,
		int resX = 1, int resY = 1, int resZ = 1
	);

	bool generateMeshData() override;
	void drawShape() override;

	float getWidth() const { return width; }
	float getHeight() const { return height; }
	float getLength() const { return length; }

};

class sphere : public shape
{
private:
	float radius;
	int rings, slices;

	ObjectType objectType;
public:
	static int sphereCount;

	static int getTotalCount() {
		return sphereCount;
	}

	sphere(
		Vector3 position = { 0.0f, 0.0f, 0.0f },
		Quaternion rotation = QuaternionIdentity(),
		Vector3 scale = { 1.0f, 1.0f, 1.0f },
		float radius = 0.5f,
		int rings = 16, int slices = 16
	);

	bool generateMeshData() override;
	void drawShape() override;

	float getRadius() const { return radius; }
};

class cylinder : public shape
{
private:
	float radius;
	float height;
	int slices;

	ObjectType objectType;
public:
	static int cylinderCount;

	static int getTotalCount() {
		return cylinderCount;
	}

	cylinder(
		Vector3 position = { 0.0f, 0.0f, 0.0f },
		Quaternion rotation = QuaternionIdentity(),
		Vector3 scale = { 1.0f, 1.0f, 1.0f },
		float radius = 0.5f,
		float height = 1.0f,
		int slices = 16
	);

	bool generateMeshData() override;
	void drawShape() override;

	float getRadius() const { return radius; }
	float getHeight() const { return height; }
};

void selectObjectByRay(Ray ray);
void selectObjects(shape* object, bool);

// All object arrays
extern std::vector<std::unique_ptr<shape>> objects;
// Selected objects array, stores pointers to the selected objects in the scene
extern std::vector<shape*> selectedObjects;
// Active object pointer, points to the last selected object
// use for entering edit mode, expanding split screen for one specific object, etc.
extern shape* activeObject;
#endif // object_h
