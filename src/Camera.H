#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <GL/glu.h>
#include <iostream>


// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 5.0f;
const float SENSITIVITY = 0.5f;
const float ZOOM = 45.0f;

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Camera
{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    glm::mat4 projection;
    glm::mat4 view;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    float lastX = -1;
    float lastY = -1;
    bool firstMouse = true;

    bool wHold = false;
    bool aHold = false;
    bool sHold = false;
    bool dHold = false;


    void update() {
        updateCameraVectors();        
        
    }
    void use(float aspect) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(90, aspect, 0.01, 20000);
        projection = glm::perspective<float>(90, aspect, 0.01, 20000);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glm::vec3 central = Position + Front;
        gluLookAt(Position.x, Position.y, Position.z, central.x, central.y, central.z, Up.x, Up.y, Up.z);
        view = glm::lookAt<float>(Position,central,Up);
    }
    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction,bool HOLD)
    {
        switch (direction)
        {
        case FORWARD:
            if (HOLD) {
                wHold = true;
                sHold = false;
            }
            else {
                wHold = false;
            }
            break;
        case BACKWARD:
            if (HOLD) {
                sHold = true;
                wHold = false;
            }
            else {
                sHold = false;
            }
            break;
        case LEFT:
            if (HOLD) {
                aHold = true;
                dHold = false;
            }
            else {
                aHold = false;
            }
            break;
        case RIGHT:
            if (HOLD) {
                dHold = true;
                aHold = false;
            }
            else {
                dHold = false;
            }
            break;
        default:
            break;
        }
        
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        if (Yaw > 180.0f)
            Yaw -= 360;
        if (Yaw < -180.0f)
            Yaw += 360;
        if (Yaw > 360.f || Yaw < -360.f) 
            Yaw = glm::mod(Yaw, 360.f);

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();

        

    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }
    /// <summary>
    /// 
    /// </summary>
    /// <param name="yaw">between 0~360 x axis</param>
    /// <param name="pitch">between 90~-90 y axis</param>
    void setCamera(float yaw, float pitch) {
        this->Yaw = yaw;
        this->Pitch = pitch;
        updateCameraVectors();
    }
private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
#endif