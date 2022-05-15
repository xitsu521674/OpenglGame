#include <glm/glm.hpp>
#include <vector>
#include "RenderUtilities/Shader.h"
#include <glm/gtc/matrix_transform.hpp>

class Particle {
public:
	glm::vec3 direciton;
	glm::vec3 position;
	glm::vec2 offset;
	float life;

};
class ParticleSystem {
public:
	ParticleSystem(Shader* shader) {
		this->shader = shader;
		init();
	}
	Shader* shader;
	int maxNumber = 5000;
	unsigned int VAO, VBO;
	std::vector<Particle> particles;
	int lastUnused = 0;
	void init() {
		float particle_quad[] = {
			// positions          
			-0.5f,  0.5f, 0.0f, 0.0f, 0.0f,
			-0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 1.0f,

			-0.5f,  0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 1.0f,


		};
		/*float particle_quad[] = {
				-1.0f,  1.0f, -1.0f,
				-1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,

				-1.0f, -1.0f,  1.0f,
				-1.0f, -1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f,  1.0f,
				-1.0f, -1.0f,  1.0f,

				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,

				-1.0f, -1.0f,  1.0f,
				-1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f, -1.0f,  1.0f,
				-1.0f, -1.0f,  1.0f,

				-1.0f,  1.0f, -1.0f,
				 1.0f,  1.0f, -1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				-1.0f,  1.0f,  1.0f,
				-1.0f,  1.0f, -1.0f,

				-1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f,  1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f,  1.0f,
				 1.0f, -1.0f,  1.0f
		};*/
		glGenVertexArrays(1, &this->VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(this->VAO);
		// fill mesh buffer
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
		// set mesh attributes
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glBindVertexArray(0);
		for (int i = 0; i < maxNumber; ++i) {
			Particle p;
			p.life = 0;
			particles.push_back(p);
		}
	}
	void spawn(glm::vec3 position,glm::vec3 direction,glm::vec2 offset) {
		int id = -1;
		for (int i = 0; i < maxNumber; ++i) {
			if (particles[(lastUnused + i) % maxNumber].life == 0) {
				id = (lastUnused + i) % maxNumber;
				break;
			}
		}
		if (id == -1)
			return;
		lastUnused = id;
		particles[id].life = 20;
		particles[id].position = position;
		particles[id].direciton = direction;
		particles[id].offset = offset;
	}
	void update() {
		for (Particle& p : particles) {
			if (p.life != 0) {
				p.life--;
				p.position += p.direciton;
			}
		}
	}
	void draw(glm::mat4 projection,glm::mat4 view,float yaw,float pitch,unsigned int texture) {
		shader->Use();
		glUniformMatrix4fv(glGetUniformLocation(shader->Program, "projection"), 1,false, &projection[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shader->Program, "view"), 1,false, &view[0][0]);
		glUniform1i(glGetUniformLocation(shader->Program, "texture1"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		for (Particle particle : particles)
		{
			if (particle.life > 0.0f)
			{
				glm::mat4 model(1.0f);
				model = glm::translate(model, particle.position);
				model = glm::rotate(model, glm::radians(-yaw-90), glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, glm::radians(-(180-pitch)), glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(100, 100, 100));
				
				glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model"), 1,false, &model[0][0]);
				glUniform2fv(glGetUniformLocation(shader->Program, "offset"), 1, &particle.offset[0]);
				glBindVertexArray(this->VAO);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glBindVertexArray(0);
			}
		}
		// don't forget to reset to default blending mode
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
};