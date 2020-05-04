#include <visualizer/DrawableCube.hpp>

namespace Visualizer {

CubeInfo::CubeInfo(CubeInfo* parent, glm::ivec3 order, glm::ivec3 limits)
    : m_limits{ limits }
    , m_currentIter{ 1, 1, 1 }
    , m_order{ order }
    , m_parent{ parent }
{
}

void CubeInfo::tick()
{
    if (m_currentIter[m_order[0]] < m_limits[m_order[0]]) {
        m_currentIter[m_order[0]]++;
    } else {
        m_currentIter[m_order[0]] = 0;
        if (m_currentIter[m_order[1]] < m_limits[m_order[1]]) {
            m_currentIter[m_order[1]]++;
        } else {
            m_currentIter[m_order[1]] = 0;
            if (m_currentIter[m_order[2]] < m_limits[m_order[2]]) {
                m_currentIter[m_order[2]]++;
            } else {
                m_currentIter[m_order[2]] = 0;
                m_parent->tick();
            }
        }
    }
}

glm::ivec3 CubeInfo::currentIter() { return m_currentIter; }

DrawableCube::DrawableCube(
    Transformation transformation, CubeInfo cubeInfo, std::shared_ptr<const Mesh>&& mesh)
    : m_cubeInfo{ cubeInfo }
    , m_transformation{ transformation }
    , m_mesh{ std::move(mesh) }
{
}

void DrawableCube::tick()
{
    m_cubeInfo.tick();
    // TODO: Tick Transformation
}

}