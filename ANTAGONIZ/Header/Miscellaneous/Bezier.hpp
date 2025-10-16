#include <glm/glm.hpp>
#include <vector>
#include <iostream>

class BezierCurve
{
public:
    BezierCurve(const std::vector<glm::vec3>& controlPoints) : controlPoints(controlPoints) {}

    glm::vec3 evaluate(float t) const
    {
        return deCasteljau(controlPoints, t);
    }

    std::vector<glm::vec3>& getControlPoints() 
    {
        return controlPoints;
    }

private:
    std::vector<glm::vec3> controlPoints;

    glm::vec3 deCasteljau(const std::vector<glm::vec3>& points, float t) const
    {
        if (points.size() == 1)
        {
            return points[0];
        }

        std::vector<glm::vec3> newPoints;
        for (size_t i = 0; i < points.size() - 1; ++i)
        {
            newPoints.push_back((1 - t) * points[i] + t * points[i + 1]);
        }
        return deCasteljau(newPoints, t);
    }
};

struct BMod
{        
public:
    BMod(glm::vec3* d, glm::vec3 off) { data = d; offset = off; }
    glm::vec3* data;
    glm::vec3 offset;
    glm::vec3 getValue()
    {
        return data == nullptr ? offset : (*data)+ offset;
    }
};

class BezierCurveModifier
{
public:
    BezierCurveModifier(std::vector<BMod> controlPoints) : controlPoints(controlPoints) {}

    glm::vec3 evaluate(float t) const 
    {
        return deCasteljau(controlPoints, t);
    }
private:
    std::vector<BMod> controlPoints;

    glm::vec3 deCasteljau(std::vector<BMod> points, float t) const
    {
        if (points.size() == 1) 
        {
            return points[0].getValue();
        }

        std::vector<BMod> newPoints;
        for (size_t i = 0; i < points.size() - 1; ++i) 
        {
            glm::vec3 interpolated = (1 - t) * points[i].getValue() + t * points[i + 1].getValue();        
            newPoints.push_back(BMod{nullptr, interpolated });
        }
        return deCasteljau(newPoints, t);
    }
};
