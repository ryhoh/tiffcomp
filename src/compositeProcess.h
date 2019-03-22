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

        bool verbose_ = true;  // trueでプログレスバーを出す

        // todo ビットの深さ，画像サイズなどが全てのファイル間で同一かチェックする関数

        void pile_n_pixels(unsigned char *base, int pixel_n);
        inline int brightness(unsigned char *pixels);
    public:
        CompositeProcess() = delete;
        CompositeProcess(
            std::string &output_name,
            std::vector<std::string> &input_names
        );
        ~CompositeProcess();

        void setUp();
        void run();

        inline int getPixel_n() { return this->pixel_n_; }
        inline int getPixel_n_extra() { return this->pixel_n_extra_; }
        inline int getPhaze_n() { return this->phaze_n_; }
        inline void setVerbose(bool verbose) { this->verbose_ = verbose; }
    };

    inline int CompositeProcess::brightness(unsigned char *pixel)
    {
        return 306 * *pixel + 601 * *(pixel + 1) + 117 * *(pixel + 2);
    }
}


#endif
