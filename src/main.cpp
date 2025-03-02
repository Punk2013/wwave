#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <fftw3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <memory>

#include "wwaver.h"
#include "loader.h"
#include "shader.h"
#include "VAO.h"
#include "VBO.h"
#include <sciplot/sciplot.hpp>
using namespace sciplot;

template <typename T>
void draw_wave(const std::vector<T>& audio_data, const char* filename, int width=2560, int height=1440) {
    Vec x = linspace(0, 1, audio_data.size());
    Plot2D plot;
    plot.drawCurve(x, audio_data);
    Figure fig = {{plot}};
    Canvas canvas = {{fig}};
    canvas.size(width, height);
    canvas.save(filename);
}

int main(int argc, char* argv[]) {

    Loader loader;
    loader.read_wav(argv[1]);

    int nSamples = loader.m_nSamples;
    int sampleRate = loader.m_sampleRate;
    int sampleSize = loader.m_sampleSize;
    int nChannels = loader.m_nChannels;
    auto data = loader.m_samples;
    auto data_float = loader.m_samplesFloat;

    std::shared_ptr<Wwaver> waver = std::make_shared<Wwaver>(800, 600);
    GLFWwindow* window = waver->m_window;

    // Shaders
    Shader waveShader = waver->m_shaders[0];
    Shader dftShader = waver->m_shaders[1];

    // Buffers
    VAO vao_wave, vao_dft;
    VBO vbo_wave, vbo_dft;

    vbo_wave.data(nSamples * sampleSize, data.data(), GL_STATIC_DRAW);

    vao_wave.attribute(vbo_wave, 0, GL_SHORT, nChannels, 0, true);
    vao_dft.attribute(vbo_dft, 0, GL_FLOAT, 1, 0);


    float deltaTime = 0.0f;
    int pos = 0;
    int posPerChannel = 0;
    int displayedSamples = sampleRate;
    int displayedSamplesPerChannel = displayedSamples / nChannels;

    std::vector<float> dft(sampleRate);
    fftwf_plan plan;

    FILE* audio = popen(("audacious --headless " + std::string(argv[1])).c_str(), "r");
    float audioStart = glfwGetTime();
    float lastFrame = audioStart;
    float timePlayed = 0.0f;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window) && (nSamples - pos >= displayedSamples)) {
        /* Render here */
        glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // int frameSamples = std::round(sampleRate * nChannels * deltaTime);
        // int frameSamplesPerChannel = std::round(sampleRate * deltaTime);
        // displayedSamples = 600;
        // displayedSamplesPerChannel = displayedSamples / nChannels;
        pos = std::round(sampleRate * nChannels * timePlayed);
        posPerChannel = pos / nChannels;

        // Discrete Fourier transform
        plan = fftwf_plan_r2r_1d(displayedSamples, data_float.data() + pos, dft.data(), FFTW_REDFT00, FFTW_ESTIMATE);
        fftwf_execute(plan);

        // Find value for normalization
        auto p = std::minmax_element(dft.begin(), dft.end());
        float dftmax = std::max(abs(*p.first), abs(*p.second));

        vbo_dft.data(displayedSamples * sizeof(float), dft.data(), GL_DYNAMIC_DRAW);

        dftShader.use();
        vao_dft.bind();

        dftShader.setInt("uNSamples", displayedSamples);
        dftShader.setFloat("uDftmax", dftmax);
        dftShader.setVec4("uColor", 0.4, 0.5, 0.6, 1.0);

        glDrawArrays(GL_LINE_STRIP, 0, displayedSamples);

        waveShader.use();
        vao_wave.bind();

        waveShader.setInt("uNSamples", displayedSamplesPerChannel);
        waveShader.setInt("uPos", posPerChannel);
        dftShader.setVec4("uColor", 0.8, 0.5, 0.6, 1.0);

        glDrawArrays(GL_LINE_STRIP, posPerChannel, displayedSamplesPerChannel);

        // Calculate frametime and playback time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        timePlayed = currentFrame - audioStart;

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    fftwf_destroy_plan(plan);
    glfwTerminate();
    return 0;
}
