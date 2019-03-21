#include <iostream>
#include <algorithm>
#include <cstring>
#include "../lib/progress.h"
#include "tiffcomp.h"

ryhoh_tiff::CompositeProcess::CompositeProcess(
    std::string &output_name,
    std::vector<std::string> &input_names
)
{
    if (input_names.size() < 2)
        throw std::invalid_argument(
            "CompositeProcess error #1: input files insufficient");

    for (std::string &input_name: input_names)
        this->inputTiffs_.push_back(new ryhoh_tiff::InputTiff(input_name));
    this->outputTiff_ = new ryhoh_tiff::OutputTiff(output_name);
}

ryhoh_tiff::CompositeProcess::~CompositeProcess()
{
    for (ryhoh_tiff::InputTiff *inputTiff: this->inputTiffs_)
        delete inputTiff;
    delete this->outputTiff_;
}

void ryhoh_tiff::CompositeProcess::setUp()
{
    for (ryhoh_tiff::InputTiff *inputTiff: this->inputTiffs_)
        inputTiff->loadParam();  // 全ての入力ファイルについて，エントリをロード

    // エントリから，処理に必要なパラメータを設定
    std::map<short, int> &param_0 = this->inputTiffs_[0]->getParam();
    const int width = param_0[ryhoh_tiff::WIDTH];
    const int height = param_0[ryhoh_tiff::HEIGHT];

    this->pixel_n_ = width * height;
    this->pixel_n_extra_ = this->pixel_n_ % this->pixel_n_per_phaze_;
    this->phaze_n_ = this->pixel_n_ / this->pixel_n_per_phaze_;

    // 入力ファイル0から出力ファイルへコピー
    this->outputTiff_->copyWriteFrom(*(this->inputTiffs_[0]));

    for (ryhoh_tiff::InputTiff *inputTiff: this->inputTiffs_)
    {   // 全てのストリームについて，画像開始位置にセット
        std::map<short, int> &param = inputTiff->getParam();
        const int img_start = param[ryhoh_tiff::IMG_START];
        inputTiff->getFin().seekg(img_start, std::ios_base::beg);
    }
    std::map<short, int> param_out = this->outputTiff_->getParam();
    const int img_start = param_out[ryhoh_tiff::IMG_START];
    this->outputTiff_->getFout().seekp(img_start, std::ios_base::beg);
}

void ryhoh_tiff::CompositeProcess::run()
{
    ryhoh_prgr::Progress *progress;
    if (this->verbose_)
    {   // プログレスバーを作成し一度表示
        progress = new ryhoh_prgr::Progress(this->phaze_n_);
        std::cout << progress->generateString() << "\r";
    }

    unsigned char *res = new unsigned char[this->pixel_n_per_phaze_ * 3];
    for (int phaze_i=0; phaze_i < this->phaze_n_; ++phaze_i)
    {   // ピクセルを phaze_n_ 回に分割して合成処理
        std::fill(res, res + this->pixel_n_per_phaze_ * 3, 0);
        pile_n_pixels(res, this->pixel_n_per_phaze_);
        this->outputTiff_->getFout().write(
            (char *)res, this->pixel_n_per_phaze_ * 3);

        if (this->verbose_)
        {   // プログレスバーを1進め表示
            progress->click();
            std::cout << progress->generateString() << "\r";
        }
    }
    // phazeでやり残した分
    std::fill(res, res + this->pixel_n_extra_ * 3, 0);
    pile_n_pixels(res, this->pixel_n_extra_);
    this->outputTiff_->getFout().write((char *)res, this->pixel_n_extra_ * 3);

    if (this->verbose_)
    {
        delete progress;
        std::cout << std::endl;  // プログレスバーから改行
    }
    delete res;
}

// fixme ここで base を [0, pixel_n * 3) でコンポジットすべきだが，一部できていない
void ryhoh_tiff::CompositeProcess::pile_n_pixels(unsigned char *base, int pixel_n)
{
    const int byte_n = pixel_n * 3;
    unsigned char *buff = new unsigned char[byte_n];
    for (ryhoh_tiff::InputTiff *inputTiff: this->inputTiffs_)
    {
        inputTiff->getFin().read((char *)buff, byte_n);

        for (int pixel_i=0; pixel_i < pixel_n; ++pixel_i)
        {   // 輝度を計算して比較する
            const int base_brightness = brightness(&base[pixel_i * 3]);
            const int buff_brightness = brightness(&buff[pixel_i * 3]);
            if (base_brightness < buff_brightness)
                for (int pile_i=0; pile_i < 3; ++pile_i)
                    base[pixel_i * 3 + pile_i] = buff[pixel_i * 3 + pile_i];
        }
    }
    delete buff;
}
