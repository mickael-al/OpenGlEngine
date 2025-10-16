#ifndef __MORTON_OCTREE__
#define __MORTON_OCTREE__

#include <vector>
#include <cstdint>
#include <array>
#include <queue>
#include <limits>
#include <glm/glm.hpp>

// MortonOctree: construit un octree à partir de Morton codes déjà triés.
// Les points et morton codes doivent être alignés (m_points[i] correspond à m_morton[i]).

struct MortonOctree
{
    struct Node
    {
        glm::vec3 min, max;     // AABB du nœud
        size_t start, end;      // plage [start,end)
        int children[8];        // indices vers nodes (-1 si absent)
        bool leaf;

        Node(const glm::vec3& lo = glm::vec3(0.0f), const glm::vec3& hi = glm::vec3(0.0f), size_t s = 0, size_t e = 0)            
        {
            min = lo;
            max = hi;
            start = s;
            end = e;
            leaf = true;
            std::fill(std::begin(children), std::end(children), -1);
        }
    };

    std::vector<Node> nodes;
    std::vector<unsigned int> indices; // indices alignés sur m_points triés
    const std::vector<glm::vec3>* points = nullptr;
    const std::vector<uint64_t>* morton = nullptr;

    unsigned int maxDepth;
    unsigned int maxLeaf;
    static constexpr unsigned int bitsPerCoord = 21; // correspond à morton3D_64

    MortonOctree(const std::vector<glm::vec3>& pts,
        const std::vector<uint64_t>& mortonCodes,
        const glm::vec3& boundsMin,
        const glm::vec3& boundsMax,
        unsigned int maxDepth_ = 16,
        unsigned int maxLeaf_ = 8)
        : points(&pts), morton(&mortonCodes), maxDepth(maxDepth_), maxLeaf(maxLeaf_)
    {
        if (pts.empty()) { return; }

        indices.resize(pts.size());
        for (size_t i = 0; i < pts.size(); ++i) { indices[i] = static_cast<unsigned int>(i); }

        glm::vec3 bmin = boundsMin;
        glm::vec3 bmax = boundsMax;
        const float eps = 0.0001f;
        if (abs(bmax.x - bmin.x) <= eps) { bmin.x -= eps; bmax.x += eps; }
        if (abs(bmax.y - bmin.y) <= eps) { bmin.y -= eps; bmax.y += eps; }
        if (abs(bmax.z - bmin.z) <= eps) { bmin.z -= eps; bmax.z += eps; }

        nodes.reserve(1024);
        nodes.emplace_back(bmin, bmax, 0, pts.size());
        nodes[0].leaf = false;
        buildNode(0, 0);
    }

