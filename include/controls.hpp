#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/* 
I comandi in input di movimento necessitano di ricalcolare la View e la Projection matrix.
La projection in realtà è sempre uguale perché la rotella non funziona, probabilmente converrà
dividere in due moduli diversi.
Approfondire cosa è necessario 
*/


struct controlsStruct
{
	glm::vec3 position;
	float horizontalAngle;
	float verticalAngle;
	float FoV;
	float speed;
	float mouseSpeed;
};

class controls
{
private:
	GLFWwindow *window;

	controlsStruct contr_struct;

	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;

public:
	controls(GLFWwindow *window );
	glm::mat4 getViewMatrix() const;
	glm::mat4 getProjectionMatrix() const;
	glm::vec3 getPosition() const;
	glm::vec2 getHorizontalDirection() const;
	void computeMatricesFromInputs();
};

