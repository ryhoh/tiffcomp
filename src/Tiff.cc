// 作成 - 2019/03/18 Tetsuya Hori
#include <cstring>
#include "../lib/progress.h"
#include "Tiff.h"


ryhohTiff::InputTiff::InputTiff(const std::string &fileName)
{
    this->fin_.open(fileName, std::ios::binary);
    if (this->fin_.fail())
    {
        std::cerr
            << "InputTiff error #1: open \"" + fileName + "\" failed"
            << std::endl;
        exit(EXIT_FAILURE);
    }
}

ryhohTiff::InputTiff::~InputTiff()
{
    this->fin_.close();
}

void ryhohTiff::InputTiff::loadParam()
{
    this->fin_.seekg(ryhohTiff::IDF_PTR, std::ios_base::beg);
    const int idf_start = readInt();
    this->fin_.seekg(idf_start, std::ios_base::beg);
    const short idf_entry_num = readShort();

    for (short entry_i=0; entry_i<idf_entry_num; ++entry_i)
    {
        const short header = readShort();
        this->fin_.seekg(6, std::ios_base::cur);
        setParam(header, readInt());
    }

    this->checkBitDepth();
}

void ryhohTiff::InputTiff::checkBitDepth()
{
    const int bitDepth = this->getParam(ryhohTiff::BIT_DEPTH);
    if (bitDepth < 17) return ;

    this->fin_.seekg(bitDepth, std::ios_base::beg);
    this->setParam(ryhohTiff::BIT_DEPTH, readShort());
}

short ryhohTiff::InputTiff::readShort()
{
    BinaryPack binaryPack = BinaryPack();
    this->fin_.read(binaryPack.buf_char, 2);
    return binaryPack.buf_short[0];
}

int ryhohTiff::InputTiff::readInt()
{
    BinaryPack binaryPack = BinaryPack();
    this->fin_.read(binaryPack.buf_char, 4);
    return binaryPack.buf_int;
}


ryhohTiff::OutputTiff::OutputTiff(const std::string &fileName)
{
    this->fout_.open(fileName, std::ios::binary);
    if (this->fout_.fail())
    {
        std::cerr
            << "OutputTiff error #1: open \"" + fileName + "\" failed"
            << std::endl;
        exit(EXIT_FAILURE);
    }
}

ryhohTiff::OutputTiff::~OutputTiff()
{
    this->fout_.close();
}

void ryhohTiff::OutputTiff::copyWriteFrom(ryhohTiff::InputTiff &inputTiff)
{
    const int buff_size = 4096;
    char buff[buff_size];
    std::ifstream &source = inputTiff.getFin();
    source.seekg(0, std::ios_base::beg);

    while (true)
    {
        source.read(buff, buff_size);
        const auto readSize = source.gcount();
        if (readSize == 0) break;
        this->fout_.write(buff, readSize);
    }
}


// int comp(int fileNum, char *file[]);
// int checkPixel(TIFF image[], FILE *fpw, int fileNum);
// int checkPixel16(TIFF image[], FILE *fpw, int fileNum);
// void compare(unsigned char array[], FILE *fp, int pixNum);
// void compare16(unsigned short array[], FILE *fp, int pixNum);
// void clearArray(unsigned char array[], int num);
// void clearArray16(unsigned short array[], int num);
