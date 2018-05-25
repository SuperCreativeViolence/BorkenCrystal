#include <math.h>
#include <time.h>
#include <vector>
#include "Triangle.h"
#include "Object.h"


typedef struct Line {

	btVector3 point[2];

}Line;

std::vector<Mesh*> break_into_pieces(Mesh* mesh, int pieces);
std::vector<std::vector<Triangle*>*>* voronoi_Fracture(std::vector<Triangle*> triangles);

//voronoi �� �ݺ������� ȣ���ϴ� �Լ�, Object �� �ɰ� ������ �޾Ƽ� �ɰ��� Object ���� �迭�� ��ȯ��
std::vector<Mesh*> break_into_pieces(Mesh* mesh, int pieces)
{
	std::vector<Triangle*> triangles = mesh->GetTriangles();
	std::vector<std::vector<Triangle*>*>* triangles_sets = voronoi_Fracture(triangles);
	std::vector<std::vector<Triangle*>*>* two_triangles;
	srand(time(NULL));
	int random;
	for (int i = 2; i < pieces; i++) {
		random = rand() % size(*triangles_sets);
		triangles = *((*triangles_sets)[random]);
		two_triangles = voronoi_Fracture(triangles);
		(*triangles_sets).push_back((*two_triangles)[0]);
		(*triangles_sets).push_back((*two_triangles)[1]);
		(*triangles_sets).erase((*triangles_sets).begin() + random);
	}


	std::vector<Mesh*> meshes;
	Mesh *m;
	btVector3 *position;

	float x, y, z;
	for (int i = 0; i < size(*triangles_sets); i++)
	{

		triangles = *((*triangles_sets)[i]);
		x = 0, y = 0, z = 0;
		for (int j = 0; j < size(triangles); j++)
		{
			x += triangles[j]->pos[0].m_floats[0] + triangles[j]->pos[1].m_floats[0] + triangles[j]->pos[2].m_floats[0];
			y += triangles[j]->pos[0].m_floats[1] + triangles[j]->pos[1].m_floats[1] + triangles[j]->pos[2].m_floats[1];
			z += triangles[j]->pos[0].m_floats[2] + triangles[j]->pos[1].m_floats[2] + triangles[j]->pos[2].m_floats[2];
		}
		x /= size(triangles);
		y /= size(triangles);
		z /= size(triangles);
		position = new btVector3(x, y, z);
		m = new Mesh(*position, triangles, 1, SPEC);
		meshes.push_back(m);
	}


	return meshes;
}






