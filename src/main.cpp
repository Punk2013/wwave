#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <fftw3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

#include "shader.h"
#include "VAO.h"
#include "VBO.h"
#include <sciplot/sciplot.hpp>
using namespace sciplot;

enum class mode {
    wave,
    DFT,
    waveAndDFT,
    levelLine,
};

// WAVE file header structure
struct Twavheader
{
    char chunk_ID[4];              //  4  riff_mark[4];
    uint32_t chunk_size;           //  4  file_size;
    char format[4];                //  4  wave_str[4];
 
    char sub_chunk1_ID[4];         //  4  fmt_str[4];
    uint32_t sub_chunk1_size;      //  4  pcm_bit_num;
    uint16_t audio_format;         //  2  pcm_encode;
    uint16_t num_channels;         //  2  sound_channel;
    uint32_t sample_rate;          //  4  pcm_sample_freq;
    uint32_t byte_rate;            //  4  byte_freq;
    uint16_t block_align;          //  2  block_align;
    uint16_t bits_per_sample;      //  2  sample_bits;
 
    char sub_chunk2_ID[4];         //  4  data_str[4];
    uint32_t sub_chunk2_size;      //  4  sound_size;
};                                 // 44  bytes TOTAL

std::tuple<Twavheader, std::vector<int16_t>> read_wav(std::string fname)
{
    // Open the WAV file
    std::ifstream wavfile(fname, std::ios::binary);
 
    if(wavfile.is_open())
    {
        // Read the WAV header
        Twavheader wav;
        wavfile.read(reinterpret_cast<char*>(&wav), sizeof(Twavheader));
 
        // If the file is a valid WAV file
        if (std::string(wav.format, 4) != "WAVE" || std::string(wav.chunk_ID, 4) != "RIFF")
        {
            wavfile.close();
            std::cerr << "Not a WAVE or RIFF!" << std::endl;
            throw;
        }
 
        // Properties of WAV File
        std::cout << "FileName:" << fname << std::endl;
        std::cout << "File size:" << wav.chunk_size+8 << std::endl;
        std::cout << "Resource Exchange File Mark:" << std::string(wav.chunk_ID, 4) << std::endl;
        std::cout << "Format:" << std::string(wav.format, 4) << std::endl;
        std::cout << "Channels: " << wav.num_channels << std::endl;
        std::cout << "Sample Rate: " << wav.sample_rate << " Hz" << std::endl;
        std::cout << "Bits Per Sample: " << wav.bits_per_sample << " bits" << std::endl;
 
        // Read wave data
        while (std::string(wav.sub_chunk2_ID, 4) != "data") {
            wavfile.seekg(wav.sub_chunk2_size, std::ios::cur);
            wavfile.read(reinterpret_cast<char*>(&wav.sub_chunk2_ID), 4);
            wavfile.read(reinterpret_cast<char*>(&wav.sub_chunk2_size), 4);
        }
        std::vector<int16_t> audio_data( wav.sub_chunk2_size / sizeof(int16_t) );
        wavfile.read(reinterpret_cast<char*>( audio_data.data() ), wav.sub_chunk2_size );
        wavfile.close();  // Close audio file

        std::cout << "Number of samples: " << wav.sub_chunk2_size / sizeof(int16_t) << std::endl;

        return {wav, audio_data};
    }
    throw;
}

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
    auto [header, data] = read_wav(argv[1]);
    std::vector<float> data_float(data.begin(), data.end()); //convert to float for fft

    int nSamples = data.size();
    int sampleRate = header.sample_rate;
    int sampleSize = header.bits_per_sample / 8;
    int nChannels = header.num_channels;

    if (nChannels != 2 ) {
        std::cerr << "Unsupported number of audio channels: " << nChannels << std::endl;
        return -1;
    }

    /* Initialize the library */
    if (!glfwInit())
        return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window;
    const int width = 800;
    const int height = 600;
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(width, height, "wwave", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
    }

    glViewport(0, 0, width, height);

    // Shaders
    Shader waveShader("src/shaders/wave.vert", "src/shaders/basic.frag");
    Shader dftShader("src/shaders/dft.vert", "src/shaders/basic.frag");
    Shader levelLineShader("src/shaders/fullscreen.vert", "src/shaders/levelline.frag");

    // Buffers
    VAO vao_wave, vao_dft;
    VBO vbo_wave, vbo_dft;

    vbo_wave.data(nSamples * sampleSize, data.data(), GL_STATIC_DRAW);

    vao_wave.attribute(vbo_wave, 0, GL_SHORT, nChannels, 0, true);
    vao_dft.attribute(vbo_dft, 0, GL_FLOAT, 1, 0);

    GLuint functionTexture;
    glGenTextures(1, &functionTexture);
    glBindTexture(GL_TEXTURE_BUFFER, functionTexture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, vbo_wave.ID);

    float deltaTime = 0.0f;
    int pos = 0;
    int posPerChannel = 0;
    int displayedSamples = sampleRate;
    int displayedSamplesPerChannel = displayedSamples / nChannels;
    mode mode = mode::levelLine;

    std::vector<float> dft(displayedSamples);
    fftwf_plan plan;

    FILE* audio = popen(("audacious --headless " + std::string(argv[1])).c_str(), "r");
    float audioStart = glfwGetTime();
    float lastFrame = audioStart;
    float timePlayed = 0.0f;

    /* Loop until the user closes the window or we are out of samples*/
    while (!glfwWindowShouldClose(window) && (nSamples - pos >= displayedSamples)) {
        /* Render here */
        glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // int frameSamples = std::round(sampleRate * nChannels * deltaTime);
        // int frameSamplesPerChannel = std::round(sampleRate * deltaTime);
        // displayedSamples = 600;
        // displayedSamplesPerChannel = displayedSamples / nChannels;
        
        //position of the first sample to be displayed; samples alternate channels lrlrlrlr
        pos = std::round(sampleRate * nChannels * timePlayed) - displayedSamples;
        pos = std::max(0, pos);
        posPerChannel = pos / nChannels;

        // Discrete Fourier transform
        plan = fftwf_plan_r2r_1d(displayedSamples, data_float.data() + pos, dft.data(), FFTW_REDFT00, FFTW_ESTIMATE);
        fftwf_execute(plan);

        // Find value for normalization
        auto p = std::minmax_element(dft.begin(), dft.end());
        float dftmax = std::max(abs(*p.first), abs(*p.second));

        // mode specific
        if (mode == mode::DFT || mode == mode::waveAndDFT) {
            vbo_dft.data(displayedSamples * sizeof(float), dft.data(), GL_DYNAMIC_DRAW);

            dftShader.use();
            vao_dft.bind();

            dftShader.setInt("uNSamples", displayedSamples);
            dftShader.setFloat("uDftmax", dftmax);
            dftShader.setVec4("uColor", 0.4, 0.5, 0.6, 1.0);

            glDrawArrays(GL_LINE_STRIP, 0, displayedSamples);
        }

        if (mode == mode::wave || mode == mode::waveAndDFT) {
            waveShader.use();
            vao_wave.bind();

            waveShader.setInt("uNSamples", displayedSamplesPerChannel);
            waveShader.setInt("uPos", posPerChannel);
            dftShader.setVec4("uColor", 0.8, 0.5, 0.6, 1.0);

            glDrawArrays(GL_LINE_STRIP, posPerChannel, displayedSamplesPerChannel);
        }

        if (mode == mode::levelLine) {
            levelLineShader.use();
            vao_wave.bind();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_BUFFER, functionTexture);

            levelLineShader.setInt("functionSampler", 0);
            levelLineShader.setVec2("uRes", float(width), float(height));
            levelLineShader.setInt("uNSamples", displayedSamplesPerChannel);
            levelLineShader.setInt("uPos", posPerChannel);

            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

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
