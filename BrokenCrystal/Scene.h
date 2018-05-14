#ifndef SCENE_H
#define SCENE_H
#undef NDEBUG
#include <BulletPhysics/btBulletDynamicsCommon.h>
#include "Object.h"
#include "Camera.h"
#include "InputManager.h"
#include "lodepng.h"
#include <gl/freeglut.h>

#include <iostream>
#include <vector>
#include <set>
#include <iterator>
#include <algorithm>

// Draw ����
#include <BulletPhysics/BulletCollision/CollisionShapes/btTriangleMeshShape.h>
#include <BulletPhysics/BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletPhysics/BulletCollision/CollisionShapes/btConvexPolyhedron.h>

#define AT_R(row, col) ((col) * width * 3 + ((row) * 3))
#define AT_G(row, col) (AT_R(row, col) + 1)
#define AT_B(row, col) (AT_R(row, col) + 2)

// Object ��� ������ ����
typedef std::vector<Object*> Objects;

// �浹 �̺�Ʈ�� ����
typedef std::pair<const btRigidBody*, const btRigidBody*> CollisionPair;
typedef std::set<CollisionPair> CollisionPairs;


// �����ɽ��ð� ����
struct RayResult
{
	btRigidBody* pBody;
	btVector3 hitPoint;
	btVector3 hitNormal;
};

class Scene
{
public:
	Scene();
	Scene(int width, int height, int maxLevel, bool antialiasing);
	~Scene();

	Camera* camera;

	void Initialize();

	void Idle();
	void Reshape(int w, int h);

	void RenderScene();
	void UpdateScene(float deltaTime);
	void InitializePhysics();
	void ShutdownPhysics();
	void CreateObjects();

	Object* CreateObject(btCollisionShape* pShape, const float &mass,
		const btVector3 &color = btVector3(1.0f, 1.0f, 1.0f),
		const btVector3 &initialPosition = btVector3(0.0f, 0.0f, 0.0f),
		const btQuaternion &initialRotation = btQuaternion(0, 0, 1, 1));

	// �浹 �̺�Ʈ �Լ���
	void CheckForCollisionEvents();
	void CollisionEvent(btRigidBody* body0, btRigidBody * body1);
	void SeparationEvent(btRigidBody * body0, btRigidBody * body1);

	// �����ɽ���
	bool RayCast(const btVector3 &start, const btVector3 &dir, RayResult &out, bool includeStatic = false);
	void CreatePickingConstraint(int x, int y);
	void RemovePickingConstraint();
	void ApplyCentralForce(int x, int y, float power);

	// ����Ʈ���̽�
	void RayTrace();
	btVector3 Trace(const btVector3 &from, const btVector3 &to, int level);

	// �н�Ʈ���̽�
	void PathTrace(int samples);
	btVector3 TracePath(const Ray &ray, int depth, unsigned short *Xi);
	ObjectIntersection intersect(const Ray &ray);
	void SaveImage(const char *file_path);

	void CreateSphere(const btVector3& position, const float& radius, const Material& material, const float &mass);

private:
	void DrawAxis(int size);
	void DrawGrid(float size, float step);
	void DrawBox(const btVector3 &halfSize);
	void DrawSphere(btScalar radius, int lats, int longs);
	void DrawShape(btScalar* transform, const btCollisionShape* pShape, const btVector3 &color);

	Objects objects;
	btClock clock;

	// Bullet Physics �ʼ� ������Ʈ
	btBroadphaseInterface* broadphase;
	btCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btConstraintSolver* solver;
	btDynamicsWorld* world;

	// �浹 ���� ������Ʈ
	CollisionPairs collisionPairs_LastUpdate;
	btRigidBody* pickedBody;
	btTypedConstraint* pickConstraint;
	btScalar oldPickingDistance;

	// ��ο� ���� (���߿� Drawer�� ���� ����)
	struct ShapeCache
	{
		struct Edge
		{
			btVector3 n[2]; int v[2];
		};
		ShapeCache(btConvexShape* s) : m_shapehull(s)
		{
		}
		btShapeHull					m_shapehull;
		btAlignedObjectArray<Edge>	m_edges;
	};
	btAlignedObjectArray<ShapeCache*>	m_shapecaches;
	ShapeCache* cache(btConvexShape* shape);

	// ����Ʈ���̽� ����
	int height;
	int width;
	int maxLevel;
	bool antialiasing;
	float *pixels;
	bool test = false;

	// �н�Ʈ���̽� ����
	int samples = 500;
	btVector3 *pixel_buffer;
};

#endif