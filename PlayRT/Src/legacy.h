#pragma once
for (const auto& light : scene.lights) {
    vec3 lightDir = glm::normalize(light.position - intersection->position);
    Ray shadowRay{ intersection->position + normal * 0.001f, lightDir };

    auto shadowIntersection = ray_cast(shadowRay, scene.bvh->nodes, scene.triangles);
    if (shadowIntersection && shadowIntersection->t < glm::length(light.position - intersection->position)) {
        continue;
    }

    color += light.intensity * std::max(0.0f, glm::dot(normal, lightDir));
}

template<Intersectable Primitive>
std::optional<Intersection> ray_cast(const Ray & ray, const std::vector<BVHNode>&nodes, const std::vector<Primitive>&primitives) {
   
}
