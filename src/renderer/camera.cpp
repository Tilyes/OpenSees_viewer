// =============================================================================
// 文件: renderer/camera.cpp
// 作用: Camera 实现：orbit 改 yaw/pitch，zoom 改 distance，view_matrix 用 lookAt 计算，projection 用 perspective
// =============================================================================

#include "camera.h"

namespace viewer {

Camera::Camera() { reset(); }

glm::mat4 Camera::view_matrix() const {
    float rad_yaw = glm::radians(yaw_);
    float rad_pitch = glm::radians(pitch_);

    glm::vec3 eye;
    eye.x = target_.x + distance_ * cos(rad_pitch) * sin(rad_yaw);
    eye.y = target_.y + distance_ * sin(rad_pitch);
    eye.z = target_.z + distance_ * cos(rad_pitch) * cos(rad_yaw);

    return glm::lookAt(eye, target_, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::projection_matrix(float aspect_ratio) const {
    return glm::perspective(glm::radians(fov_), aspect_ratio, near_, far_);
}

void Camera::orbit(float dx, float dy) {
    yaw_ += dx * 0.5f;
    pitch_ += dy * 0.5f;
    pitch_ = glm::clamp(pitch_, -89.0f, 89.0f);
}

void Camera::pan(float dx, float dy) {
    float rad_yaw = glm::radians(yaw_);
    glm::vec3 right = glm::vec3(cos(rad_yaw), 0.0f, -sin(rad_yaw));
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    target_ += right * dx * 0.01f * distance_;
    target_ += up * dy * 0.01f * distance_;
}

void Camera::zoom(float delta) {
    distance_ *= (1.0f - delta * 0.1f);
    distance_ = glm::clamp(distance_, 0.1f, 10000.0f);
}

void Camera::reset() {
    target_ = glm::vec3(0.0f);
    distance_ = 10.0f;
    yaw_ = 0.0f;
    pitch_ = 30.0f;
}

} // namespace viewer
