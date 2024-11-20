//
//  main.cpp
//  sphere
//
//  Created by Эдвард on 20.11.2024.
//

#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Параметры сферы
const int latitudeBands = 40; // Количество полос по широте
const int longitudeBands = 40; // Количество полос по долготе
const float radius = 2.0f; // Радиус сферы

// Буферы для хранения вершин и индексов
std::vector<GLfloat> sphereVertices; // Координаты вершин
std::vector<GLuint> sphereIndices; // Индексы треугольников

// Функция для генерации вершин и индексов сферы
void generateSphere() {
    for (int lat = 0; lat <= latitudeBands; ++lat) {
        float theta = lat * glm::pi<float>() / latitudeBands; // Угол широты
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int lon = 0; lon <= longitudeBands; ++lon) {
            float phi = lon * 2.0f * glm::pi<float>() / longitudeBands; // Угол долготы
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            // Вычисление координат вершины
            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;

            // Добавление позиции вершины
            sphereVertices.push_back(radius * x);
            sphereVertices.push_back(radius * y);
            sphereVertices.push_back(radius * z);

            // Добавление нормали вершины (единичный вектор)
            sphereVertices.push_back(x);
            sphereVertices.push_back(y);
            sphereVertices.push_back(z);

            // Добавление цвета вершины для визуализации
            sphereVertices.push_back((x + 1.0f) * 0.8f); // Красный компонент
            sphereVertices.push_back((y + 1.0f) * 0.8f); // Зелёный компонент
            sphereVertices.push_back((z + 1.0f) * 0.8f); // Синий компонент
        }
    }

    // Создание треугольников из вершин
    for (int lat = 0; lat < latitudeBands; ++lat) {
        for (int lon = 0; lon < longitudeBands; ++lon) {
            int first = (lat * (longitudeBands + 1)) + lon;
            int second = first + longitudeBands + 1;

            // Треугольник 1
            sphereIndices.push_back(first);
            sphereIndices.push_back(second);
            sphereIndices.push_back(first + 1);

            // Треугольник 2
            sphereIndices.push_back(second);
            sphereIndices.push_back(second + 1);
            sphereIndices.push_back(first + 1);
        }
    }
}

// Параметры камеры
glm::vec3 cameraPos = glm::vec3(5.0f, 5.0f, 5.0f); // Позиция камеры
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); // Вектор вверх для камеры

// Обработка ввода от пользователя
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true); // Закрытие окна при нажатии ESC
}

// Вершинный шейдер
const char* vertexShaderSource = R"(
#version 120
attribute vec3 aPos; // Позиция вершины
attribute vec3 aNormal; // Нормаль вершины
attribute vec3 aColor; // Цвет вершины

varying vec3 fragColor; // Цвет, передаваемый в фрагментный шейдер
varying vec3 fragNormal; // Нормаль, передаваемая в фрагментный шейдер

uniform mat4 model; // Матрица модели
uniform mat4 view; // Матрица вида
uniform mat4 projection; // Матрица проекции
uniform mat3 normalMatrix; // Матрица нормалей

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0); // Расчёт позиции
    fragNormal = normalize(normalMatrix * aNormal); // Трансформация нормалей
    fragColor = aColor; // Передача цвета
}
)";

// Фрагментный шейдер
const char* fragmentShaderSource = R"(
#version 120
varying vec3 fragColor; // Цвет вершины
varying vec3 fragNormal; // Нормаль вершины

uniform vec3 lightDir; // Направление света
uniform vec3 viewDir; // Направление камеры

void main() {
    vec3 norm = normalize(fragNormal); // Нормализация нормали
    vec3 light = normalize(lightDir); // Нормализация направления света

    // Диффузное освещение
    float ambient = 0.2; // Окружающий свет
    float diff = max(dot(norm, light), 0.0); // Интенсивность света

    // Цвет света
    vec3 lightColor = vec3(0.5, 0.5, 0.5); // Белый свет
    vec3 resultColor = (ambient + diff) * fragColor * lightColor; // Итоговый цвет
    gl_FragColor = vec4(resultColor, 1.0); // Установка цвета фрагмента
}
)";

// Функция компиляции шейдеров
GLuint setupShaders(const char* vertexSrc, const char* fragmentSrc) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSrc, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSrc, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int main() {
    // Инициализация GLFW
    if (!glfwInit()) {
        std::cerr << "Не удалось инициализировать GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // Создание окна
    GLFWwindow* window = glfwCreateWindow(800, 600, "Триангуляция сферы", NULL, NULL);
    if (!window) {
        std::cerr << "Не удалось создать окно GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Инициализация GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Не удалось инициализировать GLEW" << std::endl;
        return -1;
    }

    // Включение теста глубины
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Установка цвета фона

    generateSphere(); // Генерация сферы

    GLuint shaderProgram = setupShaders(vertexShaderSource, fragmentShaderSource);

    // Создание VAO, VBO, EBO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Загрузка данных вершин
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(GLfloat), &sphereVertices[0], GL_STATIC_DRAW);

    // Загрузка индексов
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(GLuint), &sphereIndices[0], GL_STATIC_DRAW);

    // Настройка атрибутов вершин
    GLuint posAttrib = glGetAttribLocation(shaderProgram, "aPos");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);

    GLuint normalAttrib = glGetAttribLocation(shaderProgram, "aNormal");
    glEnableVertexAttribArray(normalAttrib);
    glVertexAttribPointer(normalAttrib, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));

    GLuint colorAttrib = glGetAttribLocation(shaderProgram, "aColor");
    glEnableVertexAttribArray(colorAttrib);
    glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));

    float rotationAngle = 0.0f; // Угол вращения

    // Главный цикл отрисовки
    while (!glfwWindowShouldClose(window)) {
        processInput(window); // Обработка ввода

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Очистка экрана

        glUseProgram(shaderProgram); // Использование программы шейдеров

        rotationAngle += 0.05f; // Увеличение угла вращения
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f)); // Вращение модели
        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f), cameraUp); // Матрица вида
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f); // Матрица проекции

        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model))); // Матрица нормалей

        // Передача матриц в шейдер
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix3fv(glGetUniformLocation(shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

        // Передача направления света
        glm::vec3 lightDir = glm::vec3(0.0f, 1.0f, 1.0f); // Направление света
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightDir"), 1, glm::value_ptr(lightDir));

        // Передача направления камеры
        glm::vec3 viewDir = glm::normalize(cameraPos - glm::vec3(0.0f));
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewDir"), 1, glm::value_ptr(viewDir));

        glBindVertexArray(VAO); // Привязка VAO
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sphereIndices.size()), GL_UNSIGNED_INT, 0); // Отрисовка

        glfwSwapBuffers(window); // Обновление экрана
        glfwPollEvents(); // Обработка событий
    }

    // Очистка ресурсов
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate(); // Завершение работы GLFW
    return 0;
}
