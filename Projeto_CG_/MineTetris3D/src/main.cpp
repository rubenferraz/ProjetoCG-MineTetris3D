#include <iostream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

#include "WindowManager.h"
#include "Shape.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Board.h"
#include "Tetromino.h"

using namespace std;
using namespace glm;

double get_last_elapsed_time();
bool isPastInterval(double interval);
void drawScore(shared_ptr<Program> tetrominoProg, shared_ptr<Shape> cube, int score);
void drawNumber(shared_ptr<Program> tetrominoProg, shared_ptr<Shape> cube, int digit, int rowOffset, int colOffset);
void drawWordNext(shared_ptr<Program> tetrominoProg, shared_ptr<Shape> cube, int rowOffset, int colOffset);
void drawWordHold(shared_ptr<Program> tetrominoProg, shared_ptr<Shape> cube, int rowOffset, int colOffset);
void drawWordMineTetris(shared_ptr<Program> tetrominoProg, shared_ptr<Shape> cube, int rowOffset, int colOffset);


class camera {
public:
	glm::vec3 pos, rot;
	int w, a, s, d;
	camera() {
		w = a = s = d = 0;
		pos = rot = glm::vec3(0, 0, 0);
        pos = vec3(-0.670194, 0, 1.48292);
        rot = vec3(0, -0.0721549, 0);
	}
	glm::mat4 process(double ftime) {
		float speed = 0;
		if (w == 1) {
			speed = 2*ftime;
		}
		else if (s == 1) {
			speed = -2*ftime;
		}
		float yangle=0;
		if (a == 1)
			yangle = -2*ftime;
		else if(d==1)
			yangle = 2*ftime;
		rot.y += yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed,1);
		dir = dir*R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R*T;
	}
};

class Application : public EventCallbacks {

public:

	WindowManager * windowManager = nullptr;

	shared_ptr<Program> tetrominoProg, backgroundProg;
    
    shared_ptr<Shape> cube, sphere;

	GLuint BlockTexId, BackgroundTexId, NormalTexId;
    
    bool leftPress, rightPress, downPress, rotate, holdPress, spacePress = 0;
    
