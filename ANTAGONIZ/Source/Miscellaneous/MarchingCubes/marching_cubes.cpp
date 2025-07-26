#include "marching_cubes.hpp"

std::vector<glm::vec3> MarchingCubes::triangulate_field(float * scalarFunction, unsigned int odSize, float isovalue)
{
    unsigned int max = odSize;
    unsigned int max2 = max * max;
    unsigned int i, j, k,l;
    float x, y, z,mu;
    int cubeIndex, intersectionsKey, idx, v1,v2;
    glm::vec3 tempPos[8];
    float tempVal[8];
    glm::vec3 intersections[12];
    std::vector<glm::vec3> triangles;
    triangles.reserve(max* max* max*3);
    
    for (i = 0; i + 1 < max; i++)
    {
        for (j = 0; j + 1 < max; j++)
        {
            for (k = 0; k + 1 < max; k++)
            {
                x = i;
                y = j;
                z = k;

                tempPos[0] = glm::vec3(x, y, z);
                tempPos[1] = glm::vec3(x + 1.0f, y, z);
                tempPos[2] = glm::vec3(x + 1.0f, y, z + 1.0f);
                tempPos[3] = glm::vec3(x, y, z + 1.0f);
                tempPos[4] = glm::vec3(x, y + 1.0f, z);
                tempPos[5] = glm::vec3(x + 1.0f, y + 1.0f, z);
                tempPos[6] = glm::vec3(x + 1.0f, y + 1.0f, z + 1.0f);
                tempPos[7] = glm::vec3(x, y + 1.0f, z + 1.0f);

                tempVal[0] = scalarFunction[i + j * max + k * max2];
                tempVal[1] = scalarFunction[(i + 1) + j * max + k * max2];
                tempVal[2] = scalarFunction[(i + 1) + j * max + (k + 1) * max2];
                tempVal[3] = scalarFunction[i + j * max + (k + 1) * max2];
                tempVal[4] = scalarFunction[i + (j + 1) * max + k * max2];
                tempVal[5] = scalarFunction[(i + 1) + (j + 1) * max + k * max2];
                tempVal[6] = scalarFunction[(i + 1) + (j + 1) * max + (k + 1) * max2];
                tempVal[7] = scalarFunction[i + (j + 1) * max + (k + 1) * max2];

                cubeIndex = ((tempVal[0] < isovalue) << 0) | ((tempVal[1] < isovalue) << 1) | ((tempVal[2] < isovalue) << 2) 
                          | ((tempVal[3] < isovalue) << 3) | ((tempVal[4] < isovalue) << 4) | ((tempVal[5] < isovalue) << 5) 
                          | ((tempVal[6] < isovalue) << 6) | ((tempVal[7] < isovalue) << 7);

                intersectionsKey = edgeTable[cubeIndex];
                idx = 0;
                while (intersectionsKey)
                {
                    if (intersectionsKey & 1)
                    {
                        v1 = edgeToVertices[idx].first;
                        v2 = edgeToVertices[idx].second;
                        mu = (isovalue - tempVal[v1]) / (tempVal[v2] - tempVal[v1]);
                        intersections[idx].x = mu * (tempPos[v2].x - tempPos[v1].x) + tempPos[v1].x;
                        intersections[idx].y = mu * (tempPos[v2].y - tempPos[v1].y) + tempPos[v1].y;
                        intersections[idx].z = mu * (tempPos[v2].z - tempPos[v1].z) + tempPos[v1].z;
                    }
                    idx++;
                    intersectionsKey >>= 1;
                }

                for (l = 0; triangleTable[cubeIndex][l] != -1; l += 3)
                {                                        
                    triangles.push_back(intersections[triangleTable[cubeIndex][l + 0]]);
                    triangles.push_back(intersections[triangleTable[cubeIndex][l + 1]]);
                    triangles.push_back(intersections[triangleTable[cubeIndex][l + 2]]);
                }
            }
        }
    }
    triangles.shrink_to_fit();
    return triangles;
}