    // Renvoie toutes les bounding boxes à une profondeur donnée
    std::vector<std::pair<glm::vec3, glm::vec3>> getBoundingBoxesAtDepth(unsigned int targetDepth) const
    {
        std::vector<std::pair<glm::vec3, glm::vec3>> boxes;
        if (nodes.empty()) return boxes;

        struct Item { int nodeIdx; unsigned int depth; };
        std::queue<Item> q;
        q.push({ 0, 0 });

        while (!q.empty())
        {
            Item it = q.front(); q.pop();
            const Node& node = nodes[it.nodeIdx];

            if (it.depth == targetDepth)
            {
                boxes.emplace_back(node.min, node.max);
                continue;
            }

            if (it.depth < targetDepth)
            {
                for (int c = 0; c < 8; ++c)
                {
                    int child = node.children[c];
                    if (child != -1)
                        q.push({ child, it.depth + 1 });
                }
            }
        }

        return boxes;
    }


private:
    void buildNode(int nodeIdx, unsigned int depth)
    {
        Node& node = nodes[nodeIdx];
        size_t count = node.end - node.start;

        if (count == 0)
        {
            node.leaf = true;
            return;
        }

        if (count <= maxLeaf || depth >= maxDepth || depth >= bitsPerCoord)
        {
            node.leaf = true;
            return;
        }

        unsigned int bitPos = bitsPerCoord - 1 - depth;
        unsigned int shift = 3u * bitPos;

        std::array<size_t, 8> counts{};
        for (size_t i = node.start; i < node.end; ++i)
        {
            int child = static_cast<int>(((*morton)[i] >> shift) & 0x7u);
            counts[child]++;
        }

        int nonZero = 0;
        for (int i = 0; i < 8; ++i)
        {
            if (counts[i] > 0)
            {
                ++nonZero;
            }
        }
        if (nonZero <= 1)
        {
            node.leaf = true;
            return;
        }

        std::array<size_t, 8> offsets{};
        offsets[0] = node.start;
        for (int i = 1; i < 8; ++i)
        {
            offsets[i] = offsets[i - 1] + counts[i - 1];
        }

        std::vector<unsigned int> tmpIdx(count);
        std::array<size_t, 8> writePos = offsets;

        for (size_t i = node.start; i < node.end; ++i)
        {
            int child = static_cast<int>(((*morton)[i] >> shift) & 0x7u);
            size_t pos = writePos[child]++;
            tmpIdx[pos - node.start] = indices[i];
        }

        for (size_t k = 0; k < count; ++k)
        {
            indices[node.start + k] = tmpIdx[k];
        }

        glm::vec3 mid = (node.min + node.max) * 0.5f;
        for (int c = 0; c < 8; ++c)
        {
            size_t cStart = offsets[c];
            size_t cEnd = cStart + counts[c];
            if (cStart >= cEnd)
            {
                node.children[c] = -1;
                continue;
            }

            glm::vec3 cmin = node.min;
            glm::vec3 cmax = node.max;

            if (c & 1) { cmin.x = mid.x; }
            else { cmax.x = mid.x; }
            if (c & 2) { cmin.y = mid.y; }
            else { cmax.y = mid.y; }
            if (c & 4) { cmin.z = mid.z; }
            else { cmax.z = mid.z; }

            int childNodeIdx = static_cast<int>(nodes.size());
            nodes.emplace_back(cmin, cmax, cStart, cEnd);
            nodes[childNodeIdx].leaf = false;
            node.children[c] = childNodeIdx;

            buildNode(childNodeIdx, depth + 1);
        }
    }

public:
    int findNearestIndex(const glm::vec3& target) const
    {
        if (!points || points->empty() || nodes.empty())
        {
            return -1;
        }

        auto sqDistAABB = [](const glm::vec3& p, const glm::vec3& mn, const glm::vec3& mx) -> float {
            float d = 0.0f;
            if (p.x < mn.x) { float t = mn.x - p.x; d += t * t; }
            else if (p.x > mx.x) { float t = p.x - mx.x; d += t * t; }
            if (p.y < mn.y) { float t = mn.y - p.y; d += t * t; }
            else if (p.y > mx.y) { float t = p.y - mx.y; d += t * t; }
            if (p.z < mn.z) { float t = mn.z - p.z; d += t * t; }
            else if (p.z > mx.z) { float t = p.z - mx.z; d += t * t; }
            return d;
        };

        struct Item { float dist; int nodeIdx; };
        struct Cmp { bool operator()(Item const& a, Item const& b) const { return a.dist > b.dist; } };

        std::priority_queue<Item, std::vector<Item>, Cmp> pq;
        pq.push({ sqDistAABB(target, nodes[0].min, nodes[0].max), 0 });

        float bestDist2 = 3.4028235E+38;
        int bestIndex = -1;

        while (!pq.empty())
        {
            Item it = pq.top(); pq.pop();
            if (it.dist > bestDist2)
            {
                break;
            }

            const Node& node = nodes[it.nodeIdx];

            if (node.leaf)
            {
                for (size_t i = node.start; i < node.end; ++i)
                {
                    unsigned int pIdx = indices[i];
                    const glm::vec3& p = (*points)[pIdx];
                    float dx = p.x - target.x, dy = p.y - target.y, dz = p.z - target.z;
                    float d2 = dx * dx + dy * dy + dz * dz;
                    if (d2 < bestDist2)
                    {
                        bestDist2 = d2;
                        bestIndex = static_cast<int>(pIdx);
                    }
                }
            }
            else
            {
                for (int c = 0; c < 8; ++c)
                {
                    int ci = node.children[c];
                    if (ci == -1) { continue; }
                    float d = sqDistAABB(target, nodes[ci].min, nodes[ci].max);
                    if (d <= bestDist2)
                    {
                        pq.push({ d, ci });
                    }
                }
            }
        }

        return bestIndex;
    }
};
#endif // !__MORTON_OCTREE__