    camera mycam;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
        if (key == GLFW_KEY_W && action == GLFW_PRESS){
			mycam.w = 1;
		}
		else if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
			mycam.w = 0;
		}
		else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
			mycam.s = 1;
		}
		else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
			mycam.s = 0;
		}
		else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
			mycam.a = 1;
		}
		else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
			mycam.a = 0;
		}
		else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			mycam.d = 1;
		}
		else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
			mycam.d = 0;
		}
        if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
            leftPress = true;
        }
        else if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
            leftPress = false;
        }
        else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
            rightPress = true;
        }
        else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
            rightPress = false;
        }
        else if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
            rotate = true;
        }
        else if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
            rotate = false;
        }
        else if (key == GLFW_KEY_SPACE && action ==  GLFW_PRESS) {
            spacePress = true;
        }
        else if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
            spacePress = false;
        }
        else if (key == GLFW_KEY_DOWN && action ==  GLFW_PRESS) {
            downPress = true;
        }
        else if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
            downPress = false;
        }
        else if (key == GLFW_KEY_X && action ==  GLFW_PRESS) {
            holdPress = true;
        }
        else if (key == GLFW_KEY_X && action == GLFW_RELEASE) {
            holdPress = false;
        }
	}

    void mouseCallback(GLFWwindow *window, int button, int action, int mods) {}
    
	void resizeCallback(GLFWwindow *window, int in_width, int in_height) {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	void initGeom() {
		string resourceDirectory = "../res" ;
		cube = make_shared<Shape>();
		cube->loadMesh(resourceDirectory + "/cube.obj");
		cube->resize();
		cube->init();
        
        sphere = make_shared<Shape>();
        sphere->loadMesh(resourceDirectory + "/sphere.obj");
        sphere->resize();
        sphere->init();

		int width, height, channels;
		char filepath[1000];
		string str = resourceDirectory + "/obs.png";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &BlockTexId);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, BlockTexId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		GLuint Tex1Location = glGetUniformLocation(tetrominoProg->pid, "blockTex");
		glUseProgram(tetrominoProg->pid);
		glUniform1i(Tex1Location, 0);
        
        
        str = resourceDirectory + "/diamond.png";
        strcpy(filepath, str.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &NormalTexId);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, NormalTexId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        GLuint Tex2Location = glGetUniformLocation(tetrominoProg->pid, "normalTex");
        glUseProgram(tetrominoProg->pid);
        glUniform1i(Tex2Location, 1);
        
        /*width, height, channels;
        filepath[1000];*/
        str = resourceDirectory + "/minewpp.jpg";
        strcpy(filepath, str.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &BackgroundTexId);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, BackgroundTexId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        GLuint Tex3Location = glGetUniformLocation(backgroundProg->pid, "backgroundTex");
        glUseProgram(backgroundProg->pid);
        glUniform1i(Tex3Location, 2);
	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory) {
		GLSL::checkVersion();
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		tetrominoProg = std::make_shared<Program>();
		tetrominoProg->setVerbose(true);
		tetrominoProg->setShaderNames("../shaders/shader_vertex_block.glsl", "../shaders/shader_fragment_block.glsl");
		if (!tetrominoProg->init()) {
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
        
        backgroundProg = std::make_shared<Program>();
        backgroundProg->setVerbose(true);
        backgroundProg->setShaderNames("../shaders/shader_vertex_background.glsl", "../shaders/shader_fragment_background.glsl");
        if (!backgroundProg->init()) {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        
		tetrominoProg->addUniform("P");
		tetrominoProg->addUniform("V");
		tetrominoProg->addUniform("M");
		tetrominoProg->addUniform("camPos");
        tetrominoProg->addUniform("blockType");
		tetrominoProg->addAttribute("vertPos");
		tetrominoProg->addAttribute("vertNor");
		tetrominoProg->addAttribute("vertTex");
        tetrominoProg->addAttribute("normalTex");
        
        backgroundProg->addUniform("P");
        backgroundProg->addUniform("V");
        backgroundProg->addUniform("M");
        backgroundProg->addUniform("camPos");
        backgroundProg->addAttribute("vertPos");
        backgroundProg->addAttribute("vertNor");
        backgroundProg->addAttribute("vertTex");
        backgroundProg->addAttribute("backgroundTex");
	}
    
	void render(Board *board, Tetromino *currentTetromino, Tetromino *nextTetromino, Tetromino *holdTetromino, Position *pos) {
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        double frametime = get_last_elapsed_time();
        mat4 V = mycam.process(frametime);
		mat4 P = ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);
		if (width < height) {
			P = ortho(-1.0f, 1.0f, -1.0f / aspect,  1.0f / aspect, -2.0f, 100.0f);
        }
		P = perspective((float)(3.14159 / 2.2), (float)((float)width/ (float)height), 0.1f, 1000.0f);
        
        static float angle = 3.14/2.0;
        mat4 skyTrans = translate(mat4(1), -mycam.pos);
        mat4 skyTrans2 = translate(mat4(1), vec3(0, 0.8, 0));
        mat4 skyScale = scale(mat4(1), vec3(2, 2, 2));
        mat4 skyRotation = glm::rotate(mat4(1), angle, vec3(1.0f, 0.0f, 0.0f));
        mat4 skyM = skyTrans * skyTrans2 * skyRotation * skyScale;
        
        backgroundProg->bind();
        glDisable(GL_DEPTH_TEST);
        glUniformMatrix4fv(backgroundProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(backgroundProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(backgroundProg->getUniform("M"), 1, GL_FALSE, &skyM[0][0]);
        glUniform3fv(backgroundProg->getUniform("camPos"), 1, &mycam.pos[0]);
        sphere->draw(backgroundProg, false);
        backgroundProg->unbind();
        
        glEnable(GL_DEPTH_TEST);
        
        tetrominoProg->bind();
        glUniformMatrix4fv(tetrominoProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(tetrominoProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniform3fv(tetrominoProg->getUniform("camPos"), 1, &mycam.pos[0]);
        tetrominoProg->unbind();

		mat4 tetrominoScaleMat = scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
        mat4 tetrominoTransMat, M;
        
        static Position ghostPos;
        static int score = 0;
        
        //tetromino moves down every second, need to check if elapsed time is >= 1 second
        if (isPastInterval(1)) {
            pos->row++;
        }
        
        // check left, right and rotate
        if (leftPress) {
            pos->col--;
            if (board->isCollision(*currentTetromino, *pos)) pos->col++;
            leftPress = false;
        } else if (rightPress) {
            pos->col++;
            if (board->isCollision(*currentTetromino, *pos)) pos->col--;
            rightPress = false;
        } else if (downPress) {
            pos->row++;
            downPress = false;
        } else if (rotate) {
            currentTetromino->rotation = (currentTetromino->rotation + 1) % 4;
            if (board->isCollision(*currentTetromino, *pos)) {
                currentTetromino->rotation--;
                if (currentTetromino->rotation < 0) currentTetromino->rotation = 3;
            }
            rotate = false;
        } else if (spacePress) {
            while (!board->isCollision(*currentTetromino, *pos)) {
                pos->row++;
            }
            spacePress = false;
        } else if (holdPress) {
            if (holdTetromino->type == -1) {
                holdTetromino->type = currentTetromino->type;
                holdTetromino->rotation = currentTetromino->rotation;
                currentTetromino->type = nextTetromino->type;
                currentTetromino->rotation = nextTetromino->rotation;
                nextTetromino->type = rand() % 7;
                nextTetromino->rotation = rand() % 4;
            } else {
                Tetromino temp;
                temp.type = currentTetromino->type;
                temp.rotation = currentTetromino->rotation;
                currentTetromino->type = holdTetromino->type;
                currentTetromino->rotation = holdTetromino->rotation;
                holdTetromino->type = temp.type;
                holdTetromino->rotation = temp.rotation;
            }
            while (board->isCollision(*currentTetromino, *pos)) {
                if (pos->col > COLUMNS / 2) pos->col--;
                else if (pos->col <= COLUMNS / 2) pos->col++;
            }
            holdPress = false;
        }
        
        //check if collision, if so, store piece at pos->row-1;
        if (board->isCollision(*currentTetromino, *pos)) {
            pos->row--;
            board->storeTetromino(*currentTetromino, *pos);
            currentTetromino->type = nextTetromino->type;
            currentTetromino->rotation = nextTetromino->rotation;
            nextTetromino->type = rand() % 7;
            nextTetromino->rotation = rand() % 4;
            pos->row = 0;
            pos->col = 5;
        }
        
        if (board->isGameOver()) {
            board->clearBoard();
            holdTetromino->type = -1;
            holdTetromino->rotation = -1;
            score = 0;
        }
        
        //desenha tetromino atual
        for (int row = 0; row < BLOCK_SIZE; row++) {
            for (int col = 0; col < BLOCK_SIZE; col++) {
                Position tetrominoPos;
                tetrominoPos.row = row;
                tetrominoPos.col = col;
                if (currentTetromino->getValue(tetrominoPos) != 0) {
                    tetrominoTransMat = translate(mat4(1), vec3((col+pos->col)/5.0 - 1, -(row+pos->row)/5.0 + 1.9, -6));
                    M = tetrominoTransMat * tetrominoScaleMat;
                    tetrominoProg->bind();
                    glUniformMatrix4fv(tetrominoProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                    glUniform1i(tetrominoProg->getUniform("blockType"), currentTetromino->getValue(tetrominoPos));
                    cube->draw(tetrominoProg,false);
                    tetrominoProg->unbind();
                }
            }
        }
        
        //desenha tetromino silhueta
        ghostPos.row = pos->row;
        ghostPos.col = pos->col;
        
        while (!board->isCollision(*currentTetromino, ghostPos)) {
            ghostPos.row++;
        }
        ghostPos.row--;
        
        for (int row = 0; row < BLOCK_SIZE; row++) {
            for (int col = 0; col < BLOCK_SIZE; col++) {
                Position tetrominoPos;
                tetrominoPos.row = row;
                tetrominoPos.col = col;
                if (currentTetromino->getValue(tetrominoPos) != 0) {
                    tetrominoTransMat = translate(mat4(1), vec3((col+ghostPos.col)/5.0 - 1, -(row+ghostPos.row)/5.0 + 1.9, -6));
                    M = tetrominoTransMat * tetrominoScaleMat;
                    tetrominoProg->bind();
                    glUniformMatrix4fv(tetrominoProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                    glUniform1i(tetrominoProg->getUniform("blockType"), -1);
                    cube->draw(tetrominoProg,false);
                    tetrominoProg->unbind();
                }
            }
        }
        
        
        //desenha tetromino NEXT
        for (int row = 0; row < BLOCK_SIZE; row++) {
            for (int col = 0; col < BLOCK_SIZE; col++) {
                Position tetrominoPos;
                tetrominoPos.row = row;
                tetrominoPos.col = col;
                if (nextTetromino->getValue(tetrominoPos) != 0) {
                    tetrominoTransMat = translate(mat4(1), vec3((col+18)/5.0 - 1, -(row+7)/5.0 + 1.9, -6));
                    M = tetrominoTransMat * tetrominoScaleMat;
                    tetrominoProg->bind();
                    glUniformMatrix4fv(tetrominoProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                    glUniform1i(tetrominoProg->getUniform("blockType"), nextTetromino->getValue(tetrominoPos));
                    cube->draw(tetrominoProg,false);
                    tetrominoProg->unbind();
                }
            }
        }
        
        //desenha tetromino HOLD
        if (holdTetromino->type != -1) {
            for (int row = 0; row < BLOCK_SIZE; row++) {
                for (int col = 0; col < BLOCK_SIZE; col++) {
                    Position tetrominoPos;
                    tetrominoPos.row = row;
                    tetrominoPos.col = col;
                    if (holdTetromino->getValue(tetrominoPos) != 0) {
                        tetrominoTransMat = translate(mat4(1), vec3((col-10)/5.0 - 1, -(row+7)/5.0 + 1.9, -6));
                        M = tetrominoTransMat * tetrominoScaleMat;
                        tetrominoProg->bind();
                        glUniformMatrix4fv(tetrominoProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                        glUniform1i(tetrominoProg->getUniform("blockType"), holdTetromino->getValue(tetrominoPos));
                        cube->draw(tetrominoProg,false);
                        tetrominoProg->unbind();
                    }
                }
            }
        }
        
        drawScore(tetrominoProg, cube, score);
        drawWordNext(tetrominoProg, cube, -1, 13);
        drawWordHold(tetrominoProg, cube, -1, -22);
        drawWordMineTetris(tetrominoProg, cube, -10, -24); //Mudar lugar palavra

        
        //draw board
        for (int row = 0; row < 20; row++) {
            for (int col = 0; col < 10; col++) {
                if (board->board[row][col] != 0 ) {
                    tetrominoTransMat = translate(mat4(1), vec3(col/5.0 - 1, -row/5.0 + 1.9, -6));
                    M = tetrominoTransMat * tetrominoScaleMat;
                    tetrominoProg->bind();
                    glUniformMatrix4fv(tetrominoProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                    glUniform1i(tetrominoProg->getUniform("blockType"), board->board[row][col]);
                    cube->draw(tetrominoProg,false);
                    tetrominoProg->unbind();
                }
            }
        }
        for (int row = -1; row <= ROWS; row++) {
            for (int col = -1; col <= COLUMNS; col++) {
                if (row == -1 || row == ROWS || col == -1 || col == COLUMNS) {
                    tetrominoTransMat = translate(mat4(1), vec3(col/5.0 - 1, -row/5.0 + 1.9, -6));
                    M = tetrominoTransMat * tetrominoScaleMat;
                    tetrominoProg->bind();
                    glUniformMatrix4fv(tetrominoProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                    glUniform1i(tetrominoProg->getUniform("blockType"), -2);
                    cube->draw(tetrominoProg,false);
                    tetrominoProg->unbind();
                }
            }
        }
        score += board->deleteFullLines();
	}
};

int main(int argc, char **argv) {
	std::string resourceDir = "../resources";
	if (argc >= 2) {
		resourceDir = argv[1];
	}

	Application *application = new Application();

	WindowManager * windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	application->init(resourceDir);
	application->initGeom();
    
    srand(time(NULL));
    Board *board = new Board();
    Position currentPos;
    Tetromino currentTetromino, nextTetromino, holdTetromino;
    
    currentPos.row = 0;
    currentPos.col = 5;
    
    currentTetromino.type = rand() % 7;
    currentTetromino.rotation = rand() % 4;
    nextTetromino.type = rand() % 7;
    nextTetromino.rotation = rand() % 4;
    
    holdTetromino.type = -1;
    holdTetromino.rotation = -1;
    
	while(!glfwWindowShouldClose(windowManager->getHandle())) {
		application->render(board, &currentTetromino, &nextTetromino, &holdTetromino, &currentPos);
		glfwSwapBuffers(windowManager->getHandle());
		glfwPollEvents();
	}

	windowManager->shutdown();
	return 0;
}

double get_last_elapsed_time() {
    static double lasttime = glfwGetTime();
    double actualtime = glfwGetTime();
    double difference = actualtime- lasttime;
    lasttime = actualtime;
    return difference;
}

bool isPastInterval(double interval) {
    static double previousTime = glfwGetTime();
    double currentTime = glfwGetTime();
    if (currentTime - previousTime >= interval) {
        previousTime = currentTime;
        return true;
    }
    return false;
}

void drawScore(shared_ptr<Program> tetrominoProg, shared_ptr<Shape> cube, int score) {
    int counter = 0;
    int digit = 0;
    int value = score;
    vector<int> digits;
    
    if (value == 0) drawNumber(tetrominoProg, cube, 0, 16, 13);
    
    while (value > 0) {
        digit = value % 10;
        value /= 10;
        digits.push_back(digit);
    }
    
    for (int index = digits.size() - 1; index >= 0; index--) {
        drawNumber(tetrominoProg, cube, digits.at(index), 16, 13 + (5 * counter));
        counter++;
    }
}

void drawNumber(shared_ptr<Program> tetrominoProg, shared_ptr<Shape> cube, int digit, int rowOffset, int colOffset) {
    int numbers[10][5][4] = {
        { //0
            {1, 1, 1, 1},
            {1, 0, 0, 1},
            {1, 0, 0, 1},
            {1, 0, 0, 1},
            {1, 1, 1, 1}
        },
        { //1
            {0, 1, 1, 0},
            {0, 0, 1, 0},
            {0, 0, 1, 0},
            {0, 0, 1, 0},
            {0, 1, 1, 1}
        },
        { //2
            {1, 1, 1, 1},
            {0, 0, 0, 1},
            {1, 1, 1, 1},
            {1, 0, 0, 0},
            {1, 1, 1, 1}
        },
        { //3
            {1, 1, 1, 1},
            {0, 0, 0, 1},
            {1, 1, 1, 1},
            {0, 0, 0, 1},
            {1, 1, 1, 1}
        },
        { //4
            {1, 0, 0, 1},
            {1, 0, 0, 1},
            {1, 1, 1, 1},
            {0, 0, 0, 1},
            {0, 0, 0, 1}
        },
        { //5
            {1, 1, 1, 1},
            {1, 0, 0, 0},
            {1, 1, 1, 1},
            {0, 0, 0, 1},
            {1, 1, 1, 1}
        },
        { //6
            {1, 1, 1, 1},
            {1, 0, 0, 0},
            {1, 1, 1, 1},
            {1, 0, 0, 1},
            {1, 1, 1, 1}
        },
        { //7
            {1, 1, 1, 1},
            {0, 0, 0, 1},
            {0, 0, 0, 1},
            {0, 0, 0, 1},
            {0, 0, 0, 1}
        },
        { //8
            {1, 1, 1, 1},
            {1, 0, 0, 1},
            {1, 1, 1, 1},
            {1, 0, 0, 1},
            {1, 1, 1, 1}
        },
        { //9
            {1, 1, 1, 1},
            {1, 0, 0, 1},
            {1, 1, 1, 1},
            {0, 0, 0, 1},
            {0, 0, 0, 1}
        }
    };
    mat4 blockScaleMat = scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
    mat4 blockTransMat;
    
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 4; col++) {
            if (numbers[digit][row][col] != 0) {
                blockTransMat = translate(mat4(1), vec3((col+colOffset)/5.0 - 1, -(row+rowOffset)/5.0 + 1.9, -6));
                mat4 M = blockTransMat * blockScaleMat;
                tetrominoProg->bind();
                glUniformMatrix4fv(tetrominoProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                glUniform1i(tetrominoProg->getUniform("blockType"), -3);
                cube->draw(tetrominoProg,false);
                tetrominoProg->unbind();
            }
        }
    }
}

void drawWordNext(shared_ptr<Program> tetrominoProg, shared_ptr<Shape> cube, int rowOffset, int colOffset) {
    int word[4][5][4] = {
        { //N
            {1, 0, 0, 1},
            {1, 1, 0, 1},
            {1, 1, 1, 1},
            {1, 0, 1, 1},
            {1, 0, 0, 1}
        },
        { //E
            {1, 1, 1, 1},
            {1, 1, 0, 0},
            {1, 1, 1, 0},
            {1, 1, 0, 0},
            {1, 1, 1, 1}
        },
        { //X
            {1, 0, 0, 1},
            {1, 1, 1, 1},
            {0, 1, 1, 0},
            {1, 0, 0, 1},
            {1, 0, 0, 1}
        },
        { //T
            {1, 1, 1, 1},
            {1, 1, 1, 1},
            {0, 1, 1, 0},
            {0, 1, 1, 0},
            {0, 1, 1, 0}
        }
    };
    
    mat4 blockScaleMat = scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
    mat4 blockTransMat;
    
    for (int letter = 0; letter < 4; letter++) {
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 4; col++) {
                if (word[letter][row][col] != 0) {
                    blockTransMat = translate(mat4(1), vec3((col+(colOffset + (letter * 5)))/5.0 - 1, -(row+rowOffset)/5.0 + 1.9, -6));
                    mat4 M = blockTransMat * blockScaleMat;
                    tetrominoProg->bind();
                    glUniformMatrix4fv(tetrominoProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                    glUniform1i(tetrominoProg->getUniform("blockType"), -3);
                    cube->draw(tetrominoProg,false);
                    tetrominoProg->unbind();
                }
            }
        }
    }
}

void drawWordHold(shared_ptr<Program> tetrominoProg, shared_ptr<Shape> cube, int rowOffset, int colOffset) {
    int word[4][5][4] = {
        { //H
            {1, 0, 0, 1},
            {1, 0, 0, 1},
            {1, 1, 1, 1},
            {1, 0, 0, 1},
            {1, 0, 0, 1}
        },
        { //O
            {1, 1, 1, 1},
            {1, 0, 0, 1},
            {1, 0, 0, 1},
            {1, 0, 0, 1},
            {1, 1, 1, 1}
        },
        { //L
            {1, 0, 0, 0},
            {1, 0, 0, 0},
            {1, 0, 0, 0},
            {1, 0, 0, 0},
            {1, 1, 1, 1}
        },
        { //D
            {1, 1, 1, 0},
            {1, 0, 0, 1},
            {1, 0, 0, 1},
            {1, 0, 0, 1},
            {1, 1, 1, 0}
        }
    };
    
    mat4 blockScaleMat = scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
    mat4 blockTransMat;
    
    for (int letter = 0; letter < 4; letter++) {
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 4; col++) {
                if (word[letter][row][col] != 0) {
                    blockTransMat = translate(mat4(1), vec3((col+(colOffset + (letter * 5)))/5.0 - 1, -(row+rowOffset)/5.0 + 1.9, -6));
                    mat4 M = blockTransMat * blockScaleMat;
                    tetrominoProg->bind();
                    glUniformMatrix4fv(tetrominoProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                    glUniform1i(tetrominoProg->getUniform("blockType"), -3);
                    cube->draw(tetrominoProg,false);
                    tetrominoProg->unbind();
                }
            }
        }
    }
}

void drawWordMineTetris(shared_ptr<Program> tetrominoProg, shared_ptr<Shape> cube, int rowOffset, int colOffset) {
    int word[12][5][4] = {
    { // M
        {1, 0, 0, 1},
        {1, 1, 1, 1},
        {1, 1, 1, 1},
        {1, 0, 0, 1},
        {1, 0, 0, 1}
    },
    { // I
        {1, 1, 1, 1},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {1, 1, 1, 1}
    },
    { // N
        {1, 0, 0, 1},
        {1, 1, 0, 1},
        {1, 0, 1, 1},
        {1, 0, 1, 1},
        {1, 0, 0, 1}
    },
    { // E
        {1, 1, 1, 1},
        {1, 0, 0, 0},
        {1, 1, 1, 0},
        {1, 0, 0, 0},
        {1, 1, 1, 1}
    },
    { // T
        {1, 1, 1, 1},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0}
    },
    { // E
        {1, 1, 1, 1},
        {1, 0, 0, 0},
        {1, 1, 1, 0},
        {1, 0, 0, 0},
        {1, 1, 1, 1}
    },
    { // T
        {1, 1, 1, 1},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0}
    },
    { // R
        {1, 1, 1, 0},
        {1, 0, 0, 1},
        {1, 1, 1, 0},
        {1, 0, 1, 0},
        {1, 0, 0, 1}
    },
    { // I
        {1, 1, 1, 1},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {1, 1, 1, 1}
    },
    { // S
        {0, 1, 1, 1},
        {1, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 1},
        {1, 1, 1, 0}
    },
    { // 3
        {1, 1, 1, 0},
        {0, 0, 0, 1},
        {0, 1, 1, 0},
        {0, 0, 0, 1},
        {1, 1, 1, 0}
    },
    { // D
        {1, 1, 1, 0},
        {1, 0, 0, 1},
        {1, 0, 0, 1},
        {1, 0, 0, 1},
        {1, 1, 1, 0}
    }
};

    
    mat4 blockScaleMat = scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
    mat4 blockTransMat;
    
    for (int letter = 0; letter < 12; letter++) {
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 4; col++) {
                if (word[letter][row][col] != 0) {
                    blockTransMat = translate(mat4(1), vec3((col+(colOffset + (letter * 5)))/5.0 - 1, -(row+rowOffset)/5.0 + 1.9, -6));
                    mat4 M = blockTransMat * blockScaleMat;
                    tetrominoProg->bind();
                    glUniformMatrix4fv(tetrominoProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                    glUniform1i(tetrominoProg->getUniform("blockType"), -30);
                    cube->draw(tetrominoProg,false);
                    tetrominoProg->unbind();
                }
            }
        }
    }
}