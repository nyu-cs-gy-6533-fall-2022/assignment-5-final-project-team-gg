# assignment-5-final-project-team-gg
assignment-5-final-project-team-gg created by GitHub Classroom

## TASK 1: free move camera(4%)-Charles
### Key callback
```cpp
  case GLFW_KEY_UP:
      cameraPos -= cameraDirection * 0.25f;
      break;
  case GLFW_KEY_DOWN:
      cameraPos += cameraDirection * 0.25f;
      break;
  case GLFW_KEY_LEFT:
      cameraPos -= glm::normalize(glm::cross(cameraDirection, cameraUp)) * 0.25f;
      break;
  case GLFW_KEY_RIGHT:
      cameraPos += glm::normalize(glm::cross(cameraDirection, cameraUp)) * 0.25f;
      break;
```
### Mouse callback(right click to switch control(key/mouse))
```cpp
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        mode = (mode == 0) ? 1 : 0;
    }
```

### Cursor Position
```cpp
static void cursor_position_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    float sensitivity = 0.5f; 
    glm::vec3 front;

    if (firstMouse){
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
    float xz = cos(glm::radians(pitch));
    front.x = cos(glm::radians(yaw)) * xz;
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * xz;
    cameraDirection = glm::normalize(front);
}
```

## TASK 2: Objects(12%)-Charles


## TASK 3: Lights(16%)-Charles


## TASK 4: Ray Tracing(16%)-Nick


## TASK 5: Spatial Data Structure(20%)-Nick


## TASK 6: Screen-space Methods (12 %)-Patrick


## Task 7: Particle System (20 %)-Patrick
