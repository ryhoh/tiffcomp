#include <cstring>
#include "../lib/progress.h"
#include "tiffcomp.h"

// todo まずは8bitで動くものを作る
ryhoh_tiff::CompositeProcess::CompositeProcess(
    std::string &output_name, std::vector<std::string> &input_names)
{
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
    {   // 全てのストリームについて，エントリをロードし画像開始位置にセット
        inputTiff->loadParam();
        const int img_start = inputTiff->getParam(ryhoh_tiff::IMG_START);
        inputTiff->getFin().seekg(img_start, std::ios_base::beg);
    }
    const int img_start = this->outputTiff_->getParam(ryhoh_tiff::IMG_START);
    this->outputTiff_->getFout().seekp(img_start, std::ios_base::beg);

    // エントリから，処理に必要なパラメータを設定
    const int width = this->inputTiffs_[0]->getParam(ryhoh_tiff::WIDTH);
    const int height = this->inputTiffs_[0]->getParam(ryhoh_tiff::HEIGHT);

    this->pixel_n_ = width * height;
    this->pixel_n_extra_ = this->pixel_n_ % this->pixel_n_per_phaze_;
    this->phaze_n_ = this->pixel_n_ / this->pixel_n_per_phaze_;
}

void ryhoh_tiff::CompositeProcess::run()
{
    ryhoh_prgr::Progress *progress = new ryhoh_prgr::Progress(this->phaze_n_);
    unsigned char *res = new unsigned char[this->pixel_n_ * 3];
    for (int phaze_i=0; phaze_i < this->phaze_n_; ++phaze_i)
    {
        memset(res, 0, sizeof(res));
        pile_n_pixels(res, this->pixel_n_per_phaze_);
        this->outputTiff_->getFout().write((char *)res, this->pixel_n_per_phaze_ * 3);
    }
    // phazeでやり残した分
    memset(res, 0, sizeof(this->pixel_n_extra_ * 3));
    pile_n_pixels(res, this->pixel_n_extra_);
    this->outputTiff_->getFout().write((char *)res, this->pixel_n_extra_ * 3);

    delete progress;
    delete res;
}

void ryhoh_tiff::CompositeProcess::pile_n_pixels(unsigned char *base, int pixel_n)
{

}
