// 作成 - 2019/03/18 Tetsuya Hori

#ifndef TIFF_H
#define TIFF_H

#include <map>
#include <vector>
#include <fstream>

namespace ryhoh_tiff
{
    const std::string OUTPUT_NAME = "output.tif";
    
    // 画像データに関するパラメータ
    const short WIDTH = 0x0100;
    const short HEIGHT = 0x0101;
    const short BIT_DEPTH = 0x0102;
    const short IMG_START = 0x0111;

    const int IDF_PTR = 4;


    class Tiff
    {
    private:
        std::map<short, int> param_;
        std::string fileName_;
    public:
        // Tiff() = delete;
        // Tiff(std::string fileName): fileName(fileName) {}
        Tiff() {}
        virtual ~Tiff() {}

        inline int getParam(int key) { return this->param_[key]; }
        inline void setParam(int key, int value) { this->param_[key] = value; }
    };


    class InputTiff: public Tiff
    {
    private:
        // バイナリ => 数値  変換用の共用体
        union BinaryPack
        {
            char buf_char[4];
            short buf_short[2];
            int buf_int;
        };
        std::ifstream fin_;
    public:
        InputTiff() = delete;
        InputTiff(const std::string &fileName);
        InputTiff(const InputTiff &inputTiff) {}
        ~InputTiff();

        void loadParam();
        void checkBitDepth();
        short readShort();
        int readInt();

        std::ifstream &getFin() { return this->fin_; }
    };


    class OutputTiff: public Tiff
    {
    private:
        std::ofstream fout_;
    public:
        OutputTiff() = delete;
        OutputTiff(const std::string &fileName);
        OutputTiff(const OutputTiff &outputTiff) {}
        ~OutputTiff();

        void copyWriteFrom(InputTiff &inputTiff);

        std::ofstream &getFout() { return this->fout_; }
    };
}


#endif
