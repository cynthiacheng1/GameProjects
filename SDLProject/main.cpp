#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, projectionMatrix, player1Matrix, player2Matrix, ballMatrix;

glm::vec3 player1_position = glm::vec3(0,0,0);
glm::vec3 player1_movement = glm::vec3(0,0,0);
glm::vec3 player2_position = glm::vec3(0,0,0);
glm::vec3 player2_movement = glm::vec3(0,0,0);
glm::vec3 ball_position = glm::vec3(0,0,0);
glm::vec3 ball_movement = glm::vec3(0,0,0);
float player_speed = 1.0f;



void Initialize() {
    //initializing SDL
    SDL_Init(SDL_INIT_VIDEO);
    //create a window(title, centered, pixels, openGL context)
    displayWindow = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320, 240, SDL_WINDOW_OPENGL);
    //open GL context + draw to that window
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    //Cam to draw from here
    glViewport(0, 0, 640, 480);

    //setting up shader program, use this to fill in points
    program.Load("shaders/vertex.glsl", "shaders/fragment.glsl");

    //identity matrix
    viewMatrix = glm::mat4(1.0f);
    player1Matrix = glm::mat4(1.0f);
    player2Matrix = glm::mat4(1.0f);
    ballMatrix = glm::mat4(1.0f);
    //the box of my world
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    //use this for looking at stuff
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    //set color to red (last one is alpha channel)
    //program.SetColor(1.0f, 0.0f, 0.0f, 1.0f);

    //use this to render
    glUseProgram(program.programID);
    
    player1_position.x = 4.7;
    player1_position.y = 0;
    
    player2_position.x = -4.7;
    player2_position.y = 0;
    
    ball_position.x = 0;
    ball_position.y = 0;
    
    //background color - whenever you clear the screen use this color
    glClearColor(0.7f, 0.6f, 0.85f, 1.0f);
}

void ProcessInput() {
    player1_movement = glm::vec3(0);
    player2_movement = glm::vec3(0);
    ball_movement = glm::vec3(0);
     SDL_Event event;
     while (SDL_PollEvent(&event)) {
         switch (event.type) {
             case SDL_QUIT:
             case SDL_WINDOWEVENT_CLOSE:
                 gameIsRunning = false;
                 break;

             case SDL_KEYDOWN:
             switch (event.key.keysym.sym) {
                 case SDLK_LEFT:
                     ball_movement.x = -1.0f;
                 // Move the player left
                 break;

                 case SDLK_RIGHT:
                     ball_movement.x = 1.0f;
                     break;

                 case SDLK_SPACE:
                     // Some sort of action
                     break;
            }
             break; // SDL_KEYDOWN
             }
         }
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_LEFT]) {
        player1_movement.y = -1.0f;
        //ball_movement.y = -1.0f;
    }
    else if (keys[SDL_SCANCODE_RIGHT]) {
        player1_movement.y = 1.0f;
        //ball_movement.y = 1.0f;
    }
    if(keys[SDL_SCANCODE_A]){
        player2_movement.y = -1.0f;
    }
    else if (keys[SDL_SCANCODE_D]){
        player2_movement.y = 1.0f;
    }
    
    if (glm::length(player1_movement) > 1.0f){
        player1_movement = glm::normalize(player1_movement);
    }
    if (glm::length(player2_movement) > 1.0f){
        player2_movement = glm::normalize(player2_movement);
    }
    if (glm::length(ball_movement) > 1.0f){
        ball_movement = glm::normalize(ball_movement);
    }

}
float lastTicks = 0.0f;
int x_direction = 1;
int y_direction = 1;
void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    player1_position += player1_movement*player_speed*deltaTime;
    if (player1_position.y > 2.7 ){
        if (player1_movement.y < 0){
            player1_position += player1_movement*player_speed*deltaTime;
        }
        player1Matrix = player1Matrix;
    }
    else if (player1_position.y < -2.7 ){
        if (player1_movement.y > 0){
            player1_position += player1_movement*player_speed*deltaTime;
        }
        player1Matrix = player1Matrix;
    }
    else{
        player1_position += player1_movement*player_speed*deltaTime;
        player1Matrix = glm::mat4(1.0f);
        player1Matrix = glm::translate(player1Matrix, player1_position);
    }

    if (player2_position.y > 2.7 ){
        if (player2_movement.y < 0){
            player2_position += player2_movement*player_speed*deltaTime;
        }
        player2Matrix = player2Matrix;
    }
    else if (player2_position.y < -2.7){
        //only updates if in the opp direction
        if (player2_movement.y > 0){
            player2_position += player2_movement*player_speed*deltaTime;
        }
        player2Matrix = player2Matrix;
    }
    else{
        player2_position += player2_movement*player_speed*deltaTime;
        player2Matrix = glm::mat4(1.0f);
        player2Matrix = glm::translate(player2Matrix, player2_position);
    }
    
