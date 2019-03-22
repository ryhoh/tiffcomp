#include "tiffcomp.h"

int main(int argc, char *argv[])
{
    ryhoh_tiff::CompositeProcess *compositeProcess;
    try
    {
        compositeProcess = get_opt(argc, argv);
    }
    catch (std::exception &e)
    {
        const std::string error_s = std::string(e.what());
        if (error_s.compare(std::string("-o without name")) == 0
            || error_s.compare(std::string("-o with option")) == 0)
        {
            std::cout << "エラー： オプションoの後には，出力ファイル名を与えてください"
                << std::endl;
            exit(EXIT_FAILURE);
        }
        else
        {
            std::cout << "その他のエラー： " << e.what() << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    compositeProcess->setUp();
    compositeProcess->run();
    delete compositeProcess;
}

ryhoh_tiff::CompositeProcess *get_opt(int arg_c, char *argv[])
{
    std::vector<std::string> input_names;
    std::string output_name = "output.tif";
    bool verbose = false;

    for (int arg_i=1; arg_i < arg_c; ++arg_i)
    {
        char *arg = argv[arg_i];
        if (arg[0] == '-')
        {   // オプションが指定される
            for (char *arg_p=arg+1; *arg_p!='\0'; ++arg_p)
            {
                switch (*arg_p)
                {
                case 'h':
                    print_help();
                    exit(EXIT_SUCCESS);
                case 'v':
                    print_version();
                    exit(EXIT_SUCCESS);
                case 'o':
                    ++arg_i;
                    if (arg_i == arg_c)
                    {
                        throw std::invalid_argument("-o without name");
                    }
                    else if(argv[arg_i][0] == '-')
                    {
                        throw std::invalid_argument("-o with option");
                    }
                    output_name = std::string(argv[arg_i]);
                    break;
                case 'V':
                    verbose = true;
                    break;
                default: break;
                }
            }
        }
        else
        {   // 入力ファイルが指定される
            input_names.emplace_back(arg);
        }
    }

    ryhoh_tiff::CompositeProcess *compositeProcess
        = new ryhoh_tiff::CompositeProcess(output_name, input_names);
    compositeProcess->setVerbose(verbose);
    return compositeProcess;
}

void print_help()
{
    std::string text
        = std::string("\n")
        + "tiffcomp - tiff画像の合成プログラム\n"
        + "\n"
        + "連続撮影した夜景画像などを与えることで，比較明合成を行うプログラムです．\n"
        + "\n"
        + "使い方：\n"
        + "プログラムの引数に，合成するファイル1つ1つのパスとオプションを与える．\n"
        + "例1） tiffcomp sample/ex1.tif sample/ex2.tif sample/ex3.tif\n"
        + "例2） tiffcomp -o out.tif -V sample/ex1.tif sample/ex2.tif sample/ex3.tif\n"
        + "\n"
        + "オプション：\n"
        + "    -h : ヘルプ（このテキスト）を表示\n"
        + "    -o + (ファイル名) : 出力ファイル名を変更 （デフォルトは\"output.tif\"）\n"
        + "    -v : バージョン情報を表示\n"
        + "    -V : プログレスバーを用いて途中経過を表示\n";
    std::cout << text << std::endl;
}

void print_version()
{
    std::string text
        = std::string("")
        + "tiffcomp - built in 2019/03/22";
    std::cout << text << std::endl;
}