// Object, �� �ٸ�ü ���� ������ 2���� ���� ��� �� ���� �������� 2���� �ٸ�ü�� ������
std::vector<std::vector<Triangle*>*>* voronoi_Fracture(std::vector<Triangle*> triangles)
{

	// 1. bounding volume ����
	float min_x = 10000000, min_y = 10000000, min_z = 10000000, max_x = -10000000, max_y = -10000000, max_z = -10000000;

	for (auto & triangle : triangles)
		for (int i = 0; i < 3; i++) {
			if (triangle->pos[i].m_floats[0] > max_x)
				max_x = triangle->pos[i].m_floats[0];
			else if (triangle->pos[i].m_floats[0] < min_x)
				min_x = triangle->pos[i].m_floats[0];

			if (triangle->pos[i].m_floats[1] > max_y)
				max_y = triangle->pos[i].m_floats[1];
			else if (triangle->pos[i].m_floats[1] < min_y)
				min_y = triangle->pos[i].m_floats[1];

			if (triangle->pos[i].m_floats[2] > max_z)
				max_z = triangle->pos[i].m_floats[2];
			else if (triangle->pos[i].m_floats[2] < min_z)
				min_z = triangle->pos[i].m_floats[2];
		}


	srand(time(NULL));
	float width_x = max_x - min_x;
	float width_y = max_y - min_y;
	float width_z = max_z - min_z;
	int r;
	bool isInObj;
	btVector3 p[2];
	btVector3 normal, point_normal;
	float p_x, p_y, p_z;

	// ���� point 2�� ����
	int ccount = 0;
	for (int i = 0; i < 2; i++) {
		isInObj = true;
		r = rand() % 100000;
		p_x = r / 100000.0 * width_x + min_x;
		r = rand() % 100000;
		p_y = r / 100000.0 * width_y + min_y;
		r = rand() % 100000;
		p_z = r / 100000.0 * width_z + min_z;

		p[i] = btVector3(p_x, p_y, p_z);

		// ���� ������ ���� �ٸ�ü ���� �ִ� ���� �´��� üũ
		for (auto & triangle : triangles) {
			normal = (triangle->pos[1] - triangle->pos[0]).cross(triangle->pos[2] - triangle->pos[0]).normalize();
			point_normal = (triangle->pos[0] - p[i]).normalize();
			if (point_normal.dot(normal) <= 0) {
				isInObj = false;
				break;
			}
		}

		//�ٸ�ü ���̸� �ٽ� ����
		if (!isInObj)
			i--;
		
	}


	// �־��� �� ���� �����̵���ϴ� plane ����
	// p_middle �� ��� ���� ��, p_normal �� ����� normal Vector
	btVector3 p_middle = btVector3((p[0] + p[1]) / 2);
	btVector3 p_normal = btVector3(p[0] - p[1]);
	btVector3 p_division[2];
	int division_factor;
	int divided_line[2];
	int division_point;
	int line_num = 0;
	Triangle *newTriangle, *newTriangle_1, *newTriangle_2;
	std::vector<Triangle*>* obj_1_triangles = new(std::vector<Triangle*>);
	std::vector<Triangle*>* obj_2_triangles = new(std::vector<Triangle*>);
	Line *line;
	std::vector<Line*> lines;
	Material material = SPEC;




	// ��� �ﰢ���� ����
	for (auto & triangle : triangles) {

		// �ش� �ﰢ���� ����� �����ٸ� d_factor = 2, �������� �����ؼ� ������ = 1, ������ ������  = 0
		division_factor = 0;

		for (int i = 0; i < 3; i++) {

			// �ﰢ���� �� ������ ����� ��������(������ ����� �����ϴ���) �Ǻ�
			if ((p_normal.dot(triangle->pos[i]) - p_normal.dot(p_middle))
				* (p_normal.dot(triangle->pos[(i + 1) % 3]) - p_normal.dot(p_middle)) < 0) {

				// ���������� ���� ��ġ
				p_division[division_factor] = p_normal.dot(p_middle - triangle->pos[i]) /
					p_normal.dot(triangle->pos[(i + 1) % 3] - triangle->pos[i]) * (triangle->pos[(i + 1) % 3] - triangle->pos[i])
					+ triangle->pos[i];

				divided_line[division_factor++] = i;

			}
		}

		if (division_factor == 2) { // �ﰢ���� �ﰢ�� + �簢������ ���� ���

									// ���� 0,1 �� �߶����� �� �� 1 �� �� ���� ����, 1,2 �� �߶����� �� �� 2, 2,0 �� �߶����� �� �� 0
			division_point = (divided_line[0] + divided_line[1] + 1) * 2 % 3;

			// �ﰢ���� �ﰢ�� + �簢������ ������ ��, ������ �����ϴ� ���� �ﰢ���̰� �ƴ� ���� �簢��
			// p1, p2 �� ����� ���� object �� ���ԵǹǷ� �Ǻ�

			if ((p[0] - triangle->pos[division_point]).length() < (p[1] - triangle->pos[division_point]).length())
			{

				if (division_point == 0)
				{
					newTriangle = new Triangle(triangle->pos[division_point], p_division[0], p_division[1], material);
					newTriangle_1 = new Triangle(triangle->pos[1], triangle->pos[2], p_division[0], material);
					newTriangle_2 = new Triangle(triangle->pos[2], p_division[1], p_division[0], material);
				}
				else if (division_point == 1)
				{
					newTriangle = new Triangle(triangle->pos[division_point], p_division[1], p_division[0], material);
					newTriangle_1 = new Triangle(triangle->pos[0], p_division[1], triangle->pos[2], material);
					newTriangle_2 = new Triangle(triangle->pos[0], p_division[0], p_division[1], material);
				}
				else if (division_point == 2)
				{
					newTriangle = new Triangle(triangle->pos[division_point], p_division[1], p_division[0], material);
					newTriangle_1 = new Triangle(triangle->pos[0], triangle->pos[1], p_division[1], material);
					newTriangle_2 = new Triangle(triangle->pos[1], p_division[0], p_division[1], material);
				}

				(*obj_1_triangles).push_back(newTriangle);
				(*obj_2_triangles).push_back(newTriangle_1);
				(*obj_2_triangles).push_back(newTriangle_2);
				line = new Line();
				line->point[0] = p_division[0];
				line->point[1] = p_division[1];
				lines.push_back(line);
			}

			else
			{
				if (division_point == 0)
				{
					newTriangle = new Triangle(triangle->pos[division_point], p_division[0], p_division[1], material);
					newTriangle_1 = new Triangle(triangle->pos[1], triangle->pos[2], p_division[0], material);
					newTriangle_2 = new Triangle(triangle->pos[2], p_division[1], p_division[0], material);
				}
				else if (division_point == 1)
				{
					newTriangle = new Triangle(triangle->pos[division_point], p_division[1], p_division[0], material);
					newTriangle_1 = new Triangle(triangle->pos[0], p_division[1], triangle->pos[2], material);
					newTriangle_2 = new Triangle(triangle->pos[0], p_division[0], p_division[1], material);
				}
				else if (division_point == 2)
				{
					newTriangle = new Triangle(triangle->pos[division_point], p_division[1], p_division[0], material);
					newTriangle_1 = new Triangle(triangle->pos[0], triangle->pos[1], p_division[1], material);
					newTriangle_2 = new Triangle(triangle->pos[1], p_division[0], p_division[1], material);
				}

				(*obj_2_triangles).push_back(newTriangle);
				(*obj_1_triangles).push_back(newTriangle_1);
				(*obj_1_triangles).push_back(newTriangle_2);
				line = new Line();
				line->point[0] = p_division[0];
				line->point[1] = p_division[1];
				lines.push_back(line);

			}

		}

		else if (division_factor == 1) { // �ﰢ���� �� �ﰢ������ ���� ���, �� �������� �����鼭 ���� ���
										 // �ﰢ�� 2���� push_back
			if ((p[0] - triangle->pos[divided_line[0]]).length() < (p[1] - triangle->pos[divided_line[0]]).length())
			{
				newTriangle_1 = new Triangle(triangle->pos[(divided_line[0] + 2) % 3], triangle->pos[divided_line[0]], p_division[0], material);
				(*obj_1_triangles).push_back(newTriangle_1);
				newTriangle_2 = new Triangle(triangle->pos[(divided_line[0] + 1) % 3], triangle->pos[(divided_line[0] + 2) % 3], p_division[0], material);
				(*obj_2_triangles).push_back(newTriangle_2);
			}
			else
			{
				newTriangle_1 = new Triangle(triangle->pos[(divided_line[0] + 1) % 3], triangle->pos[(divided_line[0] + 2) % 3], p_division[0], material);
				(*obj_1_triangles).push_back(newTriangle_1);
				newTriangle_2 = new Triangle(triangle->pos[(divided_line[0] + 2) % 3], triangle->pos[divided_line[0]], p_division[0], material);
				(*obj_2_triangles).push_back(newTriangle_2);
			}

			line = new Line();
			line->point[0] = p_division[0];
			line->point[1] = triangle->pos[(divided_line[0] + 2) % 3];
			lines.push_back(line);
		}

		else {
			if ((triangle->pos[0] - p[0]).length() < (triangle->pos[0] - p[1]).length())
				(*obj_1_triangles).push_back(triangle);
			else
				(*obj_2_triangles).push_back(triangle);
		}
	}

	// ������ ������ �ܸ��� trianglize �Ͽ� ���� object �� ��� �Ҵ�

	float avg[3] = { 0, 0, 0 };
	btVector3 *avg_p, a, b, c;

	for (auto & line : lines)
		for (int i = 0; i < 3; i++) 
			avg[i] += line->point[0].m_floats[i] + line->point[1].m_floats[i];

	for (int i = 0; i < 3; i++)
		avg[i] /= (size(lines) * 2);

	avg_p = new btVector3(avg[0], avg[1], avg[2]);

	for (auto & line : lines) {
		a = line->point[0];
		b = line->point[1];
		c = *avg_p;

		normal = (b - a).cross(c - a).normalize();
		point_normal = (a - p[0]).normalize();
		if (normal.dot(point_normal) > 0) {
			newTriangle_1 = new Triangle(a, b, c, material);
			newTriangle_2 = new Triangle(a, c, b, material);
		}
		else {
			newTriangle_1 = new Triangle(a, c, b, material);
			newTriangle_2 = new Triangle(a, b, c, material);
		}
		(*obj_1_triangles).push_back(newTriangle_1);
		(*obj_2_triangles).push_back(newTriangle_2);
	}

	//

	std::vector<std::vector<Triangle*>*>* result = new(std::vector<std::vector<Triangle*>*>);
	(*result).push_back(obj_1_triangles);
	(*result).push_back(obj_2_triangles);


	return result;
}