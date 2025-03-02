#include <fstream>
#include <iostream>

#include "loader.h"

void Loader::read_wav(std::string fname)
{
    // Open the WAV file
    std::ifstream wavfile(fname, std::ios::binary);
 
    if(!wavfile.is_open())
    {
        throw;
    }
    // Read the WAV header
    Twavheader wav;
    wavfile.read(reinterpret_cast<char *>(&wav), sizeof(Twavheader));

    // If the file is a valid WAV file
    if (std::string(wav.format, 4) != "WAVE" || std::string(wav.chunk_ID, 4) != "RIFF")
    {
        wavfile.close();
        std::cerr << "Not a WAVE or RIFF!" << std::endl;
        throw;
    }

    // Properties of WAV File
    std::cout << "FileName:" << fname << std::endl;
    std::cout << "File size:" << wav.chunk_size + 8 << std::endl;
    std::cout << "Resource Exchange File Mark:" << std::string(wav.chunk_ID, 4) << std::endl;
    std::cout << "Format:" << std::string(wav.format, 4) << std::endl;
    std::cout << "Channels: " << wav.num_channels << std::endl;
    std::cout << "Sample Rate: " << wav.sample_rate << " Hz" << std::endl;
    std::cout << "Bits Per Sample: " << wav.bits_per_sample << " bits" << std::endl;

    // Read wave data
    while (std::string(wav.sub_chunk2_ID, 4) != "data")
    {
        wavfile.seekg(wav.sub_chunk2_size, std::ios::cur);
        wavfile.read(reinterpret_cast<char *>(&wav.sub_chunk2_ID), 4);
        wavfile.read(reinterpret_cast<char *>(&wav.sub_chunk2_size), 4);
    }
    std::vector<int16_t> audio_data(wav.sub_chunk2_size / sizeof(int16_t));
    wavfile.read(reinterpret_cast<char *>(audio_data.data()), wav.sub_chunk2_size);
    wavfile.close(); // Close audio file

    std::cout << "Number of samples: " << wav.sub_chunk2_size / sizeof(int16_t) << std::endl;

    m_samples = audio_data;
    m_samplesFloat = std::vector<float>(audio_data.begin(), audio_data.end()); //convert to float for fft
    m_nChannels = wav.num_channels;
    m_sampleRate = wav.sample_rate;
    m_sampleSize = wav.bits_per_sample / 8;
    m_nSamples = audio_data.size();

    if (m_nChannels != 2 ) {
        std::cerr << "Unsupported number of audio channels: " << m_nChannels << std::endl;
        throw;
    }

}