//    printf("%6.4lf", player1_position.y);

    float xdist_p1 = fabs((player1_position.x)-ball_position.x)-((.5+.5)/2.0f);
    float ydist_p1 = fabs((player1_position.y)-ball_position.y)-((.5+2)/2.0f);
    
    float xdist_p2 = fabs((player2_position.x)-ball_position.x)-((.5+.5)/2.0f);
    float ydist_p2 = fabs((player2_position.y)-ball_position.y)-((.5+2)/2.0f);
    

    if (xdist_p1<0 && ydist_p1<0){
        ballMatrix = glm::mat4(1.0f);
        ball_movement.x = 1.0f*x_direction;
        ball_movement.y = 1.0f*y_direction;
        ball_position += ball_movement*player_speed*deltaTime;
        ballMatrix = glm::translate(ballMatrix, ball_position);
        x_direction = -1;
    }
    else if (xdist_p2<0 && ydist_p2<0){
        ballMatrix = glm::mat4(1.0f);
        ball_movement.x = 1.0f*x_direction;
        ball_movement.y = 1.0f*y_direction;
        ball_position += ball_movement*player_speed*deltaTime;
        ballMatrix = glm::translate(ballMatrix, ball_position);
        x_direction = 1;
    }
    //top wall
    if (ball_position.y > 3.5){
        ballMatrix = glm::mat4(1.0f);
        ball_movement.x = 1.0f*x_direction;
        ball_movement.y = 1.0f*y_direction;
        ball_position += ball_movement*player_speed*deltaTime;
        ballMatrix = glm::translate(ballMatrix, ball_position);
        y_direction = -1;
    }
    //bottom wall
    else if (ball_position.y < -3.5){
        ballMatrix = glm::mat4(1.0f);
        ball_movement.x = 1.0f*x_direction;
        ball_movement.y = 1.0f*y_direction;
        ball_position += ball_movement*player_speed*deltaTime;
        ballMatrix = glm::translate(ballMatrix, ball_position);
        y_direction = 1;
    }
    else if (ball_position.x > 4.4 ||ball_position.x < -4.3){
        gameIsRunning = false;
    }
    else{
        ballMatrix = glm::mat4(1.0f);
        ball_movement.x = 1.0f*x_direction;
        ball_movement.y = 1.0f*y_direction;
        ball_position += ball_movement*player_speed*deltaTime;
        ballMatrix = glm::translate(ballMatrix, ball_position);
    }
    
}

void Render() {
    //clear screen
    glClear(GL_COLOR_BUFFER_BIT);

    float vertices[] = {-.25,-1,-.25,1,.25,1,-.25,-1,.25,-1,.25,1};
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    
    //telling shader use this model matrix
    program.SetModelMatrix(player1Matrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    program.SetModelMatrix(player2Matrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    float ballvertices[] = {-.25,-.25,-.25,.25,.25,.25,-.25,-.25,.25,-.25,.25,.25};
    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ballvertices);
    
    program.SetModelMatrix(ballMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program.positionAttribute);
    SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();

    //our main loop
    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }

    Shutdown();
    return 0;
}
