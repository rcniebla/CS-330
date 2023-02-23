#include <iostream>         // cout, cerr
#include <iomanip>
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnOpengl/camera.h> // Camera class

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Niebla Project 1"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;
    float cameraSpeed = 2.5f;
    glm::mat4 projection;
    bool perspective = true;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbo;         // Handle for the vertex buffer object
        GLuint nVertices;    // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gBaseMesh;
    GLMesh gBookMesh;
    GLMesh gBallMesh;
    GLMesh gCandleMesh;
    GLMesh gTopperMesh;
    GLMesh gCableMesh;
    // Texture id
    GLuint gTextureIdGranite;
    glm::vec2 gUVScale(2.5f, 2.5f);
    GLint gTexWrapMode = GL_REPEAT;
    GLuint gTextureIdBook;
    GLuint gTextureIdBall;
    GLuint gTextureIdCandle;
    GLuint gTextureIdTopper;
    GLuint gTextureIdCable;
    // Shader program
    GLuint gProgramId;
    GLuint gLampProgramId;
    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    // Subject position and scale
    glm::vec3 gPosition(0.0f, 0.0f, 0.0f);
    glm::vec3 bookPos(0.0f, 0.0f, 4.0f);
    glm::vec3 ballPos(3.0f, 0.1f, -1.0f);
    glm::vec3 defScale(1.0,1.0,1.0);
    glm::vec3 gScale(2.0f);
    glm::vec3 ballscale(0.25f);
    glm::vec3 candleScale(1.0, 1.0, 1.0);
    glm::vec3 candlePos(-1.0f, 0.1f, -1.0f);
    glm::vec3 topperPos(-1.0f, 1.3f, -1.0f);

    // Cube color
    glm::vec3 gObjectColor(1.f, 0.2f, 0.0f);


    // Light color position and scale
    glm::vec3 gLightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 gLightPosition(7.0f, 3.0f, 7.0f);
    glm::vec3 gLightScale(0.25f);

    glm::vec3 gAmbientColor(0.0f, 0.09f, 0.13f);
    glm::vec3 gAmbientPosition(-5.0f, 2.0f, -5.0f);
    glm::vec3 gambientScale(0.75f);

}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh& mesh);
void UCreateBook(GLMesh& mesh);
void UCreateBall(GLMesh& mesh);
void UCreateCandle(GLMesh& mesh);
void UCreateTopper(GLMesh& mesh);
void UCreateCable(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
    layout(location = 1) in vec3 normal; // VAP position 1 for normals
    layout(location = 2) in vec2 textureCoordinate;

    out vec3 vertexNormal; // For outgoing normals to fragment shader
    out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
    out vec2 vertexTextureCoordinate;

    //Global variables for the transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
        vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)
        vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
        vertexTextureCoordinate = textureCoordinate;
    }
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
    in vec3 vertexNormal; // For incoming normals
    in vec3 vertexFragmentPos; // For incoming fragment position
    in vec2 vertexTextureCoordinate;

    out vec4 fragmentColor;

    // Uniform / Global variables for object color, light color, light position, and camera/view position
    uniform vec3 objectColor;
    uniform vec3 lightColor;
    uniform vec3 lightPos;
    uniform vec3 ambientColor;
    uniform vec3 ambientPos;
    uniform vec3 viewPosition;
    uniform sampler2D uTexture; // Useful when working with multiple textures
    uniform vec2 uvScale;

    void main()
    {
        /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

        //Calculate Ambient lighting*/
        float globalStrength = 0.5f; // Set ambient or global lighting strength
        vec3 global = globalStrength * lightColor; // Generate ambient light color

        float ambientStrength = 1.7f; // Set ambient or global lighting strength
        vec3 ambient = ambientStrength * ambientColor; // Generate ambient light color


        //Calculate Diffuse lighting*/
        vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
        vec3 globalDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
        float impact = max(dot(norm, globalDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
        vec3 diffuse = impact * lightColor; // Generate diffuse light color

        vec3 ambientDirection = normalize(ambientPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
        float ambientImpact = max(dot(norm, ambientDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
        vec3 ambientdiffuse = ambientImpact * ambientColor; // Generate diffuse light color

        //Calculate Specular lighting*/
        float specularIntensity = 0.8f; // Set specular light strength
        float highlightSize = 16.0f; // Set specular highlight size
        vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
        vec3 reflectDir = reflect(-globalDirection, norm);// Calculate reflection vector
        //Calculate specular component
        float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
        vec3 specular = specularIntensity * specularComponent * lightColor;

        // Texture holds the color to be used for all three components
        vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

        // Calculate Phong result
        vec3 phong = (global + ambient + diffuse + specular) * textureColor.xyz;

        fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU.
    }
);

/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

    //Uniform / Global variables for the  transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
    }
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

    void main()
    {
        fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
    }
);

// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}


int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMesh(gBaseMesh); // Calls the function to create the Vertex Buffer Object
    UCreateBook(gBookMesh);
    UCreateBall(gBallMesh);
    UCreateCandle(gCandleMesh);
    UCreateTopper(gTopperMesh);
    UCreateCable(gCableMesh);

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;

    // Load multiple textures
    const char* texFilename = "../Includes/T_granite.png";
    if (!UCreateTexture(texFilename, gTextureIdGranite))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../Includes/T_Book.png";
    if (!UCreateTexture(texFilename, gTextureIdBook))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../Includes/T_Ball.png";
    if (!UCreateTexture(texFilename, gTextureIdBall))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../Includes/T_Candle.png";
    if (!UCreateTexture(texFilename, gTextureIdCandle))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../Includes/T_Topper.png";
    if (!UCreateTexture(texFilename, gTextureIdTopper))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "../Includes/T_Cable.png";
    if (!UCreateTexture(texFilename, gTextureIdCable))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gBaseMesh);
    UDestroyMesh(gBookMesh);
    UDestroyMesh(gBallMesh);
    UDestroyMesh(gCandleMesh);
    UDestroyMesh(gTopperMesh);
    UDestroyMesh(gCableMesh);

    // Release texture
    UDestroyTexture(gTextureIdGranite);
    UDestroyTexture(gTextureIdBook);
    UDestroyTexture(gTextureIdBall);
    UDestroyTexture(gTextureIdCandle);
    UDestroyTexture(gTextureIdTopper);
    UDestroyTexture(gTextureIdCable);

    // Release shader program
    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(gLampProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);
    glfwSetKeyCallback(*window, key_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    //static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraOffset = cameraSpeed * gDeltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, cameraOffset);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, cameraOffset);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, cameraOffset);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, cameraOffset);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.Position = glm::vec3(gCamera.Position.x, gCamera.Position.y + 0.01f, gCamera.Position.z);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.Position = glm::vec3(gCamera.Position.x, gCamera.Position.y - 0.01f, gCamera.Position.z);
}

//toggle perspective mode implemented as above func triggers every frams which caused camera to switch rapidly
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        perspective = !perspective;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (cameraSpeed < 0.1f) {
        cameraSpeed = 0.1f;
    }
    else {
        cameraSpeed += yoffset;                     //setting scroll to controll camera speed
    }
    //gCamera.ProcessMouseScroll(yoffset);          // commenting our scroll to zoom  
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}


