// 作成 - 2019/03/18 Tetsuya Hori
#include <gtest/gtest.h>
#include "../src/tiff.h"

TEST(InputTiffTest, loadParamTest)
{
    using ryhoh_tiff::InputTiff;

    std::string input_name = "sample/ex1.tif";
    InputTiff *inputTiff = new InputTiff(input_name);
    inputTiff->loadParam();
    std::map<short, int> &param = inputTiff->getParam();
    ASSERT_EQ(param[ryhoh_tiff::WIDTH]    , 1600);
    ASSERT_EQ(param[ryhoh_tiff::HEIGHT]   , 1067);
    ASSERT_EQ(param[ryhoh_tiff::BIT_DEPTH],    8);

    delete inputTiff;
}

TEST(OutputTiffTest, copyWriteFromTest)
{
    using ryhoh_tiff::InputTiff;
    using ryhoh_tiff::OutputTiff;

    std::string input_name = "sample/ex1.tif";
    std::string output_name = ".copyWriteFrom.tif";
    InputTiff *inputTiff = new InputTiff(input_name);
    OutputTiff *outputTiff = new OutputTiff(output_name);
    outputTiff->copyWriteFrom(*inputTiff);
    // sample/ex1.tif と output.tif のdiffを取る

    delete(inputTiff);
    delete(outputTiff);
}
