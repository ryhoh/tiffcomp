// 作成 - 2019/03/18 Tetsuya Hori
#include <gtest/gtest.h>
#include "../src/Tiff.h"

TEST(InputTiffTest, loadParamTest)
{
    using ryhohTiff::InputTiff;

    InputTiff *inputTiff = new InputTiff("sample/ex1.tif");
    inputTiff->loadParam();
    ASSERT_EQ(inputTiff->getParam(ryhohTiff::WIDTH)    , 1600);
    ASSERT_EQ(inputTiff->getParam(ryhohTiff::HEIGHT)   , 1067);
    ASSERT_EQ(inputTiff->getParam(ryhohTiff::BIT_DEPTH),    8);

    delete inputTiff;
}

TEST(OutputTiffTest, copyWriteFromTest)
{
    using ryhohTiff::InputTiff;
    using ryhohTiff::OutputTiff;

    InputTiff *inputTiff = new InputTiff("sample/ex1.tif");
    OutputTiff *outputTiff = new OutputTiff(".copyWriteFrom.tif");
    outputTiff->copyWriteFrom(*inputTiff);
    // sample/ex1.tif と output.tif のdiffを取る

    delete(inputTiff);
    delete(outputTiff);
}