void MarchingCubes::optimized_triangulate_field(float* scalarFunction, unsigned int odSize, float isovalue, ChunkArray<VertexBiome> * vertexPool, ChunkArray<unsigned int> * indexPool, FixedHashTable64* fht)
{
    unsigned int max = odSize;
    unsigned int max2 = max * max;
    unsigned int i, j, k, l;
    float x, y, z, mu;
    int cubeIndex, intersectionsKey, idx, v1, v2;
    glm::vec3 tempPos[8];
    float tempVal[8];
    glm::vec3 intersections[12];
    float divider = (1.0f / (float)max) * 65535.0f;
    VertexBiome* cv = nullptr;    
    unsigned short px, py, pz;
    uint64_t packed = 0ULL;
    uint32_t targetVertex;
    
    for (i = 0; i + 1 < max; ++i)
    {
        for (j = 0; j + 1 < max; ++j)
        {
            for (k = 0; k + 1 < max; ++k)
            {
                x = i;
                y = j;
                z = k;

                tempPos[0] = glm::vec3(x, y, z);
                tempPos[1] = glm::vec3(x + 1.0f, y, z);
                tempPos[2] = glm::vec3(x + 1.0f, y, z + 1.0f);
                tempPos[3] = glm::vec3(x, y, z + 1.0f);
                tempPos[4] = glm::vec3(x, y + 1.0f, z);
                tempPos[5] = glm::vec3(x + 1.0f, y + 1.0f, z);
                tempPos[6] = glm::vec3(x + 1.0f, y + 1.0f, z + 1.0f);
                tempPos[7] = glm::vec3(x, y + 1.0f, z + 1.0f);

                tempVal[0] = scalarFunction[i + j * max + k * max2];
                tempVal[1] = scalarFunction[(i + 1) + j * max + k * max2];
                tempVal[2] = scalarFunction[(i + 1) + j * max + (k + 1) * max2];
                tempVal[3] = scalarFunction[i + j * max + (k + 1) * max2];
                tempVal[4] = scalarFunction[i + (j + 1) * max + k * max2];
                tempVal[5] = scalarFunction[(i + 1) + (j + 1) * max + k * max2];
                tempVal[6] = scalarFunction[(i + 1) + (j + 1) * max + (k + 1) * max2];
                tempVal[7] = scalarFunction[i + (j + 1) * max + (k + 1) * max2];

                cubeIndex = ((tempVal[0] < isovalue) << 0) | ((tempVal[1] < isovalue) << 1) | ((tempVal[2] < isovalue) << 2)
                    | ((tempVal[3] < isovalue) << 3) | ((tempVal[4] < isovalue) << 4) | ((tempVal[5] < isovalue) << 5)
                    | ((tempVal[6] < isovalue) << 6) | ((tempVal[7] < isovalue) << 7);

                intersectionsKey = edgeTable[cubeIndex];
                idx = 0;
                while (intersectionsKey)
                {
                    if (intersectionsKey & 1)
                    {
                        v1 = edgeToVertices[idx].first;
                        v2 = edgeToVertices[idx].second;
                        mu = (isovalue - tempVal[v1]) / (tempVal[v2] - tempVal[v1]);
                        intersections[idx].x = mu * (tempPos[v2].x - tempPos[v1].x) + tempPos[v1].x;
                        intersections[idx].y = mu * (tempPos[v2].y - tempPos[v1].y) + tempPos[v1].y;
                        intersections[idx].z = mu * (tempPos[v2].z - tempPos[v1].z) + tempPos[v1].z;
                    }
                    idx++;
                    intersectionsKey >>= 1;
                }

                for (l = 0; triangleTable[cubeIndex][l] != -1; l += 3)
                {      
#pragma unroll
                    for (int n = 0; n < 3; ++n)
                    {
                        glm::vec3 vp = intersections[triangleTable[cubeIndex][l + n]];
                        px = static_cast<unsigned short>(vp.x * divider);
                        py = static_cast<unsigned short>(vp.y * divider);
                        pz = static_cast<unsigned short>(vp.z * divider);
                        packed = (uint64_t(px) << 32) | (uint64_t(py) << 16) | uint64_t(pz);

                        if (fht->insert_with_index(packed, &targetVertex))
                        {
                            cv = vertexPool->push_get_ptr();
                            cv->x = px;
                            cv->y = py;
                            cv->z = pz;
                        }

                        *(indexPool->push_get_ptr()) = targetVertex;
                    }
                }
            }
        }
    }
}

