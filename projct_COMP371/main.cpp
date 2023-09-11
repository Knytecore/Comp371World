// Draw and get windows
#include <gl/glew.h>
#include <GLFW/glfw3.h>

// To do math
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Include files
#include "shader.h"
#include "camera.h"
#include "chunk.h"
#include "world.h"
#include "Model.h"
#include "skybox.h"

// This determine the size of chunks (width and height)
// as well as the view distance in any direction (in chunks)
#define CHUNKSIZE 64
#define VIEWDISTANCE 2

// For i/o and generating the seed.
#include <iostream>
#include <chrono>

// Callback functions. These handle various forms of input such as
// resizing, clicking, scrolling, pressing and holding keys, etc.
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// screen/window width and height
unsigned int sWIDTH = 1280;
unsigned int sHEIGHT = 720;

// Player camera
Camera camera(glm::vec3(CHUNKSIZE/2.0f, 50.0f, CHUNKSIZE/2.0f));

// Current world
world *currentWorld;

// Last mouse position
float lastX = (float)sWIDTH / 2.0;
float lastY = (float)sHEIGHT / 2.0;
bool firstMouse = true;

// Delta time and previous measured time
float dT = 0.0f;
float T0 = 0.0f;

// Points to the current skybox to use.
Skybox* currentSkybox;

// Initial values for flags. These should be toggleable via keyboard shortcuts.
bool bFlashlight = true;
bool bNoclip = false;
bool bShadow = true;
bool bMaterial = true;
bool bDaytime = true;
bool bCursorVisible = true;

