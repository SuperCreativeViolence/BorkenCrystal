#include "KDTree.h"


KDNode* KDNode::Build(std::vector<Triangle*> &triangles_, int depth)
{
	KDNode* node = new KDNode();
	node->leaf = false;
	node->triangles = std::vector<Triangle*>();
	node->left = NULL;
	node->right = NULL;
	node->box = AABBox();

	if (triangles_.size() == 0) return node;

	if (depth > 25 || triangles_.size() <= 6)
	{
		node->triangles = triangles_;
		node->leaf = true;
		node->box = triangles_[0]->get_bounding_box();

		for (long i = 1; i < triangles_.size(); i++)
		{
			node->box.expand(triangles_[i]->get_bounding_box());
		}

		node->left = new KDNode();
		node->right = new KDNode();
		node->left->triangles = std::vector<Triangle*>();
		node->right->triangles = std::vector<Triangle*>();

		return node;
	}

	node->box = triangles_[0]->get_bounding_box();
	btVector3 midpt = btVector3(0, 0, 0);
	double tris_recp = 1.0 / triangles_.size();

	for (long i = 1; i < triangles_.size(); i++)
	{
		node->box.expand(triangles_[i]->get_bounding_box());
		midpt = midpt + (triangles_[i]->get_midpoint() * tris_recp);
	}

	std::vector<Triangle*> left_tris;
	std::vector<Triangle*> right_tris;
	int axis = node->box.get_longest_axis();

	for (long i = 0; i < triangles_.size(); i++)
	{
		switch (axis)
		{
			case 0:
				midpt[0] >= triangles_[i]->get_midpoint()[0] ? right_tris.push_back(triangles_[i]) : left_tris.push_back(triangles_[i]);
				break;
			case 1:
				midpt[1] >= triangles_[i]->get_midpoint()[1] ? right_tris.push_back(triangles_[i]) : left_tris.push_back(triangles_[i]);
				break;
			case 2:
				midpt[2] >= triangles_[i]->get_midpoint()[2] ? right_tris.push_back(triangles_[i]) : left_tris.push_back(triangles_[i]);
				break;
		}
	}

	if (triangles_.size() == left_tris.size() || triangles_.size() == right_tris.size())
	{
		node->triangles = triangles_;
		node->leaf = true;
		node->box = triangles_[0]->get_bounding_box();

		for (long i = 1; i < triangles_.size(); i++)
		{
			node->box.expand(triangles_[i]->get_bounding_box());
		}

		node->left = new KDNode();
		node->right = new KDNode();
		node->left->triangles = std::vector<Triangle*>();
		node->right->triangles = std::vector<Triangle*>();

		return node;
	}

	node->left = Build(left_tris, depth + 1);
	node->right = Build(right_tris, depth + 1);

	return node;
}

bool KDNode::Hit(KDNode* node, const Ray &ray, double &t, double &tmin, btVector3 &normal, Material &m, btTransform transform)
{
	double dist;
	assert(node->box);
	if (node->box.intersection(ray, dist, transform))
	{
		if (dist > tmin) return false;

		bool hit_tri = false;
		bool hit_left = false;
		bool hit_right = false;
		long tri_idx;

		if (!node->leaf)
		{
			//if ( node->left->triangles.size() > 0 )
			hit_left = Hit(node->left, ray, t, tmin, normal, m, transform);

			//if ( node->right->triangles.size() > 0 )
			hit_right = Hit(node->right, ray, t, tmin, normal, m, transform);

			return hit_left || hit_right;
		}
		else
		{
			long triangles_size = node->triangles.size();
			for (long i = 0; i < triangles_size; i++)
			{
				if (node->triangles[i]->Intersect(ray, t, tmin, normal, transform))
				{
					hit_tri = true;
					tmin = t;
					tri_idx = i;
				}
			}
			if (hit_tri)
			{
				btVector3 p = ray.origin + ray.direction * tmin;
				//c = node->triangles[tri_idx]->GetColorAt(p);
				m = node->triangles[tri_idx]->GetMaterial();
				return true;
			}
		}
	}
	return false;
}