void MarchingCubes::optimized_triangulate_field_lod(float* scalarFunction, unsigned int odSize, float isovalue, ChunkArray<unsigned int>* indexPool, FixedHashTable64* fht, unsigned int skip)
{
    unsigned int max = odSize;
    unsigned int max2 = max * max;
    unsigned int i, j, k, l;
    float x, y, z, mu;
    int cubeIndex, intersectionsKey, idx, v1, v2;
    glm::vec3 tempPos[8];
    float tempVal[8];
    glm::vec3 intersections[12];
    float divider = (1.0f / (float)max) * 65535.0f;
    VertexBiome* cv = nullptr;
    unsigned short px, py, pz;
    uint64_t packed = 0ULL;
    uint32_t targetVertex;

    for (i = 0; i + skip < max; i += skip)
    {
        for (j = 0; j + skip < max; j += skip)
        {
            for (k = 0; k + skip < max; k += skip)
            {
                x = i;
                y = j;
                z = k;

                tempPos[0] = glm::vec3(x, y, z);
                tempPos[1] = glm::vec3(x + 1.0f, y, z);
                tempPos[2] = glm::vec3(x + 1.0f, y, z + 1.0f);
                tempPos[3] = glm::vec3(x, y, z + 1.0f);
                tempPos[4] = glm::vec3(x, y + 1.0f, z);
                tempPos[5] = glm::vec3(x + 1.0f, y + 1.0f, z);
                tempPos[6] = glm::vec3(x + 1.0f, y + 1.0f, z + 1.0f);
                tempPos[7] = glm::vec3(x, y + 1.0f, z + 1.0f);

                tempVal[0] = scalarFunction[i + j * max + k * max2];
                tempVal[1] = scalarFunction[(i + 1) + j * max + k * max2];
                tempVal[2] = scalarFunction[(i + 1) + j * max + (k + 1) * max2];
                tempVal[3] = scalarFunction[i + j * max + (k + 1) * max2];
                tempVal[4] = scalarFunction[i + (j + 1) * max + k * max2];
                tempVal[5] = scalarFunction[(i + 1) + (j + 1) * max + k * max2];
                tempVal[6] = scalarFunction[(i + 1) + (j + 1) * max + (k + 1) * max2];
                tempVal[7] = scalarFunction[i + (j + 1) * max + (k + 1) * max2];

                cubeIndex = ((tempVal[0] < isovalue) << 0) | ((tempVal[1] < isovalue) << 1) | ((tempVal[2] < isovalue) << 2)
                    | ((tempVal[3] < isovalue) << 3) | ((tempVal[4] < isovalue) << 4) | ((tempVal[5] < isovalue) << 5)
                    | ((tempVal[6] < isovalue) << 6) | ((tempVal[7] < isovalue) << 7);

                intersectionsKey = edgeTable[cubeIndex];
                idx = 0;
                while (intersectionsKey)
                {
                    if (intersectionsKey & 1)
                    {
                        v1 = edgeToVertices[idx].first;
                        v2 = edgeToVertices[idx].second;
                        mu = (isovalue - tempVal[v1]) / (tempVal[v2] - tempVal[v1]);
                        intersections[idx].x = mu * (tempPos[v2].x - tempPos[v1].x) + tempPos[v1].x;
                        intersections[idx].y = mu * (tempPos[v2].y - tempPos[v1].y) + tempPos[v1].y;
                        intersections[idx].z = mu * (tempPos[v2].z - tempPos[v1].z) + tempPos[v1].z;
                    }
                    idx++;
                    intersectionsKey >>= 1;
                }

                for (l = 0; triangleTable[cubeIndex][l] != -1; l += 3)
                {
#pragma unroll
                    for (int n = 0; n < 3; ++n)
                    {
                        glm::vec3 vp = intersections[triangleTable[cubeIndex][l + n]];
                        px = static_cast<unsigned short>(vp.x * divider);
                        py = static_cast<unsigned short>(vp.y * divider);
                        pz = static_cast<unsigned short>(vp.z * divider);
                        packed = (uint64_t(px) << 32) | (uint64_t(py) << 16) | uint64_t(pz);

                        if (!fht->insert_with_index(packed, &targetVertex))
                        {
                            *(indexPool->push_get_ptr()) = targetVertex;
                        }                        
                    }
                }
            }
        }
    }
}