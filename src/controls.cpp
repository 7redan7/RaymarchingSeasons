#include "controls.hpp"
#include <glm/gtc/constants.hpp>

controls::controls(GLFWwindow *window)
	: window(window), contr_struct{
		glm::vec3(0, 5, 5),
		glm::pi<float>(),
		0.0f,
		45.0f,
		8.0f,
		0.005f
	}{}

glm::mat4 controls::getViewMatrix() const
{
	return ViewMatrix;
}

glm::mat4 controls::getProjectionMatrix() const
{
	return ProjectionMatrix;
}

glm::vec3 controls::getPosition() const
{
	return contr_struct.position;
}

glm::vec2 controls::getHorizontalDirection() const
{
	return glm::vec2(sin(contr_struct.horizontalAngle), cos(contr_struct.horizontalAngle));
}

void controls::computeMatricesFromInputs()
{
	static double lastTime = glfwGetTime();
	static bool wasPressed = false;

	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);
	int windowWidth;
	int windowHeight;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);
	double centerX = windowWidth * 0.5;
	double centerY = windowHeight * 0.5;

	bool isPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

	if (isPressed)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		// Get mouse position
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		glfwSetCursorPos(window, centerX, centerY);

		if(wasPressed){
			contr_struct.horizontalAngle += contr_struct.mouseSpeed * float(centerX - xpos);
			contr_struct.verticalAngle   += contr_struct.mouseSpeed * float(centerY - ypos);
		}
	}
	else
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	wasPressed = isPressed;
	glm::vec3 direction(
		cos(contr_struct.verticalAngle) * sin(contr_struct.horizontalAngle), 
		sin(contr_struct.verticalAngle),
		cos(contr_struct.verticalAngle) * cos(contr_struct.horizontalAngle)
	);
	
	// Right vector
	glm::vec3 right = glm::vec3(
		sin(contr_struct.horizontalAngle - glm::half_pi<float>()), 
		0,
		cos(contr_struct.horizontalAngle - glm::half_pi<float>())
	);
	
	// Up vector
	glm::vec3 up = glm::cross( right, direction );

	// Move forward
	if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
		contr_struct.position += direction * deltaTime * contr_struct.speed;
	}
	// Move backward
	if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
		contr_struct.position -= direction * deltaTime * contr_struct.speed;
	}
	// Strafe right
	if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
		contr_struct.position += right * deltaTime * contr_struct.speed;
	}
	// Strafe left
	if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
		contr_struct.position -= right * deltaTime * contr_struct.speed;
	}

	float FoV = contr_struct.FoV;
	ProjectionMatrix = glm::perspective(glm::radians(FoV), float(windowWidth > 0 ? windowWidth : 1) / float(windowHeight > 0 ? windowHeight : 1), 0.1f, 150.0f);
	// Camera matrix
	ViewMatrix = glm::lookAt(
						contr_struct.position,
						contr_struct.position+direction,
						up
						);

	lastTime = currentTime;
}
