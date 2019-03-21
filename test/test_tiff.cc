// 作成 - 2019/03/18 Tetsuya Hori
#include <gtest/gtest.h>
#include "../src/tiff.h"

TEST(InputTiffTest, loadParamTest)
{
    using ryhoh_tiff::InputTiff;

    InputTiff *inputTiff = new InputTiff("sample/ex1.tif");
    inputTiff->loadParam();
    ASSERT_EQ(inputTiff->getParam(ryhoh_tiff::WIDTH)    , 1600);
    ASSERT_EQ(inputTiff->getParam(ryhoh_tiff::HEIGHT)   , 1067);
    ASSERT_EQ(inputTiff->getParam(ryhoh_tiff::BIT_DEPTH),    8);

    delete inputTiff;
}

TEST(OutputTiffTest, copyWriteFromTest)
{
    using ryhoh_tiff::InputTiff;
    using ryhoh_tiff::OutputTiff;

    InputTiff *inputTiff = new InputTiff("sample/ex1.tif");
    OutputTiff *outputTiff = new OutputTiff(".copyWriteFrom.tif");
    outputTiff->copyWriteFrom(*inputTiff);
    // sample/ex1.tif と output.tif のdiffを取る

    delete(inputTiff);
    delete(outputTiff);
}