// Suffering
int main() {
    // This simply gets a seed (unix time in ms).
    using namespace std::chrono;
    const uint64_t EPOCH = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    std::cout << "SEED: ";
    std::cout << EPOCH << std::endl;

    // Generate the simple noise mapping.
    OpenSimplexNoise::Noise heightNoise = OpenSimplexNoise::Noise(EPOCH);
    OpenSimplexNoise::Noise forestNoise = OpenSimplexNoise::Noise(3*EPOCH);
    
    // Opengl version
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create the window
    GLFWwindow* window = glfwCreateWindow(sWIDTH, sHEIGHT, "COMP371: Team#7 - Project", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    // Attach callbacks to windows.
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Initialize GLEW. This is needed to init the functions.
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    
    // Load the various shaders to be used in the program
    Shader shader("assets/shaders/main.vs", "assets/shaders/main.fs");
    Shader depthShader("assets/shaders/shadowdepth.vs", "assets/shaders/shadowdepth.fs");
    Shader skyShader("assets/shaders/skybox.vs", "assets/shaders/skybox.fs");
    
    // Set up shader parameters.
    shader.setInt("diffuseTexture", 0);
    shader.setInt("shadowMap", 1);
    
    // Create the skybox to be used for day time
    Skybox daybox = Skybox("assets/textures/skyboxes/d_left.bmp",
                           "assets/textures/skyboxes/d_right.bmp",
                           "assets/textures/skyboxes/d_back.bmp",
                           "assets/textures/skyboxes/d_front.bmp",
                           "assets/textures/skyboxes/d_top.bmp",
                           "assets/textures/skyboxes/d_bottom.bmp");
    
    // Create the skybox to be used for night time
    Skybox nightbox = Skybox("assets/textures/skyboxes/space/space_left.png",
                             "assets/textures/skyboxes/space/space_right.png",
                             "assets/textures/skyboxes/space/space_back.png",
                             "assets/textures/skyboxes/space/space_front.png",
                             "assets/textures/skyboxes/space/space_top.png",
                             "assets/textures/skyboxes/space/space_bottom.png");
    
    // Create the texture used for the ground
    unsigned int floorTexture = loadTexture("assets/textures/surfaces/dirt.png");

    // Prepare shadow map. High res for nice results
    const unsigned int SHADOW_WIDTH = 8192, SHADOW_HEIGHT = 8192;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    shader.use();
    shader.setInt("diffuseTexture", 0);
    shader.setInt("shadowMap", 1);

    // Initial light position for shadows
    glm::vec3 lightPos(-2.0f, 50.0f, -1.0f);
    
    // Make the world, make it current.
    world theWorld = world(0, 0, CHUNKSIZE, CHUNKSIZE, VIEWDISTANCE, &heightNoise);
    currentWorld = &theWorld;

    // Enter the main loop
    while (!glfwWindowShouldClose(window))
    {
        // Camera should always lie on top of the ground. Not under, not in the air
        if (!bNoclip)
            camera.setHeight(theWorld.interpolateHeight(camera.Position.x, camera.Position.z));
        shader.use();
        
        // make sure the flags match the behavior. It is preferrable to do this in main loop
        // and keep the flags global. Allows changing states easily
        if (bShadow)
            shader.setBool("sToggle", true);
        else
            shader.setBool("sToggle", false);
        
        if (bFlashlight)
            shader.setBool("lToggle", true);
        else
            shader.setBool("lToggle", false);
        
        if (bMaterial)
            shader.setBool("matToggle", true);
        else
            shader.setBool("matToggle", false);
        
        if (bDaytime) {
            shader.setBool("dayToggle", true);
            currentSkybox = & daybox;
        }
        else {
            shader.setBool("dayToggle", false);
            currentSkybox = & nightbox;
        }
        
        if (bCursorVisible)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        
        // Flashlight
        shader.setVec3("spotLight.position", camera.Position);
        shader.setVec3("spotLight.direction", camera.Front);
        shader.setVec3("spotLight.ambient", 3.0, 2.7f, 1.8f);
        shader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        shader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        shader.setFloat("spotLight.constant", 1.0f);
        shader.setFloat("spotLight.linear", 0.09f);
        shader.setFloat("spotLight.quadratic", 0.032f);
        shader.setFloat("spotLight.cutOff", glm::cos(glm::radians(5.5f)));
        shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(20.0f)));
        
        // Loop around, like super mario
        if (camera.Position.x <  0.0f) {
            camera.Position.x += CHUNKSIZE;
            theWorld.updatePos(WEST);
        }
        if (camera.Position.z <  0.0f) {
            camera.Position.z += CHUNKSIZE;
            theWorld.updatePos(SOUTH);
        }
        if (camera.Position.x >  CHUNKSIZE) {
            camera.Position.x -= CHUNKSIZE;
            theWorld.updatePos(EAST);
        }
        if (camera.Position.z >  CHUNKSIZE) {
            camera.Position.z -= CHUNKSIZE;
            theWorld.updatePos(NORTH);
        }
        
        //mUpdate time values
        float T1 = static_cast<float>(glfwGetTime());
        dT = T1 - T0;
        T0 = T1;
        
        // Process inputs
        processInput(window);

        // Directional light positioning (angle)
        lightPos.x = camera.Position.x;
        lightPos.z = camera.Position.z;
        lightPos.y = camera.Position.y+200.0f;
        
        // EVERYTHING BLACK
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Matrices for the shadows. Neede to cast shadows on a large distance, hence the values.
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = -100.0f, far_plane = 400.5f;
        lightProjection = glm::ortho(-400.0f, 400.0f, -400.0f, 400.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;
        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        // ~~~~~~~~~~~~~~~~~~~~~~~
        // Render the shadows to the buffer (depth map)
        depthShader.use();
        depthShader.setMat4("model", glm::mat4(1.0f));
        theWorld.renderChunks(depthShader, 0);
        // ~~~~~~~~~~~~~~~~~~~~~~~
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // viewport is window size
        glViewport(0, 0, sWIDTH, sHEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Need to display ALL
        shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)sWIDTH / (float)sHEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setVec3("viewPos", camera.Position);
        shader.setVec3("lightPos", lightPos);
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        // ----------------------------------------
        // Draw the world and the shadows
        shader.use();
        shader.setMat4("model", glm::mat4(1.0f));
        theWorld.renderChunks(shader, 1);
        // ----------------------------------------
        currentSkybox->render(&camera, &skyShader, sWIDTH, sHEIGHT);
        
        renderCube();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window) {
    // Quit if user presses exit buttons
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // Movement. Only move is the predicted position is valid. Else, do nothing.
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        if (bNoclip)
            camera.ProcessKeyboard(FORWARD, dT);
        else if (currentWorld->isValid(camera.nextStep(FORWARD, dT)))
            camera.ProcessKeyboard(FORWARD, dT);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        if (bNoclip)
            camera.ProcessKeyboard(BACKWARD, dT);
        else if (currentWorld->isValid(camera.nextStep(BACKWARD, dT)))
        camera.ProcessKeyboard(BACKWARD, dT);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        if (bNoclip)
            camera.ProcessKeyboard(LEFT, dT);
        else if (currentWorld->isValid(camera.nextStep(LEFT, dT)))
        camera.ProcessKeyboard(LEFT, dT);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        if (bNoclip)
            camera.ProcessKeyboard(RIGHT, dT);
        else if (currentWorld->isValid(camera.nextStep(RIGHT, dT)))
        camera.ProcessKeyboard(RIGHT, dT);
}

// Update window size. Match viewport size to it.
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    sWIDTH = width; sHEIGHT = height;
    glViewport(0, 0, width, height);
}

// Not much to say here. Mouse handling.
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// Mouse scroll. Used for zooming in and out (fov changes)
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// Key buttons. On keypress mostly, not hold.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        bCursorVisible = bCursorVisible ? false : true;
    }
    
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        bShadow = bShadow ? false : true;
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        bDaytime = bDaytime ? false : true;
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        bFlashlight = bFlashlight ? false : true;
    }
    if (key == GLFW_KEY_L && action == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        bMaterial = bMaterial ? false : true;
    
    if (key == GLFW_KEY_N && action == GLFW_PRESS)
        bNoclip = bNoclip ? false : true;
}