// Functioned called to render a frame
void URender()
{
    ////// Lamp orbits around the origin
    //const float angularVelocity = glm::radians(45.0f);

    //    glm::vec4 newPosition = glm::rotate(angularVelocity * gDeltaTime, glm::vec3(1.0f, 2.0f, 0.0f)) * glm::vec4(gLightPosition, 1.0f);
    //    gLightPosition.x = newPosition.x;
    //    gLightPosition.y = newPosition.y;
    //    gLightPosition.z = newPosition.z;
    //
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Activate the cube VAO (used by cube and lamp)
    glBindVertexArray(gBaseMesh.vao);

    // CUBE: draw cube
    //----------------

    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = glm::translate(gPosition) * glm::scale(gScale);

    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    // Switches between perspective and ortho views
    if (perspective) {
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }
    else {
        projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
    }

    // Set the shader to be used
    glUseProgram(gProgramId);

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    GLint ambientColorLoc = glGetUniformLocation(gProgramId, "ambientColor");
    GLint ambientPositionLoc = glGetUniformLocation(gProgramId, "ambientPos");
    GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");


    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(ambientColorLoc, gAmbientColor.r, gAmbientColor.g, gAmbientColor.b);
    glUniform3f(ambientPositionLoc, gAmbientPosition.x, gAmbientPosition.y, gAmbientPosition.z);
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    
    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Activate the VBOs contained within the mesh's VAOd
    glBindVertexArray(gBaseMesh.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdGranite);
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gBaseMesh.nVertices);
    ///////BOOK///////////////////////////////////////////////////////////////////////////////////////////////
    glBindVertexArray(gBookMesh.vao);
    glBindTexture(GL_TEXTURE_2D, gTextureIdBook);
    model = glm::translate(bookPos) * glm::scale(gScale);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gBookMesh.nVertices);
    
    ///////BALL///////////////////////////////////////////////////////////////////////////////////////////////
    glBindVertexArray(gBallMesh.vao);
    glBindTexture(GL_TEXTURE_2D, gTextureIdBall);
    model = glm::translate(ballPos) * glm::scale(ballscale);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gBallMesh.nVertices);
    ///////CANDLE///////////////////////////////////////////////////////////////////////////////////////////////
    glBindVertexArray(gCandleMesh.vao);
    glBindTexture(GL_TEXTURE_2D, gTextureIdCandle);
    model = glm::translate(candlePos) * glm::scale(candleScale) * glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gCandleMesh.nVertices);
    ///////TOPPER///////////////////////////////////////////////////////////////////////////////////////////////
    glBindVertexArray(gTopperMesh.vao);
    glBindTexture(GL_TEXTURE_2D, gTextureIdTopper);
    model = glm::translate(topperPos) * glm::scale(candleScale) * glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gTopperMesh.nVertices);
    ///////CABLE///////////////////////////////////////////////////////////////////////////////////////////////
    glBindVertexArray(gCableMesh.vao);
    glBindTexture(GL_TEXTURE_2D, gTextureIdCable);
    model = glm::translate(glm::vec3(-3.7f, -0.2f, 1.3f)) * glm::scale(glm::vec3(0.85, 0.85, 0.85)) * glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.05f, 0.6f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gCableMesh.nVertices);
    glBindVertexArray(gCableMesh.vao);
    glBindTexture(GL_TEXTURE_2D, gTextureIdCable);
    model = glm::translate(glm::vec3(-3.7f, -0.3f, 1.3f)) * glm::scale(glm::vec3(0.85, 0.87, 0.85)) * glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.06f, 0.3f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gCableMesh.nVertices);
    glBindVertexArray(gCableMesh.vao);
    glBindTexture(GL_TEXTURE_2D, gTextureIdCable);
    model = glm::translate(glm::vec3(-3.7f, -0.4f, 1.5f)) * glm::scale(glm::vec3(0.85f, 0.67, 0.85f)) * glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.07f, 0.7f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, gCableMesh.nVertices);
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    // LAMP: draw lamp
    //----------------
    glUseProgram(gLampProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPosition) * glm::scale(gLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");
    
    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gBaseMesh.nVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
    glUseProgram(0);
    float lineColor[] = { 0.2f, 0.2f, 0.2f, 1 };
    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
    // Vertex data
    GLfloat verts[] = {
        //Positions             // Normal               //Texture Coordinates
        // 
        //base Plane (granite countertop)
        -5.0f, -0.51f, -5.0f,   0.0f, 1.0f,  0.0f,     0.0f,  0.0f,
        -5.0f, -0.51f,  5.0f,   0.0f, 1.0f,  0.0f,     0.0f,  1.0f,
         5.0f, -0.51f,  5.0f,   0.0f, 1.0f,  0.0f,     1.0f,  1.0f,
         5.0f, -0.51f,  5.0f,   0.0f, 1.0f,  0.0f,     1.0f,  1.0f,
         5.0f, -0.51f, -5.0f,   0.0f, 1.0f,  0.0f,     1.0f,  0.0f,
        -5.0f, -0.51f, -5.0f,   0.0f, 1.0f,  0.0f,     0.0f,  0.0f,
       
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
}


/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}


