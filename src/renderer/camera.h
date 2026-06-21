// =============================================================================
// 文件: renderer/camera.h
// 作用: Camera：3D 相机。根据 yaw/pitch/distance 生成 view 矩阵和 proj 矩阵。orbit/pan/zoom 响应鼠标输入
// =============================================================================

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace viewer {

class Camera {
public:
    Camera();

    glm::mat4 view_matrix() const;
    glm::mat4 projection_matrix(float aspect_ratio) const;

    void orbit(float dx, float dy);
    void pan(float dx, float dy);
    void zoom(float delta);
    void reset();

    void set_target(glm::vec3 t) { target_ = t; }
    void set_distance(float d) { distance_ = d; }

private:
    glm::vec3 target_{0.0f};
    float distance_ = 10.0f;
    float yaw_ = 0.0f;
    float pitch_ = 30.0f;
    float fov_ = 45.0f;
    float near_ = 0.01f;
    float far_ = 1000.0f;
};

} // namespace viewer
