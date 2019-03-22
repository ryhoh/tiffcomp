#include <gtest/gtest.h>

#include "../src/tiff.h"
#include "../src/compositeProcess.h"

TEST(CompositeProcessTest, setUpTest)
{
    using ryhoh_tiff::InputTiff;
    using ryhoh_tiff::OutputTiff;
    using ryhoh_tiff::CompositeProcess;

    std::vector<std::string> input_names =
        {"sample/ex1.tif", "sample/ex2.tif", "sample/ex3.tif"};
    std::string output_name = ".setUpTest.tif";

    CompositeProcess compositeProcess =
        CompositeProcess(output_name, input_names);

    compositeProcess.setUp();

    const int pixel_n = 1600 * 1067;
    ASSERT_EQ(pixel_n, compositeProcess.getPixel_n());
}

TEST(CompositeProcessTest, runTest)
{
    using ryhoh_tiff::InputTiff;
    using ryhoh_tiff::OutputTiff;
    using ryhoh_tiff::CompositeProcess;

    std::vector<std::string> input_names =
        {"sample/ex1.tif", "sample/ex2.tif", "sample/ex3.tif"};
    std::string output_name = ".runTest.tif";

    CompositeProcess compositeProcess =
        CompositeProcess(output_name, input_names);

    compositeProcess.setUp();
    compositeProcess.run();

    // .runTest.tif と sample/idealOutput.tif のdiffを取る
}

TEST(CompositeProcessTest, runTest_non_verbose)
{
    using ryhoh_tiff::InputTiff;
    using ryhoh_tiff::OutputTiff;
    using ryhoh_tiff::CompositeProcess;

    std::vector<std::string> input_names =
        {"sample/ex1.tif", "sample/ex2.tif", "sample/ex3.tif"};
    std::string output_name = ".runTest.tif";

    CompositeProcess compositeProcess =
        CompositeProcess(output_name, input_names);

    compositeProcess.setVerbose(false);  // プログレスバーが出ないことを確認すること
    compositeProcess.setUp();
    compositeProcess.run();

    // .runTest.tif と sample/idealOutput.tif のdiffを取る
}
