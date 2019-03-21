// 作成 - 2019/03/17 Tetsuya Hori

#ifndef TIFFCOMP_H
#define TIFFCOMP_H

#include "tiff.h"


namespace ryhoh_tiff
{
    class CompositeProcess
    {
    private:
        // 1フェーズで処理するサイズが大きいほど速いが，やりすぎるとクラッシュする
        const int pixel_n_per_phaze_ = 4096;

        int pixel_n_;
        int pixel_n_extra_;
        int phaze_n_;
        std::vector<InputTiff *> inputTiffs_;
        OutputTiff *outputTiff_;

        void pile_n_pixels(unsigned char *base, int pixel_n);
    public:
        CompositeProcess() = delete;
        CompositeProcess(
            std::string &output_name, std::vector<std::string> &input_names);
        ~CompositeProcess();

        void setUp();
        void run();

        inline int getPixel_n() { return this->pixel_n_; }
        inline int getPixel_n_extra() { return this->pixel_n_extra_; }
        inline int getPhaze_n() { return this->phaze_n_; }
    };
}


#endif