// Implements the UCreateBlock function
void UCreateBook(GLMesh& mesh)
{
    // Vertex data
    GLfloat verts[] = {
        //Positions             // Normal               //Texture Coordinates
        // 

        // Book top
         1.5f,  0.0f,   0.5f,     0.0f,  1.0f,   0.0f,      0.0f,  0.0f,
        -1.5f,  0.0f,   0.5f,     0.0f,  1.0f,   0.0f,      0.0f,  0.4f,
         1.5f,  0.0f,  -1.5f,     0.0f,  1.0f,   0.0f,      0.24f,  0.0f,
        -1.5f,  0.0f,   0.5f,     0.0f,  1.0f,   0.0f,      0.0f,  0.4f,
         1.5f,  0.0f,  -1.5f,     0.0f,  1.0f,   0.0f,      0.24f,  0.0f,
        -1.5f,  0.0f,  -1.5f,     0.0f,  1.0f,   0.0f,      0.24f,  0.4f,
        //// Spine
         1.5f,  0.0f,   0.5f,     1.0f,  0.0f,   0.0f,      0.3f,  0.0f,
         1.5f, -0.5f,   0.5f,     1.0f,  0.0f,   0.0f,      0.245f,  0.0f,
        -1.5f, -0.5f,   0.5f,     1.0f,  0.0f,   0.0f,      0.245f,  0.4f,
        -1.5f, -0.5f,   0.5f,     1.0f,  0.0f,   0.0f,      0.245f,  0.4f,
        -1.5f,  0.0f,   0.5f,     1.0f,  0.0f,   0.0f,      0.3f, 0.4f,
         1.5f,  0.0f,   0.5f,     1.0f,  0.0f,   0.0f,      0.3f,  0.0f,
        //// Bottom Side
         1.5f,  0.0f,   0.5f,     0.0f,  0.0f,   1.0f,      0.4f, 0.0f,
         1.5f, -0.5f,   0.5f,     0.0f,  0.0f,   1.0f,      0.31f, 0.0f,
         1.5f, -0.5f,  -1.5f,     0.0f,  0.0f,   1.0f,      0.31f, 0.4f,
         1.5f, -0.5f,  -1.5f,     0.0f,  0.0f,   1.0f,      0.31f, 0.4f,
         1.5f,  0.0f,  -1.5f,     0.0f,  0.0f,   1.0f,      0.4f, 0.4f,
         1.5f,  0.0f,   0.5f,     0.0f,  0.0f,   1.0f,      0.4f, 0.0f,
        //// Right side
         1.5f, -0.5f,  -1.5f,    -1.0f,  0.0f,   0.0f,      0.4f, 0.0f,
         1.5f,  0.0f,  -1.5f,    -1.0f,  0.0f,   0.0f,      0.31f, 0.0f,
        -1.5f,  0.0f,  -1.5f,    -1.0f,  0.0f,   0.0f,      0.31f, 0.4f,
        -1.5f,  0.0f,  -1.5f,    -1.0f,  0.0f,   0.0f,      0.31f, 0.4f,
        -1.5f, -0.5f,  -1.5f,    -1.0f,  0.0f,   0.0f,      0.4f, 0.4f,
         1.5f, -0.5f,  -1.5f,    -1.0f,  0.0f,   0.0f,      0.4f, 0.0f,
        //// Top Side
        -1.5f,  0.0f,  -1.5f,     0.0f,  0.0f,  -1.0f,      0.31f, 0.0f,
        -1.5f, -0.5f,  -1.5f,     0.0f,  0.0f,  -1.0f,      0.4f, 0.0f,
        -1.5f,  0.0f,   0.5f,     0.0f,  0.0f,  -1.0f,      0.31f, 0.4f,
        -1.5f,  0.0f,   0.5f,     0.0f,  0.0f,  -1.0f,      0.31f, 0.4f,
        -1.5f, -0.5f,   0.5f,     0.0f,  0.0f,  -1.0f,      0.4f, 0.4f,
        -1.5f, -0.5f,  -1.5f,     0.0f,  0.0f,  -1.0f,      0.4f, 0.0f,
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

void UCreateBall(GLMesh& mesh)
{
    GLfloat verts[] = { 
        4.06586,0,-4.06586,0.357407,0.357407,-0.862856,0,0.75,
        -2.5134e-07,-0,-5.75,0.357407,0.357407,-0.862856,0,1,
        -1.77725e-07,4.06586,-4.06586,0.357407,0.357407,-0.862856,0.25,0.75,
        -1.77725e-07,4.06586,-4.06586,-0.357407,0.357407,-0.862856,0.25,0.75,
        1.09864e-14,-2.5134e-07,-5.75,-0.357407,0.357407,-0.862856,0.25,1,
        -4.06586,-3.55449e-07,-4.06586,-0.357407,0.357407,-0.862856,0.5,0.75,
        -4.06586,-3.55449e-07,-4.06586,-0.357407,-0.357407,-0.862856,0.5,0.75,
        2.5134e-07,2.19729e-14,-5.75,-0.357407,-0.357407,-0.862856,0.5,1,
        4.84849e-08,-4.06586,-4.06586,-0.357407,-0.357407,-0.862856,0.75,0.75,
        4.84849e-08,-4.06586,-4.06586,0.357407,-0.357407,-0.862856,0.75,0.75,
        -2.99721e-15,2.5134e-07,-5.75,0.357407,-0.357407,-0.862856,0.75,1,
        4.06586,7.10898e-07,-4.06586,0.357407,-0.357407,-0.862856,1,0.75,
        -2.5134e-07,-0,5.75,0.357407,0.357407,0.862856,0,0,
        4.06586,0,4.06586,0.357407,0.357407,0.862856,0,0.25,
        -1.77725e-07,4.06586,4.06586,0.357407,0.357407,0.862856,0.25,0.25,
        1.09864e-14,-2.5134e-07,5.75,-0.357407,0.357407,0.862856,0.25,0,
        -1.77725e-07,4.06586,4.06586,-0.357407,0.357407,0.862856,0.25,0.25,
        -4.06586,-3.55449e-07,4.06586,-0.357407,0.357407,0.862856,0.5,0.25,
        2.5134e-07,2.19729e-14,5.75,-0.357407,-0.357407,0.862856,0.5,0,
        -4.06586,-3.55449e-07,4.06586,-0.357407,-0.357407,0.862856,0.5,0.25,
        4.84849e-08,-4.06586,4.06586,-0.357407,-0.357407,0.862856,0.75,0.25,
        -2.99721e-15,2.5134e-07,5.75,0.357407,-0.357407,0.862856,0.75,0,
        4.84849e-08,-4.06586,4.06586,0.357407,-0.357407,0.862856,0.75,0.25,
        4.06586,7.10898e-07,4.06586,0.357407,-0.357407,0.862856,1,0.25,
        4.06586,0,4.06586,0.678598,0.678598,0.281085,0,0.25,
        5.75,0,0,0.678598,0.678598,0.281085,0,0.5,
        -1.77725e-07,4.06586,4.06586,0.678598,0.678598,0.281085,0.25,0.25,
        5.75,0,0,0.678598,0.678598,0.281085,0,0.5,
        -1.77725e-07,4.06586,4.06586,0.678598,0.678598,0.281085,0.25,0.25,
        -2.5134e-07,5.75,0,0.678598,0.678598,0.281085,0.25,0.5,
        -1.77725e-07,4.06586,4.06586,-0.678598,0.678598,0.281085,0.25,0.25,
        -2.5134e-07,5.75,0,-0.678598,0.678598,0.281085,0.25,0.5,
        -4.06586,-3.55449e-07,4.06586,-0.678598,0.678598,0.281085,0.5,0.25,
        -2.5134e-07,5.75,0,-0.678598,0.678598,0.281085,0.25,0.5,
        -4.06586,-3.55449e-07,4.06586,-0.678598,0.678598,0.281085,0.5,0.25,
        -5.75,-5.02681e-07,0,-0.678598,0.678598,0.281085,0.5,0.5,
        -4.06586,-3.55449e-07,4.06586,-0.678598,-0.678598,0.281085,0.5,0.25,
        -5.75,-5.02681e-07,0,-0.678598,-0.678598,0.281085,0.5,0.5,
        4.84849e-08,-4.06586,4.06586,-0.678598,-0.678598,0.281085,0.75,0.25,
        -5.75,-5.02681e-07,0,-0.678598,-0.678598,0.281085,0.5,0.5,
        4.84849e-08,-4.06586,4.06586,-0.678598,-0.678598,0.281085,0.75,0.25,
        6.85681e-08,-5.75,0,-0.678598,-0.678598,0.281085,0.75,0.5,
        4.84849e-08,-4.06586,4.06586,0.678598,-0.678598,0.281085,0.75,0.25,
        6.85681e-08,-5.75,0,0.678598,-0.678598,0.281085,0.75,0.5,
        4.06586,7.10898e-07,4.06586,0.678598,-0.678598,0.281085,1,0.25,
        6.85681e-08,-5.75,0,0.678598,-0.678598,0.281085,0.75,0.5,
        4.06586,7.10898e-07,4.06586,0.678598,-0.678598,0.281085,1,0.25,
        5.75,1.00536e-06,0,0.678598,-0.678598,0.281085,1,0.5,
        5.75,0,0,0.678598,0.678598,-0.281085,0,0.5,
        4.06586,0,-4.06586,0.678598,0.678598,-0.281085,0,0.75,
        -2.5134e-07,5.75,0,0.678598,0.678598,-0.281085,0.25,0.5,
        4.06586,0,-4.06586,0.678598,0.678598,-0.281085,0,0.75,
        -2.5134e-07,5.75,0,0.678598,0.678598,-0.281085,0.25,0.5,
        -1.77725e-07,4.06586,-4.06586,0.678598,0.678598,-0.281085,0.25,0.75,
        -2.5134e-07,5.75,0,-0.678598,0.678598,-0.281085,0.25,0.5,
        -1.77725e-07,4.06586,-4.06586,-0.678598,0.678598,-0.281085,0.25,0.75,
        -5.75,-5.02681e-07,0,-0.678598,0.678598,-0.281085,0.5,0.5,
        -1.77725e-07,4.06586,-4.06586,-0.678598,0.678598,-0.281085,0.25,0.75,
        -5.75,-5.02681e-07,0,-0.678598,0.678598,-0.281085,0.5,0.5,
        -4.06586,-3.55449e-07,-4.06586,-0.678598,0.678598,-0.281085,0.5,0.75,
        -5.75,-5.02681e-07,0,-0.678598,-0.678598,-0.281085,0.5,0.5,
        -4.06586,-3.55449e-07,-4.06586,-0.678598,-0.678598,-0.281085,0.5,0.75,
        6.85681e-08,-5.75,0,-0.678598,-0.678598,-0.281085,0.75,0.5,
        -4.06586,-3.55449e-07,-4.06586,-0.678598,-0.678598,-0.281085,0.5,0.75,
        6.85681e-08,-5.75,0,-0.678598,-0.678598,-0.281085,0.75,0.5,
        4.84849e-08,-4.06586,-4.06586,-0.678598,-0.678598,-0.281085,0.75,0.75,
        6.85681e-08,-5.75,0,0.678598,-0.678598,-0.281085,0.75,0.5,
        4.84849e-08,-4.06586,-4.06586,0.678598,-0.678598,-0.281085,0.75,0.75,
        5.75,1.00536e-06,0,0.678598,-0.678598,-0.281085,1,0.5,
        4.84849e-08,-4.06586,-4.06586,0.678598,-0.678598,-0.281085,0.75,0.75,
        5.75,1.00536e-06,0,0.678598,-0.678598,-0.281085,1,0.5,
        4.06586,7.10898e-07,-4.06586,0.678598,-0.678598,-0.281085,1,0.75,
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    //mesh.nVertices = sizeof(vertArray) / (sizeof(vertArray[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    ////// Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    //glBufferData(GL_ARRAY_BUFFER, vertArray.size() * sizeof(GLfloat), &vertArray[0], GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    ////// Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    ////// Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}
void UCreateCandle(GLMesh& mesh)
{
    //// Vertex data
    GLfloat verts[] = {
        1.5, 0, -1.5,0.923879, 0.382683, -0, 0, 1,
        1.5, 0, 1.5,0.923879, 0.382683, -0, 0, 0,
        1.06066, 1.06066, -1.5,0.923879, 0.382683, -0, 0.125, 1,
        1.5, 0, 1.5,0.923879, 0.382683, -0, 0, 0,
        1.06066, 1.06066, -1.5,0.923879, 0.382683, -0, 0.125, 1,
        1.06066, 1.06066, 1.5,0.923879, 0.382683, -0, 0.125, 0,

        1.06066, 1.06066, -1.5,0.382683, 0.92388, -0, 0.125, 1,
        1.06066, 1.06066, 1.5,0.382683, 0.92388, -0, 0.125, 0,
        -6.55671e-08, 1.5, -1.5,0.382683, 0.92388, -0, 0.25, 1,
        1.06066, 1.06066, 1.5,0.382683, 0.92388, -0, 0.125, 0,
        -6.55671e-08, 1.5, -1.5,0.382683, 0.92388, -0, 0.25, 1,
        -6.55671e-08, 1.5, 1.5,0.382683, 0.92388, -0, 0.25, 0,

        -6.55671e-08, 1.5, -1.5,-0.382684, 0.92388, 0, 0.25, 1,
        -6.55671e-08, 1.5, 1.5,-0.382684, 0.92388, 0, 0.25, 0,
        -1.06066, 1.06066, -1.5,-0.382684, 0.92388, 0, 0.375, 1,
        -6.55671e-08, 1.5, 1.5,-0.382684, 0.92388, 0, 0.25, 0,
        -1.06066, 1.06066, -1.5,-0.382684, 0.92388, 0, 0.375, 1,
        -1.06066, 1.06066, 1.5,-0.382684, 0.92388, 0, 0.375, 0,

        -1.06066, 1.06066, -1.5,-0.92388, 0.382683, 0, 0.375, 1,
        -1.06066, 1.06066, 1.5,-0.92388, 0.382683, 0, 0.375, 0,
        -1.5, -1.31134e-07, -1.5,-0.92388, 0.382683, 0, 0.5, 1,
        -1.06066, 1.06066, 1.5,-0.92388, 0.382683, 0, 0.375, 0,
        -1.5, -1.31134e-07, -1.5,-0.92388, 0.382683, 0, 0.5, 1,
        -1.5, -1.31134e-07, 1.5,-0.92388, 0.382683, 0, 0.5, 0,

        -1.5, -1.31134e-07, -1.5,-0.92388, -0.382684, 0, 0.5, 1,
        -1.5, -1.31134e-07, 1.5,-0.92388, -0.382684, 0, 0.5, 0,
        -1.06066, -1.06066, -1.5,-0.92388, -0.382684, 0, 0.625, 1,
        -1.5, -1.31134e-07, 1.5,-0.92388, -0.382684, 0, 0.5, 0,
        -1.06066, -1.06066, -1.5,-0.92388, -0.382684, 0, 0.625, 1,
        -1.06066, -1.06066, 1.5,-0.92388, -0.382684, 0, 0.625, 0,

        -1.06066, -1.06066, -1.5,-0.382683, -0.92388, 0, 0.625, 1,
        -1.06066, -1.06066, 1.5,-0.382683, -0.92388, 0, 0.625, 0,
        1.78873e-08, -1.5, -1.5,-0.382683, -0.92388, 0, 0.75, 1,
        -1.06066, -1.06066, 1.5,-0.382683, -0.92388, 0, 0.625, 0,
        1.78873e-08, -1.5, -1.5,-0.382683, -0.92388, 0, 0.75, 1,
        1.78873e-08, -1.5, 1.5,-0.382683, -0.92388, 0, 0.75, 0,

        1.78873e-08, -1.5, -1.5,0.382684, -0.923879, 0, 0.75, 1,
        1.78873e-08, -1.5, 1.5,0.382684, -0.923879, 0, 0.75, 0,
        1.06066, -1.06066, -1.5,0.382684, -0.923879, 0, 0.875, 1,
        1.78873e-08, -1.5, 1.5,0.382684, -0.923879, 0, 0.75, 0,
        1.06066, -1.06066, -1.5,0.382684, -0.923879, 0, 0.875, 1,
        1.06066, -1.06066, 1.5,0.382684, -0.923879, 0, 0.875, 0,

        1.06066, -1.06066, -1.5,0.92388, -0.382683, 0, 0.875, 1,
        1.06066, -1.06066, 1.5,0.92388, -0.382683, 0, 0.875, 0,
        1.5, 2.62268e-07, -1.5,0.92388, -0.382683, 0, 1, 1,
        1.06066, -1.06066, 1.5,0.92388, -0.382683, 0, 0.875, 0,
        1.5, 2.62268e-07, -1.5,0.92388, -0.382683, 0, 1, 1,
        1.5, 2.62268e-07, 1.5,0.92388, -0.382683, 0, 1, 0,

    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

void UCreateTopper(GLMesh& mesh)
{

    // Vertex data
    GLfloat verts[] = {
        1.6, 0, -0.5,0.92388, 0.382683, -0, 0, 1,
        1.6, 0, 0.5,0.92388, 0.382683, -0, 0, 0,
        1.13137, 1.13137, -0.5,0.92388, 0.382683, -0, 0.125, 1,
        1.6, 0, 0.5,0.92388, 0.382683, -0, 0, 0,
        1.13137, 1.13137, -0.5,0.92388, 0.382683, -0, 0.125, 1,
        1.13137, 1.13137, 0.5,0.92388, 0.382683, -0, 0.125, 0,

        1.13137, 1.13137, -0.5,0.382683, 0.92388, -0, 0.125, 1,
        1.13137, 1.13137, 0.5,0.382683, 0.92388, -0, 0.125, 0,
        -6.99382e-08, 1.6, -0.5,0.382683, 0.92388, -0, 0.25, 1,
        1.13137, 1.13137, 0.5,0.382683, 0.92388, -0, 0.125, 0,
        -6.99382e-08, 1.6, -0.5,0.382683, 0.92388, -0, 0.25, 1,
        -6.99382e-08, 1.6, 0.5,0.382683, 0.92388, -0, 0.25, 0,

        -6.99382e-08, 1.6, -0.5,-0.382683, 0.92388, 0, 0.25, 1,
        -6.99382e-08, 1.6, 0.5,-0.382683, 0.92388, 0, 0.25, 0,
        -1.13137, 1.13137, -0.5,-0.382683, 0.92388, 0, 0.375, 1,
        -6.99382e-08, 1.6, 0.5,-0.382683, 0.92388, 0, 0.25, 0,
        -1.13137, 1.13137, -0.5,-0.382683, 0.92388, 0, 0.375, 1,
        -1.13137, 1.13137, 0.5,-0.382683, 0.92388, 0, 0.375, 0,

        -1.13137, 1.13137, -0.5,-0.92388, 0.382683, 0, 0.375, 1,
        -1.13137, 1.13137, 0.5,-0.92388, 0.382683, 0, 0.375, 0,
        -1.6, -1.39876e-07, -0.5,-0.92388, 0.382683, 0, 0.5, 1,
        -1.13137, 1.13137, 0.5,-0.92388, 0.382683, 0, 0.375, 0,
        -1.6, -1.39876e-07, -0.5,-0.92388, 0.382683, 0, 0.5, 1,
        -1.6, -1.39876e-07, 0.5,-0.92388, 0.382683, 0, 0.5, 0,

        -1.6, -1.39876e-07, -0.5,-0.92388, -0.382684, 0, 0.5, 1,
        -1.6, -1.39876e-07, 0.5,-0.92388, -0.382684, 0, 0.5, 0,
        -1.13137, -1.13137, -0.5,-0.92388, -0.382684, 0, 0.625, 1,
        -1.6, -1.39876e-07, 0.5,-0.92388, -0.382684, 0, 0.5, 0,
        -1.13137, -1.13137, -0.5,-0.92388, -0.382684, 0, 0.625, 1,
        -1.13137, -1.13137, 0.5,-0.92388, -0.382684, 0, 0.625, 0,

        -1.13137, -1.13137, -0.5,-0.382683, -0.923879, 0, 0.625, 1,
        -1.13137, -1.13137, 0.5,-0.382683, -0.923879, 0, 0.625, 0,
        1.90798e-08, -1.6, -0.5,-0.382683, -0.923879, 0, 0.75, 1,
        -1.13137, -1.13137, 0.5,-0.382683, -0.923879, 0, 0.625, 0,
        1.90798e-08, -1.6, -0.5,-0.382683, -0.923879, 0, 0.75, 1,
        1.90798e-08, -1.6, 0.5,-0.382683, -0.923879, 0, 0.75, 0,

        1.90798e-08, -1.6, -0.5,0.382684, -0.923879, 0, 0.75, 1,
        1.90798e-08, -1.6, 0.5,0.382684, -0.923879, 0, 0.75, 0,
        1.13137, -1.13137, -0.5,0.382684, -0.923879, 0, 0.875, 1,
        1.90798e-08, -1.6, 0.5,0.382684, -0.923879, 0, 0.75, 0,
        1.13137, -1.13137, -0.5,0.382684, -0.923879, 0, 0.875, 1,
        1.13137, -1.13137, 0.5,0.382684, -0.923879, 0, 0.875, 0,

        1.13137, -1.13137, -0.5,0.92388, -0.382683, 0, 0.875, 1,
        1.13137, -1.13137, 0.5,0.92388, -0.382683, 0, 0.875, 0,
        1.6, 2.79753e-07, -0.5,0.92388, -0.382683, 0, 1, 1,
        1.13137, -1.13137, 0.5,0.92388, -0.382683, 0, 0.875, 0,
        1.6, 2.79753e-07, -0.5,0.92388, -0.382683, 0, 1, 1,
        1.6, 2.79753e-07, 0.5,0.92388, -0.382683, 0, 1, 0,

        1.6, 0, -0.5,0.92388, 0.382683, -0, 0, 0,
        1.13137, 1.13137, -0.5,0.92388, 0.382683, -0, 0, 1,
        0.0,  0.0, -0.5,0.92388, 0.382683, -0, 1, 1,
        1.13137, 1.13137, -0.5,0.92388, 0.382683, -0,0, 0,
        -6.99382e-08, 1.6, -0.5,0.92388, 0.382683, -0, 0, 1,
        0.0,  0.0, -0.5,0.92388, 0.382683, -0, 1, 1,
        -6.99382e-08, 1.6, -0.5,0.92388, 0.382683, -0, 0, 0,
        -1.13137, 1.13137, -0.5,0.92388, 0.382683, -0, 0, 1,
        0.0,  0.0, -0.5,0.92388, 0.382683, -0, 1, 1,
        -1.13137, 1.13137, -0.5,0.92388, 0.382683, 0, 0, 0,
        -1.6, -1.39876e-07, -0.5,0.92388, 0.382683, 0, 0, 1,
        0.0,  0.0, -0.5,0.92388, 0.382683, -0, 1, 1,
        -1.6, -1.39876e-07, -0.5,0.92388, 0.382683, -0, 0, 0,
        - 1.13137, -1.13137, -0.5,0.92388, 0.382683, -0, 0, 1,
        0.0,  0.0, -0.5,0.92388, 0.382683, -0, 1, 1,
        -1.13137, -1.13137, -0.5,0.92388, 0.382683, -0, 0, 0,
        1.90798e-08, -1.6, -0.5,0.92388, 0.382683, -0, 0, 1,
        0.0,  0.0, -0.5,0.92388, 0.382683, -0, 1, 1,
        1.90798e-08, -1.6, -0.5,0.92388, 0.382683, -0, 0, 0,
        1.13137, -1.13137, -0.5,0.92388, 0.382683, -0, 0, 1,
        0.0,  0.0, -0.5,0.92388, 0.382683, -0, 1, 1,
        1.13137, -1.13137, -0.5,0.92388, 0.382683, -0, 0, 0,
        1.6, 2.79753e-07, -0.5,0.92388, 0.382683, -0, 0, 1,
        0.0,  0.0, -0.5,0.92388, 0.382683, -0, 1, 1,
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

void UCreateCable(GLMesh& mesh)
{
 /*   buildUnitCircleVertices();
    buildVerticesFlat();*/

    // Vertex data
    GLfloat verts[] = {
        //Positions             // Normal               //Texture Coordinates
        // 
        //base Plane (granite countertop)
        1.6, 0, -0.125,0.92388, 0.382683, -0, 0, 1,
        1.6, 0, 0.125,0.92388, 0.382683, -0, 0, 0,
        1.13137, 1.13137, -0.125,0.92388, 0.382683, -0, 0.125, 1,
        1.6, 0, 0.125,0.92388, 0.382683, -0, 0, 0,
        1.13137, 1.13137, -0.125,0.92388, 0.382683, -0, 0.125, 1,
        1.13137, 1.13137, 0.125,0.92388, 0.382683, -0, 0.125, 0,

        1.13137, 1.13137, -0.125,0.382683, 0.92388, -0, 0.125, 1,
        1.13137, 1.13137, 0.125,0.382683, 0.92388, -0, 0.125, 0,
        -6.99382e-08, 1.6, -0.125,0.382683, 0.92388, -0, 0.25, 1,
        1.13137, 1.13137, 0.125,0.382683, 0.92388, -0, 0.125, 0,
        -6.99382e-08, 1.6, -0.125,0.382683, 0.92388, -0, 0.25, 1,
        -6.99382e-08, 1.6, 0.125,0.382683, 0.92388, -0, 0.25, 0,

        -6.99382e-08, 1.6, -0.125,-0.382683, 0.92388, 0, 0.25, 1,
        -6.99382e-08, 1.6, 0.125,-0.382683, 0.92388, 0, 0.25, 0,
        -1.13137, 1.13137, -0.125,-0.382683, 0.92388, 0, 0.375, 1,
        -6.99382e-08, 1.6, 0.125,-0.382683, 0.92388, 0, 0.25, 0,
        -1.13137, 1.13137, -0.125,-0.382683, 0.92388, 0, 0.375, 1,
        -1.13137, 1.13137, 0.125,-0.382683, 0.92388, 0, 0.375, 0,

        -1.13137, 1.13137, -0.125,-0.92388, 0.382683, 0, 0.375, 1,
        -1.13137, 1.13137, 0.125,-0.92388, 0.382683, 0, 0.375, 0,
        -1.6, -1.39876e-07, -0.125,-0.92388, 0.382683, 0, 0.5, 1,
        -1.13137, 1.13137, 0.125,-0.92388, 0.382683, 0, 0.375, 0,
        -1.6, -1.39876e-07, -0.125,-0.92388, 0.382683, 0, 0.5, 1,
        -1.6, -1.39876e-07, 0.125,-0.92388, 0.382683, 0, 0.5, 0,

        -1.6, -1.39876e-07, -0.125,-0.92388, -0.382684, 0, 0.5, 1,
        -1.6, -1.39876e-07, 0.125,-0.92388, -0.382684, 0, 0.5, 0,
        -1.13137, -1.13137, -0.125,-0.92388, -0.382684, 0, 0.625, 1,
        -1.6, -1.39876e-07, 0.125,-0.92388, -0.382684, 0, 0.5, 0,
        -1.13137, -1.13137, -0.125,-0.92388, -0.382684, 0, 0.625, 1,
        -1.13137, -1.13137, 0.125,-0.92388, -0.382684, 0, 0.625, 0,

        -1.13137, -1.13137, -0.125,-0.382683, -0.923879, 0, 0.625, 1,
        -1.13137, -1.13137, 0.125,-0.382683, -0.923879, 0, 0.625, 0,
        1.90798e-08, -1.6, -0.125,-0.382683, -0.923879, 0, 0.75, 1,
        -1.13137, -1.13137, 0.125,-0.382683, -0.923879, 0, 0.625, 0,
        1.90798e-08, -1.6, -0.125,-0.382683, -0.923879, 0, 0.75, 1,
        1.90798e-08, -1.6, 0.125,-0.382683, -0.923879, 0, 0.75, 0,

        1.90798e-08, -1.6, -0.125,0.382684, -0.923879, 0, 0.75, 1,
        1.90798e-08, -1.6, 0.125,0.382684, -0.923879, 0, 0.75, 0,
        1.13137, -1.13137, -0.125,0.382684, -0.923879, 0, 0.875, 1,
        1.90798e-08, -1.6, 0.125,0.382684, -0.923879, 0, 0.75, 0,
        1.13137, -1.13137, -0.125,0.382684, -0.923879, 0, 0.875, 1,
        1.13137, -1.13137, 0.125,0.382684, -0.923879, 0, 0.875, 0,

        1.13137, -1.13137, -0.125,0.92388, -0.382683, 0, 0.875, 1,
        1.13137, -1.13137, 0.125,0.92388, -0.382683, 0, 0.875, 0,
        1.6, 2.79753e-07, -0.125,0.92388, -0.382683, 0, 1, 1,
        1.13137, -1.13137, 0.125,0.92388, -0.382683, 0, 0.875, 0,
        1.6, 2.79753e-07, -0.125,0.92388, -0.382683, 0, 1, 1,
        1.6, 2.79753e-07, 0.125,0.92388, -0.382683, 0, 1, 0,

        1.75, 0, -0.125,0.92388, 0.382683, -0, 0, 1,
        1.75, 0, 0.125,0.92388, 0.382683, -0, 0, 0,
        1.23744, 1.23744, -0.125,0.92388, 0.382683, -0, 0.125, 1,
        1.75, 0, 0.125,0.92388, 0.382683, -0, 0, 0,
        1.23744, 1.23744, -0.125,0.92388, 0.382683, -0, 0.125, 1,
        1.23744, 1.23744, 0.125,0.92388, 0.382683, -0, 0.125, 0,

        1.23744, 1.23744, -0.125,0.382683, 0.92388, -0, 0.125, 1,
        1.23744, 1.23744, 0.125,0.382683, 0.92388, -0, 0.125, 0,
        -7.64949e-08, 1.75, -0.125,0.382683, 0.92388, -0, 0.25, 1,
        1.23744, 1.23744, 0.125,0.382683, 0.92388, -0, 0.125, 0,
        -7.64949e-08, 1.75, -0.125,0.382683, 0.92388, -0, 0.25, 1,
        -7.64949e-08, 1.75, 0.125,0.382683, 0.92388, -0, 0.25, 0,

        -7.64949e-08, 1.75, -0.125,-0.382683, 0.92388, 0, 0.25, 1,
        -7.64949e-08, 1.75, 0.125,-0.382683, 0.92388, 0, 0.25, 0,
        -1.23744, 1.23744, -0.125,-0.382683, 0.92388, 0, 0.375, 1,
        -7.64949e-08, 1.75, 0.125,-0.382683, 0.92388, 0, 0.25, 0,
        -1.23744, 1.23744, -0.125,-0.382683, 0.92388, 0, 0.375, 1,
        -1.23744, 1.23744, 0.125,-0.382683, 0.92388, 0, 0.375, 0,

        -1.23744, 1.23744, -0.125,-0.92388, 0.382683, 0, 0.375, 1,
        -1.23744, 1.23744, 0.125,-0.92388, 0.382683, 0, 0.375, 0,
        -1.75, -1.5299e-07, -0.125,-0.92388, 0.382683, 0, 0.5, 1,
        -1.23744, 1.23744, 0.125,-0.92388, 0.382683, 0, 0.375, 0,
        -1.75, -1.5299e-07, -0.125,-0.92388, 0.382683, 0, 0.5, 1,
        -1.75, -1.5299e-07, 0.125,-0.92388, 0.382683, 0, 0.5, 0,

        -1.75, -1.5299e-07, -0.125,-0.92388, -0.382684, 0, 0.5, 1,
        -1.75, -1.5299e-07, 0.125,-0.92388, -0.382684, 0, 0.5, 0,
        -1.23744, -1.23744, -0.125,-0.92388, -0.382684, 0, 0.625, 1,
        -1.75, -1.5299e-07, 0.125,-0.92388, -0.382684, 0, 0.5, 0,
        -1.23744, -1.23744, -0.125,-0.92388, -0.382684, 0, 0.625, 1,
        -1.23744, -1.23744, 0.125,-0.92388, -0.382684, 0, 0.625, 0,

        -1.23744, -1.23744, -0.125,-0.382683, -0.923879, 0, 0.625, 1,
        -1.23744, -1.23744, 0.125,-0.382683, -0.923879, 0, 0.625, 0,
        2.08685e-08, -1.75, -0.125,-0.382683, -0.923879, 0, 0.75, 1,
        -1.23744, -1.23744, 0.125,-0.382683, -0.923879, 0, 0.625, 0,
        2.08685e-08, -1.75, -0.125,-0.382683, -0.923879, 0, 0.75, 1,
        2.08685e-08, -1.75, 0.125,-0.382683, -0.923879, 0, 0.75, 0,

        2.08685e-08, -1.75, -0.125,0.382684, -0.923879, 0, 0.75, 1,
        2.08685e-08, -1.75, 0.125,0.382684, -0.923879, 0, 0.75, 0,
        1.23744, -1.23744, -0.125,0.382684, -0.923879, 0, 0.875, 1,
        2.08685e-08, -1.75, 0.125,0.382684, -0.923879, 0, 0.75, 0,
        1.23744, -1.23744, -0.125,0.382684, -0.923879, 0, 0.875, 1,
        1.23744, -1.23744, 0.125,0.382684, -0.923879, 0, 0.875, 0,

        1.23744, -1.23744, -0.125,0.92388, -0.382683, 0, 0.875, 1,
        1.23744, -1.23744, 0.125,0.92388, -0.382683, 0, 0.875, 0,
        1.75, 3.0598e-07, -0.125,0.92388, -0.382683, 0, 1, 1,
        1.23744, -1.23744, 0.125,0.92388, -0.382683, 0, 0.875, 0,
        1.75, 3.0598e-07, -0.125,0.92388, -0.382683, 0, 1, 1,
        1.75, 3.0598e-07, 0.125,0.92388, -0.382683, 0, 1, 0,
        ////////////////////CAP
        1.75, 0, -0.125, 0.92388, 0.382683, -0, 0,0,
        1.23744, 1.23744, -0.125, 0.92388, 0.382683, -0, 0, 1, 
        1.6, 0, -0.125, 0.92388, 0.382683, -0, 1, 1,
        1.6, 0, -0.125, 0.92388, 0.382683, -0, 0, 0, 
        1.23744, 1.23744, -0.125, 0.92388, 0.382683, -0, 0, 1,
        1.13137, 1.13137, -0.125, 0.92388, 0.382683, -0, 1, 1,

        1.23744, 1.23744, -0.125, 0.382683, 0.92388, -0, 0, 0,
        -7.64949e-08, 1.75, -0.125, 0.382683, 0.92388, -0, 0, 1,
        1.13137, 1.13137, -0.125, 0.382683, 0.92388, -0, 1, 1,
        1.13137, 1.13137, -0.125, 0.382683, 0.92388, -0, 0, 0,
        -7.64949e-08, 1.75, -0.125, 0.382683, 0.92388, -0, 0, 1,
        -6.99382e-08, 1.6, -0.125, 0.382683, 0.92388, -0, 1, 1,

        -7.64949e-08, 1.75, -0.125, -0.382683, 0.92388, 0, 0, 0,
        -1.23744, 1.23744, -0.125, -0.382683, 0.92388, 0, 0, 1,
        -6.99382e-08, 1.6, -0.125, -0.382683, 0.92388, 0, 1, 1,
        -6.99382e-08, 1.6, -0.125, -0.382683, 0.92388, 0, 0, 0,
        -1.23744, 1.23744, -0.125, -0.382683, 0.92388, 0, 0, 1,
        -1.13137, 1.13137, -0.125, -0.382683, 0.92388, 0, 1, 1,

        -1.23744, 1.23744, -0.125, -0.92388, 0.382683, 0, 0, 0,
        -1.75, -1.5299e-07, -0.125, -0.92388, 0.382683, 0, 0, 1,
        -1.13137, 1.13137, -0.125, -0.92388, 0.382683, 0, 1, 1,
        -1.13137, 1.13137, -0.125, -0.92388, 0.382683, 0, 0, 0,
        -1.75, -1.5299e-07, -0.125, -0.92388, 0.382683, 0, 0, 1,
        -1.6, -1.39876e-07, -0.125, -0.92388, 0.382683, 0, 1, 1,

        -1.75, -1.5299e-07, -0.125, -0.92388, -0.382684, 0, 0, 0,
        -1.23744, -1.23744, -0.125, -0.92388, -0.382684, 0, 0, 1,
        -1.6, -1.39876e-07, -0.125, -0.92388, -0.382684, 0, 1, 1,
        -1.6, -1.39876e-07, -0.125, -0.92388, -0.382684, 0, 0, 0,
        -1.23744, -1.23744, -0.125, -0.92388, -0.382684, 0, 0, 1,
        -1.13137, -1.13137, -0.125, -0.92388, -0.382684, 0, 1, 1,

        -1.23744, -1.23744, -0.125, -0.382683, -0.923879, 0, 0, 0,
        2.08685e-08, -1.75, -0.125, -0.382683, -0.923879, 0, 0, 1,
        -1.13137, -1.13137, -0.125, -0.382683, -0.923879, 0, 1, 1,
        -1.13137, -1.13137, -0.125, -0.382683, -0.923879, 0, 0, 0,
        2.08685e-08, -1.75, -0.125, -0.382683, -0.923879, 0, 0, 1,
        1.90798e-08, -1.6, -0.125, -0.382683, -0.923879, 0, 1, 1,

        2.08685e-08, -1.75, -0.125, 0.382684, -0.923879, 0, 0, 0,
        1.23744, -1.23744, -0.125, 0.382684, -0.923879, 0, 0, 1,
        1.90798e-08, -1.6, -0.125, 0.382684, -0.923879, 0, 1, 1,
        1.90798e-08, -1.6, -0.125, 0.382684, -0.923879, 0, 0, 0,
        1.23744, -1.23744, -0.125, 0.382684, -0.923879, 0, 0, 1,
        1.13137, -1.13137, -0.125, 0.382684, -0.923879, 0, 1, 1,

        1.23744, -1.23744, -0.125, 0.92388, -0.382683, 0, 0, 0,
        1.75, 3.0598e-07, -0.125, 0.92388, -0.382683, 0, 0, 1,
        1.13137, -1.13137, -0.125, 0.92388, -0.382683, 0, 1, 1,
        1.13137, -1.13137, -0.125, 0.92388, -0.382683, 0, 0, 0,
        1.75, 3.0598e-07, -0.125, 0.92388, -0.382683, 0, 0, 1,
        1.6, 2.79753e-07, -0.125, 0.92388, -0.382683, 0, 1, 1,
        //opp
        1.6, 0, 0.125, 0.92388, 0.382683, -0, 0, 0,
        1.13137, 1.13137, 0.125, 0.92388, 0.382683, -0, 0, 1,
        1.75, 0, 0.125, 0.92388, 0.382683, -0, 1, 1,
        1.75, 0, 0.125, 0.92388, 0.382683, -0, 0, 0,
        1.13137, 1.13137, 0.125, 0.92388, 0.382683, -0, 0, 1,
        1.23744, 1.23744, 0.125, 0.92388, 0.382683, -0, 1, 1,

        1.23744, 1.23744, 0.125, 0.382683, 0.92388, -0, 0, 0,
        -7.64949e-08, 1.75, 0.125, 0.382683, 0.92388, -0, 0, 1,
        1.13137, 1.13137, 0.125, 0.382683, 0.92388, -0, 1, 1,
        1.13137, 1.13137, 0.125, 0.382683, 0.92388, -0, 0, 0,
        -7.64949e-08, 1.75, 0.125, 0.382683, 0.92388, -0, 0, 1,
        -6.99382e-08, 1.6, 0.125, 0.382683, 0.92388, -0, 1, 1,

        -7.64949e-08, 1.75, 0.125, -0.382683, 0.92388, 0, 0, 0,
        -1.23744, 1.23744, 0.125, -0.382683, 0.92388, 0, 0, 1,
        -6.99382e-08, 1.6, 0.125, -0.382683, 0.92388, 0, 1, 1,
        -6.99382e-08, 1.6, 0.125, -0.382683, 0.92388, 0, 0, 0,
        -1.23744, 1.23744, 0.125, -0.382683, 0.92388, 0, 0, 1,
        -1.13137, 1.13137, 0.125, -0.382683, 0.92388, 0, 1, 1,

        -1.23744, 1.23744, 0.125, -0.92388, 0.382683, 0, 0, 0,
        -1.75, -1.5299e-07, 0.125, -0.92388, 0.382683, 0, 0, 1,
        -1.13137, 1.13137, 0.125, -0.92388, 0.382683, 0, 1, 1,
        -1.13137, 1.13137, 0.125, -0.92388, 0.382683, 0, 0, 0,
        -1.75, -1.5299e-07, 0.125, -0.92388, 0.382683, 0, 0, 1,
        -1.6, -1.39876e-07, 0.125, -0.92388, 0.382683, 0, 1, 1,

        -1.75, -1.5299e-07, 0.125, -0.92388, -0.382684, 0, 0, 0,
        -1.23744, -1.23744, 0.125, -0.92388, -0.382684, 0, 0, 1,
        -1.6, -1.39876e-07, 0.125, -0.92388, -0.382684, 0, 1, 1,
        -1.6, -1.39876e-07, 0.125, -0.92388, -0.382684, 0, 0, 0,
        -1.23744, -1.23744, 0.125, -0.92388, -0.382684, 0, 0, 1,
        -1.13137, -1.13137, 0.125, -0.92388, -0.382684, 0, 1, 1,

        -1.23744, -1.23744, 0.125, -0.382683, -0.923879, 0, 0, 0,
        2.08685e-08, -1.75, 0.125, -0.382683, -0.923879, 0, 0, 1,
        -1.13137, -1.13137, 0.125, -0.382683, -0.923879, 0, 1, 1,
        -1.13137, -1.13137, 0.125, -0.382683, -0.923879, 0, 0, 0,
        2.08685e-08, -1.75, 0.125, -0.382683, -0.923879, 0, 0, 1,
        1.90798e-08, -1.6, 0.125, -0.382683, -0.923879, 0, 1, 1,

        2.08685e-08, -1.75, 0.125, 0.382684, -0.923879, 0, 0, 0,
        1.23744, -1.23744, 0.125, 0.382684, -0.923879, 0, 0, 1,
        1.90798e-08, -1.6, 0.125, 0.382684, -0.923879, 0, 1, 1,
        1.90798e-08, -1.6, 0.125, 0.382684, -0.923879, 0, 0, 0,
        1.23744, -1.23744, 0.125, 0.382684, -0.923879, 0, 0, 1,
        1.13137, -1.13137, 0.125, 0.382684, -0.923879, 0, 1, 1,

        1.23744, -1.23744, 0.125, 0.92388, -0.382683, 0, 0, 0,
        1.75, 3.0598e-07, 0.125, 0.92388, -0.382683, 0, 0, 1,
        1.13137, -1.13137, 0.125, 0.92388, -0.382683, 0, 1, 1,
        1.13137, -1.13137, 0.125, 0.92388, -0.382683, 0, 0, 0,
        1.75, 3.0598e-07, 0.125, 0.92388, -0.382683, 0, 0, 1,
        1.6, 2.79753e-07, 0.125, 0.92388, -0.382683, 0, 1, 1,
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}