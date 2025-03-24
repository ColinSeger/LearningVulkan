#pragma once

#include <memory>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>

#include "VulkanTest/Render/Model/vulkanModel.h"

namespace lve {

	struct TransformComponent {
		glm::vec3 translation {};
		glm::vec3 scale {1.f, 1.f, 1.f};
		glm::vec3 rotation {};

        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		glm::mat4 mat4();

		glm::mat3 NormalMatrix();
	};

	class GameObject {
		using id_t = unsigned int;
		unsigned int id;


		GameObject(id_t objId) : id { objId }{}

	public:
		using Map = std::unordered_map<id_t, GameObject>;


		std::shared_ptr<VulkanModel> model{};
		glm::vec3 color{};
		TransformComponent transform{};

		GameObject(const GameObject&) = delete;
		GameObject &operator=(const GameObject&) = delete;
		GameObject(GameObject&&) = default;
		GameObject &operator=(GameObject&&) = default;	

		static GameObject CreateGameObject() {
			static id_t currentId = 0;
			return GameObject{ currentId++ };
		}

		id_t GetId() { return id; };
	};
}