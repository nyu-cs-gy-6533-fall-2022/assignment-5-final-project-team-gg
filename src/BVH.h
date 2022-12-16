#ifndef BVH
#define BVH

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <algorithm>


float min(float a, float b, float c){
    return (a > b) ? (b > c ? c : b) : (a > c ? c : a);
}

float max(float a, float b, float c){
    return (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c);
}

float min(float a, float b){
    return a > b ? b : a;
}

float max(float a, float b){
    return a > b ? a : b;
}

struct Triangle{
    glm::vec3 v1;
    glm::vec3 v2;
    glm::vec3 v3;
    //int index;
    bool is_reflecting;
    glm::vec3 color;

    Triangle(glm::vec3 vertex1, glm::vec3 vertex2, glm::vec3 vertex3, glm::vec3 c = glm::vec3(0.0f,0.0f,0.0f), bool reflecting = false){
        v1= vertex1; v2 = vertex2; v3 = vertex3;
        color = c;
        is_reflecting = reflecting;
    }

    Triangle(){}

    glm::vec3 min_vertex(){
        return glm::vec3(min(v1.x, v2.x, v3.x), min(v1.y, v2.y, v3.y), min(v1.z, v2.z, v3.z));
    }

    glm::vec3 max_vertex(){
        return glm::vec3(max(v1.x, v2.x, v3.x), max(v1.y, v2.y, v3.y), max(v1.z, v2.z, v3.z));
    }

    glm::vec3 minn(glm::vec3 v){
        return glm::vec3(min(min_vertex().x, v.x), min(min_vertex().y, v.y), min(min_vertex().z, v.z));
    }

    glm::vec3 maxx(glm::vec3 v){
        return glm::vec3(max(max_vertex().x, v.x), max(max_vertex().y, v.y), max(max_vertex().z, v.z));
    }

};

class bvh{
public:
    glm::vec3 minv;
    glm::vec3 maxv;
    bool is_leaf;
    bvh* left;
    bvh* right;
    Triangle t;

    bvh(glm::vec3 min_v, glm::vec3 max_v, bool is_root = false, Triangle triangle = Triangle()){
        minv = min_v;
        maxv = max_v;
        is_leaf = is_root;
        t = triangle;
        left = NULL;
        right = NULL;
    }

    int get_left(int root){ return root * 2 + 3;}
    int get_right(int root){ return root * 2 + 3 + 3;}

};

bool compare_X(Triangle t1, Triangle t2){
    return (t1.v1.x + t1.v2.x + t1.v3.x) > (t2.v1.x + t2.v2.x + t2.v3.x);
}

bool compare_Y(Triangle t1, Triangle t2){
    return (t1.v1.y + t1.v2.y + t1.v3.y) > (t2.v1.y + t2.v2.y + t2.v3.y);
}

bool compare_Z(Triangle t1, Triangle t2){
    return (t1.v1.z + t1.v2.z + t1.v3.z) > (t2.v1.z + t2.v2.z + t2.v3.z);
}

void write_vec3(float* offset, glm::vec3 v){
    *offset = v.x;
    *(offset + 1) = v.y;
    *(offset + 2) = v.z;
}

// axis: x->0, y->1, z->2
bvh* create_bvh(std::vector<Triangle>& tria, int begin, int end, int axis){
    if(begin + 1 == end){
        return new bvh(tria[begin].min_vertex(), tria[begin].max_vertex(), true, tria[begin]);
    }
    switch(axis){
        case 0: // x-axis
            sort(tria.begin() + begin, tria.begin() + end, compare_X);
            break;
        case 1: // y-axis
            sort(tria.begin() + begin, tria.begin() + end, compare_Y);
            break;
        case 2: // z-axis
            sort(tria.begin() + begin, tria.begin() + end, compare_Z);
            break;
    }
    glm::vec3 minV = tria[begin].min_vertex(), maxV = tria[begin].max_vertex();
    for(int i = begin; i < end; ++i){
        minV = tria[i].minn(minV);
        maxV = tria[i].maxx(maxV);
    }
    bvh* _bvh = new bvh(minV, maxV);
    switch(axis){
        case 0: // x-axis
            _bvh->left = create_bvh(tria, begin, begin + (end - begin) / 2, 1);
            _bvh->right = create_bvh(tria, begin + (end - begin) / 2, end, 1);
            break;
        case 1: // y-axis
            _bvh->left = create_bvh(tria, begin, begin + (end - begin) / 2, 2);
            _bvh->right = create_bvh(tria, begin + (end - begin) / 2, end, 2);
            break;
        case 2: // z-axis
            _bvh->left = create_bvh(tria, begin, begin + (end - begin) / 2, 0);
            _bvh->right = create_bvh(tria, begin + (end - begin) / 2, end, 0);
            break;
    }
    return _bvh;
}

void write_bvh_tbo(float* _bvh, float* tria, bvh* root, int i, int &idx){
    if(root == nullptr) return;
    // _write the bvh
    _bvh[i * 3] = root->minv.x;
    _bvh[i * 3 + 1] = root->minv.x;
    _bvh[i * 3 + 2] = root->minv.x;
    _bvh[(i + 1) * 3] = root->maxv.x;
    _bvh[(i + 1) * 3 + 1] = root->maxv.x;
    _bvh[(i + 1) * 3 + 2] = root->maxv.x;
    _bvh[(i + 2) * 3] = root->is_leaf ? (float) idx : -1;
    _bvh[(i + 2) * 3 + 1] = 0.0f;
    _bvh[(i + 2) * 3 + 2] = 0.0f;
    if(root->is_leaf){
        write_vec3(tria + idx * 8 * 3, root->t.v1);
		write_vec3(tria + (idx * 8 + 1) * 3, root->t.v2);
		write_vec3(tria + (idx * 8 + 2) * 3, root->t.v3);
		write_vec3(tria + (idx * 8 + 3) * 3, glm::vec3(0,0,0));
		write_vec3(tria + (idx * 8 + 4) * 3, glm::vec3(0,0,0));
		write_vec3(tria + (idx * 8 + 5) * 3, glm::vec3(0,0,0));
		write_vec3(tria + (idx * 8 + 6) * 3, root->t.color);
		write_vec3(tria + (idx * 8 + 7) * 3, glm::vec3(root->t.is_reflecting ? 5.0f : -1.0f,0.0f,0.0f));
		idx++;
    }
    write_bvh_tbo(_bvh, tria, root->left, root->get_left(i), idx);
    write_bvh_tbo(_bvh, tria, root->right, root->get_right(i), idx);
}



int bvh_leaves(bvh* root){
    int nodes = 0;
	if (root == NULL)
		return 0;
	else if (root->left == NULL && root->right == NULL)
		return 1;
	else
		nodes = bvh_leaves(root->left) + bvh_leaves(root->right);
	return nodes;
}

int bvh_depth(bvh* node){
    if(node == NULL){
        return 0;
    }
    int left = bvh_depth(node->left);
    int right = bvh_depth(node->right);
    return (left > right) ? (left + 1) : (right + 1);
}

void generate_bvh_tbo(bvh* root, float* _bvh, float* tria, int &bvh_size, int &tria_size){
    bvh_size = (pow(2, bvh_depth(root)) - 1) * 9;
    tria_size = bvh_leaves(root) * 24;
    _bvh = (float*)malloc(bvh_size * sizeof(float));
    tria = (float*)malloc(tria_size * sizeof(float));
    memset(_bvh, 0, bvh_size * sizeof(float));
    memset(tria, 0, tria_size * sizeof(float));
    int idx = 0;
    write_bvh_tbo(_bvh, tria, root, 0, idx);
}

#endif